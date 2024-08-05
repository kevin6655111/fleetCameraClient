#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <opencv2/opencv.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

using websocketpp::connection_hdl;

typedef websocketpp::client<websocketpp::config::asio_client> client;

class WebSocketClient {
  public:
    WebSocketClient(const std::string &uri, int timeout);
    void on_message(connection_hdl, client::message_ptr msg);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_fail(connection_hdl hdl);
    void run();
    void send_image(const cv::Mat &image);
    bool isStreaming() const { return start_streaming; }

  private:
    void reconnect();

    std::string wsUri;
    int timeout;
    client c;
    connection_hdl connection;
    bool open = false;
    bool should_reconnect = false;
    bool start_streaming = false; 
};

#endif // WEBSOCKETCLIENT_H