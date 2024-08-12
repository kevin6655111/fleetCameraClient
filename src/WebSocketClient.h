#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <opencv2/opencv.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include "Timer.h"

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
    void send_gps();
    void send_image(const cv::Mat &image);
    bool isStreaming() const { return start_streaming; }

  private:
    std::string uri;
    bool open = false; // connection status
    bool should_reconnect = false; // reconnect flag
    bool start_streaming = false; // streaming flag
    bool serverOverloaded = false; // server overloaded flag
    int uploadInterval = 50; // streaming initial upload interval is 100ms
    int rtt = 0;  // Round-Trip Time
    const int maxUploadInterval = 1000; // max upload interval is 1s
    const int minUploadInterval = 50; // min upload interval is 50ms
    const int intervalAdjustment = 50; // upload interval adjustment is 50ms
    const int timeout = 30; // reconnect timeout is 10s
    const int pingInterval = 15000; // send ping every 15s
    const int imageQuality = 70; // image quality 

    client c;
    connection_hdl connection;
    Timer last_upload;
    Timer last_ping_time;
    Timer last_gps;
};

#endif // WEBSOCKETCLIENT_H