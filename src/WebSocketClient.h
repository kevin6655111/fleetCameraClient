#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <opencv2/opencv.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

using websocketpp::connection_hdl;

typedef websocketpp::client<websocketpp::config::asio_client> client;

class WebSocketClient {
  public:
    WebSocketClient(const std::string &uri, int timeout, int uploadInterval, int maxUploadInterval);
    void connect();
    void reconnect();
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_fail(connection_hdl hdl);
    void on_message(connection_hdl, client::message_ptr msg);
    void on_pong(connection_hdl hdl, std::string payload);
    void adjustUploadInterval();
    void run();
    void send_ping();
    void send_image(const cv::Mat &image);
    bool isStreaming() const { return start_streaming; }

  private:
    std::string uri;
    int timeout = 30;
    int uploadInterval = 100;
    int maxUploadInterval = 1000;
    int rtt = 0;  // Round-Trip Time

    client c;
    connection_hdl connection;
    bool open = false;
    bool should_reconnect = false;
    bool start_streaming = false; 

    std::chrono::steady_clock::time_point last_upload;
    std::chrono::steady_clock::time_point last_ping_time;
    const int pingInterval = 30000; // send ping every 30 seconds
};

#endif // WEBSOCKETCLIENT_H