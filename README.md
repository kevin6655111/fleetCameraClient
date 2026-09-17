# LiveCamera

車載相機客戶端。透過 Spinnaker SDK 讀取相機影像，即時顯示在畫面上，並透過 WebSocket 將 GPS 座標與相機影像串流回後端伺服器（fleetMonitor）。

## 使用技術

- C++17
- [Spinnaker SDK](https://www.flir.com/products/spinnaker-sdk/)（FLIR 相機控制與影像擷取）
- OpenCV 4（影像處理與視窗顯示）
- Qt5（取得螢幕解析度以置中視窗）
- [websocketpp](https://github.com/zaphoyd/websocketpp)（WebSocket 客戶端，git submodule）
- libcurl / libicuuc / Mapnik
