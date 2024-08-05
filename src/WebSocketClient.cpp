#include "WebSocketClient.h"
#include <iostream>
#include <string>

WebSocketClient::WebSocketClient(const std::string &uri, int timeout) : uri(uri), timeout(timeout) {
  c.init_asio();

  // Disable logging
  c.clear_access_channels(websocketpp::log::alevel::frame_payload);
  c.clear_access_channels(websocketpp::log::alevel::frame_header);

  c.set_message_handler(std::bind(&WebSocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
  c.set_open_handler(std::bind(&WebSocketClient::on_open, this, std::placeholders::_1));
  c.set_close_handler(std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
  c.set_fail_handler(std::bind(&WebSocketClient::on_fail, this, std::placeholders::_1));

  connect();
}

void WebSocketClient::on_message(connection_hdl hdl, client::message_ptr msg) {
  std::string payload = msg->get_payload();

  if (payload == "START_STREAM") {
    std::cout << "Start streaming..." << std::endl;
    start_streaming = true;
  } else if (payload == "STOP_STREAM") {
    std::cout << "Stop streaming..." << std::endl;
    start_streaming = false; 
  }
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
  std::cout << "Connection closed..." << std::endl;
}

void WebSocketClient::on_fail(connection_hdl hdl) {
  open = false;
  should_reconnect = true;
  std::cout << "Connection failed..." << std::endl;
}

void WebSocketClient::reconnect() {
  std::cout << "Reconnecting in " << timeout << " seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(timeout));
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

void WebSocketClient::run() {
  while (true) {
    if (!open && should_reconnect) {
      reconnect();
      should_reconnect = false;
      connect();
    }

    c.run_one();
  }
}

void WebSocketClient::send_image(const cv::Mat &image) {
  if (open && start_streaming) {
    try {
      std::vector<uchar> buf;
      cv::imencode(".jpg", image, buf);
      std::string payload(buf.begin(), buf.end());
      c.send(connection, payload, websocketpp::frame::opcode::binary);
    } catch (const std::exception &e) {
      std::cerr << "Error sending image: " << e.what() << std::endl;    
    }
  }
}