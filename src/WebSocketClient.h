#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <opencv2/opencv.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

using websocketpp::connection_hdl;

typedef websocketpp::client<websocketpp::config::asio_client> client;

class WebSocketClient {
  public:
    WebSocketClient(const std::string &uri);
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
    bool open = false; // connection status
    bool should_reconnect = false; // reconnect flag
    bool start_streaming = false; // streaming flag
    bool serverOverloaded = false; // server overloaded flag
    int uploadInterval = 100; // streaming initial upload interval is 100ms
    const int maxUploadInterval = 1000; // max upload interval is 1s
    const int timeout = 10; // reconnect timeout is 10s
    int rtt = 0;  // Round-Trip Time
    const int pingInterval = 30000; // send ping every 30s

    client c;
    connection_hdl connection;
    std::chrono::steady_clock::time_point last_upload;
    std::chrono::steady_clock::time_point last_ping_time;
};

#endif // WEBSOCKETCLIENT_H