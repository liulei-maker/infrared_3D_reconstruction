#include "AstraCamera.h"


AstraCamera::AstraCamera()
{

}

AstraCamera::~AstraCamera()
{
    astra::terminate();
}

void AstraCamera::InitialAstraCamera(int _width, int _height, int _fps)
{
    this->m_width = _width;
    this->m_height = _height;
    this->m_fps = _fps;

    astra::initialize();
   astra::ImageStreamMode
    StreamStrat();
}

void AstraCamera::StreamStrat() {
    this->m_reader = this->m_streamSet.create_reader();
    this->m_reader.stream<astra::PointStream>().start();
}

void AstraCamera::DepthStreamStart()
{
    astra::DepthStream depthStream = ConfigureDepth(this->m_reader);
    depthStream.start();
}

void AstraCamera::ColorStreamStrat()
{
    astra::ColorStream colorStream = ConfigureColor(this->m_reader);
    colorStream.start();
}

bool AstraCamera::ReadColorAndDepthImage(cv::Mat& colorImage, cv::Mat& depthImage, DepthImageType depthImageType)
{
    astra::Frame frame = this->m_reader.get_latest_frame();
    const auto colorFrame = frame.get<astra::ColorFrame>();
    const auto depthFrame = frame.get<astra::DepthFrame>();

    if (colorFrame.is_valid() && depthFrame.is_valid())
    {
        int colorWidth = colorFrame.width();
        int colorHeight = colorFrame.height();

        colorImage.create(colorHeight, colorWidth, CV_8UC3);

        for (int i = 0; i < colorWidth * colorHeight; i++)
        {
            ushort u = i / colorWidth;
            ushort v = i % colorWidth;
            colorImage.at<cv::Vec3b>(u, v)[0] = colorFrame.data()[i].b;
            colorImage.at<cv::Vec3b>(u, v)[1] = colorFrame.data()[i].g;
            colorImage.at<cv::Vec3b>(u, v)[2] = colorFrame.data()[i].r;
        }

        int depthWidth = depthFrame.width();
        int depthHeight = depthFrame.height();

        if (depthImageType == Depth_D)
        {
            depthImage.create(depthHeight, depthWidth, CV_8UC1);
        
            for (int i = 0; i < depthWidth * depthHeight; i++)
            {
                ushort u = i / depthWidth;
                ushort v = i % depthWidth;

                if (depthFrame.data()[i]<2550)
                {
                    depthImage.at<uchar>(u, v) = depthFrame.data()[i] /10;
                }
            }
        }
        else
        {
            depthImage.create(depthHeight, depthWidth, CV_16SC3);

            for (int i = 0; i < depthWidth * depthHeight; i++)
            {
                ushort u = i / depthWidth;
                ushort v = i % depthWidth;
                ushort d = depthFrame.data()[i];
                auto depthStream = this->m_reader.stream<astra::DepthStream>();
                float worldX, worldY, worldZ;
                depthStream.coordinateMapper().convert_depth_to_world(v, u, d, worldX, worldY, worldZ);

                depthImage.at<cv::Vec3w>(u, v)[0] = worldX;
                depthImage.at<cv::Vec3w>(u, v)[1] = worldY;
                depthImage.at<cv::Vec3w>(u, v)[2] = worldZ;
            }
        }
        return true;
    }
    return false;
}

bool AstraCamera::ReadCameraColorImage(cv::Mat &colorImage)
{
    astra::Frame frame = this->m_reader.get_latest_frame();
    const auto colorFrame = frame.get<astra::ColorFrame>();

    if (colorFrame.is_valid())
    {
        int colorWidth = colorFrame.width();
        int colorHeight = colorFrame.height();

        colorImage.create(colorHeight, colorWidth, CV_8UC3);

        for (int i = 0; i < colorWidth * colorHeight; i++)
        {
            colorImage.at<cv::Vec3b>(i / colorWidth, colorWidth - i % colorWidth - 1)[0] = colorFrame.data()[i].b;
            colorImage.at<cv::Vec3b>(i / colorWidth, colorWidth - i % colorWidth - 1)[1] = colorFrame.data()[i].g;
            colorImage.at<cv::Vec3b>(i / colorWidth, colorWidth - i % colorWidth - 1)[2] = colorFrame.data()[i].r;
        }
        return true;
    }
    return false;
}

