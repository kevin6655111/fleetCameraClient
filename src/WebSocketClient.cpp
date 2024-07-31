#include "WebSocketClient.h"
#include <iostream>
#include <string>

WebSocketClient::WebSocketClient(const std::string &vehicleId, const std::string &token) : vehicleId(vehicleId), token(token) {
  c.init_asio();

  // Disable logging
  c.clear_access_channels(websocketpp::log::alevel::frame_payload);
  c.clear_access_channels(websocketpp::log::alevel::frame_header);

  c.set_message_handler(std::bind(&WebSocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
  c.set_open_handler(std::bind(&WebSocketClient::on_open, this, std::placeholders::_1));
  c.set_close_handler(std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
}

void WebSocketClient::on_message(connection_hdl hdl, client::message_ptr msg) {
  std::cout << "Received message: " << msg->get_payload() << std::endl;
}

void WebSocketClient::on_open(connection_hdl hdl) {
  connection = hdl;
  open = true;

  std::string authMessage = "{\"vehicleId\":\"" + vehicleId + "\",\"token\":\"" + token + "\"}";
  c.send(connection, authMessage, websocketpp::frame::opcode::text);
  std::cout << "Sent auth message: " << authMessage << std::endl;
}

void WebSocketClient::on_close(connection_hdl hdl) {
  open = false;
}

void WebSocketClient::run(const std::string &uri) {
  websocketpp::lib::error_code ec;
  client::connection_ptr conn = c.get_connection(uri, ec);

  if (ec) {
    std::cout << "Could not create connection: " << ec.message() << std::endl;
    return;
  }

  c.connect(conn);
  c.run();
}

void WebSocketClient::send_image(const cv::Mat &image) {
  if (open) {
    std::vector<uchar> buf;
    cv::imencode(".jpg", image, buf);
    std::string payload(buf.begin(), buf.end());
    c.send(connection, payload, websocketpp::frame::opcode::binary);
  }
}