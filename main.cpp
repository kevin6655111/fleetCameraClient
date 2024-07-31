#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include "src/WebSocketClient.h"
#include <thread>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace std;

void setNodeValue(CIntegerPtr nodePtr, int value, const string &nodeName) {
    if (IsAvailable(nodePtr) && IsWritable(nodePtr)) {
        nodePtr->SetValue(value);
        cout << nodeName << " set to " << nodePtr->GetValue() << endl;
    } else {
        cout << nodeName << " not available or not writable..." << endl;
    }
}

int main(int argc, char **argv) {
    string vehicleId = "BUG-0604"; // 車牌號碼
    string token = "yJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyaWQiOiJqdWFodWEiLCJ1c2VybmFtZSI6Imp1YWh1YSIsImlzQWRtaW4iOmZhbHNlLCJpYXQiOjE3MDQ5NTg1NDB9._LwnFsdTMOKzE0F2mIMiYPb6NJusLNXIzUjvktTHyjY"; // 金鑰

    int cameraWidth = 2300; // 相機解析度
    int cameraHeight = 1800;
    
    int windowWidth = 800; 
    int windowHeight = 600;

    WebSocketClient ws_client(vehicleId, token);

    thread client_thread([&]() {
        ws_client.run("ws://localhost:3002/AIVehicles?Id=" + vehicleId + "&Token=" + token);
    });

    QApplication app(argc, argv);

    // Get the screen resolution
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    int windowPosX = (screenWidth - windowWidth) /2;
    int windowPosY = (screenHeight - windowHeight) /2;

    // Initialize the system
    SystemPtr system = System::GetInstance();

    // Get the camera list
    CameraList camList = system->GetCameras();

    unsigned int numCameras = camList.GetSize();

    if (numCameras == 0) {
        cout << "No cameras detected." << endl;
        system->ReleaseInstance();
        return -1;
    }

    // Get the first camera 
    CameraPtr pCam = camList.GetByIndex(0);

    try {
        // Initialize the camera
        pCam->Init();

        // Set the camera resolution
        INodeMap &nodeMap = pCam->GetNodeMap();
        CIntegerPtr widthPtr = nodeMap.GetNode("Width");
        CIntegerPtr heightPtr = nodeMap.GetNode("Height");

        setNodeValue(widthPtr, cameraWidth, "Width");
        setNodeValue(heightPtr, cameraHeight, "Height");

        // Set the camera frame rate
        CFloatPtr frameRatePtr = nodeMap.GetNode("AcquisitionFrameRate");
        if (IsAvailable(frameRatePtr) && IsWritable(frameRatePtr)) {
            frameRatePtr->SetValue(15.0); // 相機幀率
            cout << "Frame rate set to " << frameRatePtr->GetValue() << endl;
        } else {
            cout << "Frame rate not available or not writable..." << endl;
        }

        // Begin acquiring images
        pCam->BeginAcquisition();

        // Create an OpenCV window
        cv::namedWindow("Camera Image", cv::WINDOW_NORMAL);
        cv::resizeWindow("Camera Image", windowWidth, windowHeight);
        cv::moveWindow("Camera Image", windowPosX, windowPosY);

        while (true) {
            // Retrieve the next received image
            ImagePtr pResultImage = pCam->GetNextImage();

            // Ensure image completion
            if (pResultImage->IsIncomplete()) {
                cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus()) << endl;
            } else {
                // Convert the image to an OpenCV Mat object
                ImageProcessor processor;
                processor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);

                ImagePtr convertedImage = processor.Convert(pResultImage, PixelFormat_BGR8);
                
                // Create an OpenCV Mat with the data from the image
                cv::Mat cvImage = cv::Mat(convertedImage->GetHeight(), convertedImage->GetWidth(), CV_8UC3, convertedImage->GetData(), convertedImage->GetStride());

                ws_client.send_image(cvImage);

                // Display the image
                cv::imshow("Camera Image", cvImage);

                if (cv::waitKey(1) == 27) {
                    break;
                }
            }

            // Release the image
            pResultImage->Release();
        }

        // End acquisition
        pCam->EndAcquisition();

        // Deinitialize the camera
        pCam->DeInit();
    } catch(Spinnaker::Exception &e) {
        cout << "Error:" << e.what() << endl;
    }

    // Release the camera
    pCam = nullptr;

    // Clear the camera list
    camList.Clear();

    // Release the system
    system->ReleaseInstance();

    // Wait for the websocket client thread to finish
    client_thread.join();

    return 0;
}