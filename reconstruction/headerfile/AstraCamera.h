#pragma once
#include "FrameListener.h"
#include <thread>
#include <windows.h> 

enum DepthImageType { Depth_D, Depth_XYZ };
class AstraCamera
{
public:
    AstraCamera();
    ~AstraCamera();

    //config camera
    void InitialAstraCamera(int _width = 640, int _height = 480, int fps = 30);
    void StreamStrat();
    void DepthStreamStart();
    void ColorStreamStrat();

    //read camera data
    bool ReadColorAndDepthImage(cv::Mat &colorImage, cv::Mat &depthImage, DepthImageType depthImageType = Depth_D);
    bool ReadCameraColorImage(cv::Mat &colorImage);
    bool ReadCameraDepthImage(cv::Mat &depthImage, DepthImageType depthImageType = Depth_D);

    void StartListener();   //write this listener to reader,listener is callback fuction
    void StopListener();    //remove listener from reader
private:
    //camera's width height fps
    int m_width;
    int m_height;
    int m_fps;
    
    FrameListener m_listener;
    astra::StreamSet m_streamSet;
    astra::StreamReader m_reader;
    //config depth and color mode
    astra::DepthStream ConfigureDepth(astra::StreamReader& reader);
    astra::ColorStream ConfigureColor(astra::StreamReader& reader);
    //refresh the camera all time,then call the callbacke fuction to listen
    static DWORD WINAPI AstraUpdateForever(LPVOID lpParamter);
};


