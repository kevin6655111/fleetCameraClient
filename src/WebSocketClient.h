#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <opencv2/opencv.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

using websocketpp::connection_hdl;

typedef websocketpp::client<websocketpp::config::asio_client> client;

class WebSocketClient {
  public:
    WebSocketClient(const std::string &carId, const std::string &key);
    void on_message(connection_hdl, client::message_ptr msg);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void run(const std::string &uri);
    void send_image(const cv::Mat &image);

  private:
    client c;
    connection_hdl connection;
    bool open = false;
    std::string carId;
    std::string key;
};

#endif // WEBSOCKETCLIENT_H