bool  AstraCamera::ReadCameraDepthImage(cv::Mat &depthImage, DepthImageType depthImageType)
{
    astra::Frame frame = this->m_reader.get_latest_frame();
    const auto depthFrame = frame.get<astra::DepthFrame>();
    if (depthFrame.is_valid())
    {
        int depthWidth = depthFrame.width();
        int depthHeight = depthFrame.height();

        if (depthImageType == Depth_D)
        {
            depthImage.create(depthHeight, depthWidth, CV_16UC1);

            for (int i = 0; i < depthWidth * depthHeight; i++)
            {
                depthImage.at<ushort>(i / depthWidth, depthWidth - i % depthWidth - 1) = depthFrame.data()[i];
            }
        }
        else
        {
            depthImage.create(depthHeight, depthWidth, CV_16UC3);

            for (int i = 0; i < depthWidth * depthHeight; i++)
            {
                ushort u = i / depthWidth;
                ushort v = depthWidth - i % depthWidth - 1;
                ushort d = depthFrame.data()[i];
                auto depthStream = this->m_reader.stream<astra::DepthStream>();
                float worldX, worldY, worldZ;
                depthStream.coordinateMapper().convert_depth_to_world(v, u, d, worldX, worldY, worldZ);
                depthImage.at<cv::Vec3w>(u, v)[0] = worldX;
                depthImage.at<cv::Vec3w>(u, v)[1] = worldY;
                depthImage.at<cv::Vec3w>(u, v)[2] = worldZ;
            }
        }
        return true;
    }
    return false;
}

void AstraCamera::StartListener()
{
    this->m_reader.add_listener(this->m_listener);
    
    HANDLE hThread = CreateThread(NULL, 0, AstraUpdateForever, this, 0, NULL);
    /*关闭显示线程的句柄，后续程序中没有调此句柄，先关闭句柄，句柄有限*/
    CloseHandle(hThread);
}

void AstraCamera::StopListener()
{
    this->m_reader.remove_listener(this->m_listener);
}

astra::DepthStream AstraCamera::ConfigureDepth(astra::StreamReader& reader)
{
    auto depthStream = reader.stream<astra::DepthStream>();

    auto oldMode = depthStream.mode();

    //We don't have to set the mode to start the stream, but if you want to here is how:
    astra::ImageStreamMode depthMode;

    depthMode.set_width(this->m_width);
    depthMode.set_height(this->m_height);
    depthMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_DEPTH_MM);
    depthMode.set_fps(this->m_fps);

    depthStream.set_mode(depthMode);

    auto newMode = depthStream.mode();
    printf("Changed depth mode: %dx%d @ %d -> %dx%d @ %d\n",
        oldMode.width(), oldMode.height(), oldMode.fps(),
        newMode.width(), newMode.height(), newMode.fps());

    return depthStream;
}

astra::ColorStream AstraCamera::ConfigureColor(astra::StreamReader& reader)
{
    auto colorStream = reader.stream<astra::ColorStream>();

    auto oldMode = colorStream.mode();

    astra::ImageStreamMode colorMode;
    //colorMode.set_width(this->m_width);
    //colorMode.set_height(this->m_height);
    colorMode.set_width(this->m_width);
    colorMode.set_height(this->m_height);
    colorMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_RGB888);
    colorMode.set_fps(m_fps);

    colorStream.set_mode(colorMode);

    auto newMode = colorStream.mode();
    printf("Changed color mode: %dx%d @ %d -> %dx%d @ %d\n",
        oldMode.width(), oldMode.height(), oldMode.fps(),
        newMode.width(), newMode.height(), newMode.fps());

    return colorStream;
}

DWORD WINAPI AstraCamera::AstraUpdateForever(LPVOID lpParamter)
{
    while (true)
    {
        astra_update();
    }
}