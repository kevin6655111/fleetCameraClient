#include "WebSocketClient.h"
#include <iostream>
#include <string>

WebSocketClient::WebSocketClient(const std::string &uri) 
  : uri(uri) {
  c.init_asio();

  // Disable logging
  c.clear_access_channels(websocketpp::log::alevel::frame_payload);
  c.clear_access_channels(websocketpp::log::alevel::frame_header);

  c.set_message_handler(std::bind(&WebSocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
  c.set_open_handler(std::bind(&WebSocketClient::on_open, this, std::placeholders::_1));
  c.set_close_handler(std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
  c.set_fail_handler(std::bind(&WebSocketClient::on_fail, this, std::placeholders::_1));
  c.set_pong_handler(std::bind(&WebSocketClient::on_pong, this, std::placeholders::_1, std::placeholders::_2));

  connect();
  last_upload = std::chrono::steady_clock::now();
  last_ping_time = std::chrono::steady_clock::now();
}

void WebSocketClient::connect() {
  websocketpp::lib::error_code ec;
  client::connection_ptr conn = c.get_connection(uri, ec);

  if (ec) {
    std::cout << "Could not create connection: " << ec.message() << std::endl;
    should_reconnect = true;
  } else {
    c.connect(conn);
  }
}

void WebSocketClient::reconnect() {
  std::cout << "Reconnecting in " << timeout << " seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(timeout));
  connect();
}

void WebSocketClient::on_open(connection_hdl hdl) {
  connection = hdl;
  open = true;
  should_reconnect = false;
  std::cout << "Connection opened..." << std::endl;
}

void WebSocketClient::on_close(connection_hdl hdl) {
  open = false;
  should_reconnect = true;
  start_streaming = false;
  serverOverloaded = false;
  uploadInterval = 50;
  rtt = 0;

  std::cout << "Connection closed..." << std::endl;
}

void WebSocketClient::on_fail(connection_hdl hdl) {
  open = false;
  should_reconnect = true;
  std::cout << "Connection failed..." << std::endl;
}

void WebSocketClient::on_message(connection_hdl hdl, client::message_ptr msg) {
  std::string payload = msg->get_payload();

  if (payload == "START_STREAM") {
    std::cout << "Start streaming..." << std::endl;
    start_streaming = true;
  } else if (payload == "STOP_STREAM") {
    std::cout << "Stop streaming..." << std::endl;
    start_streaming = false; 
  } else if (payload == "SERVER_OVERLOADED") {
    std::cout << "Server is overloaded. Increasing upload interval..." << std::endl;
    serverOverloaded = true;
    adjustUploadInterval();
  } else if (payload == "SERVER_NOT_OVERLOADED") {
    std::cout << "Server is not overloaded." << std::endl;
    serverOverloaded = false;
    uploadInterval = 50;
    adjustUploadInterval();
  }
}

void WebSocketClient::adjustUploadInterval() {
  if (serverOverloaded) {
    uploadInterval = 500;
  } else {
    if (rtt > 200) {
      // High RTT, increase upload interval
      uploadInterval = std::min(uploadInterval + intervalAdjustment, maxUploadInterval);
    } else {
      // Low RTT, decrease upload interval
      uploadInterval = std::max(uploadInterval - intervalAdjustment, minUploadInterval);
    }
  }
}

void WebSocketClient::on_pong(connection_hdl hdl, std::string msg) {
  auto now = std::chrono::steady_clock::now();
  auto rtt_duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_ping_time).count();
  rtt = static_cast<int>(rtt_duration);

  last_ping_time = now;

  adjustUploadInterval();
  std::cout << "RTT: " << rtt << " ms" << ", uploadInterval: " << uploadInterval << std::endl;
}

void WebSocketClient::send_ping() {
  if (open) {
    auto now = std::chrono::steady_clock::now();
    auto ping_duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_ping_time).count();

    if (ping_duration >= pingInterval) {
      std::cout << "Sending ping..." << std::endl;
      c.ping(connection, "ping");
      last_ping_time = now;
    }
  }
}

void WebSocketClient::run() {
  std::cout << "Running websocket client..." << std::endl;

  while (true) {
    if (!open && should_reconnect) {
      reconnect();
      should_reconnect = false;
    }

    send_ping(); // 定時發送 ping
    c.poll_one(); // 處理現有的事件
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 休眠 10 毫秒
  }
}

void WebSocketClient::send_image(const cv::Mat &image) {
  auto now = std::chrono::steady_clock::now();

  if (open && start_streaming) {
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_upload).count();

    if (duration < uploadInterval) return;

    last_upload = now;

    try {
      std::vector<uchar> buf;

      std::vector<int> compression_params;
      compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
      compression_params.push_back(70);

      cv::imencode(".jpg", image, buf, compression_params);
      std::string payload(buf.begin(), buf.end());
      c.send(connection, payload, websocketpp::frame::opcode::binary);
    } catch (const std::exception &e) {
      std::cerr << "Error sending image: " << e.what() << std::endl;    
    }
  }
}