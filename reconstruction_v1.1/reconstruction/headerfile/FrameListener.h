#pragma once
#include <cstdio>
#include <chrono>
#include <iostream>
#include <astra/astra.hpp>
#include <iomanip>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class FrameListener : public astra::FrameListener
{
public:
    virtual void on_frame_ready(astra::StreamReader& reader, astra::Frame& frame) override
    {
        save_depth(frame);
        save_color(frame);
        //check_fps();
        cv::waitKey(1);
    }

    void save_depth(astra::Frame& frame)
    {
        const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();

        if (depthFrame.is_valid())
        {
            int depthWidth = depthFrame.width();
            int depthHeight = depthFrame.height();
            int frameIndex = depthFrame.frame_index();

            cv::Mat depthImage(depthHeight, depthWidth, CV_16UC1);
            cv::Mat depthImageColor(depthHeight, depthWidth, CV_8UC3);

            for (int i = 0; i < depthWidth * depthHeight; i++)
            {
                depthImage.at<ushort>(i / depthWidth, i % depthWidth) = depthFrame.data()[i];
                depthImageColor.at<cv::Vec3b>(i / depthWidth, i % depthWidth)[0] = 0;
                depthImageColor.at<cv::Vec3b>(i / depthWidth, i % depthWidth)[1] = depthFrame.data()[i] / 5;
                depthImageColor.at<cv::Vec3b>(i / depthWidth, i % depthWidth)[2] = depthFrame.data()[i]/5;
            }
            imshow("depthImage", depthImageColor);
        }
    }
    void save_color(astra::Frame& frame)
    {
        const astra::ColorFrame colorFrame = frame.get<astra::ColorFrame>();
        if (colorFrame.is_valid())
        {
            int colorWidth = colorFrame.width();
            int colorHeight = colorFrame.height();

            cv::Mat colorImage(colorHeight, colorWidth, CV_8UC3);

            for (int i = 0; i < colorWidth * colorHeight; i++)
            {
                colorImage.at<cv::Vec3b>(i / colorWidth, i % colorWidth)[0] = colorFrame.data()[i].b;
                colorImage.at<cv::Vec3b>(i / colorWidth, i % colorWidth)[1] = colorFrame.data()[i].g;
                colorImage.at<cv::Vec3b>(i / colorWidth, i % colorWidth)[2] = colorFrame.data()[i].r;
            }
            imshow("colorImage", colorImage);
        }
    }
    void check_fps()
    {
        const float frameWeight = 0.5;

        auto newTimepoint = clock_type::now();
        float frameDuration = std::chrono::duration_cast<std::chrono::duration<float>>(newTimepoint - lastTimepoint_).count();

        frameDuration_ = frameDuration * frameWeight + frameDuration_ * (1 - frameWeight);
        lastTimepoint_ = newTimepoint;

        double fps = 1.0 / frameDuration_;

        auto precision = std::cout.precision();
        std::cout << std::fixed
            << std::setprecision(1)
            << fps << " fps ("
            << std::setprecision(2)
            << frameDuration_ * 1000.0 << " ms)"
            << std::setprecision(precision)
            << std::endl;
    }
private:
    float frameDuration_{ 0.0 };
    using clock_type = std::chrono::system_clock;
    std::chrono::time_point<clock_type> lastTimepoint_{ clock_type::now() };
};