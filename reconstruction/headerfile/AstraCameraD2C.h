#pragma once
#include "d2cSwapper.h"
#include "ObCommon.h"
#include "Visualizer.h"
#include <Open3D/Open3D.h>

using namespace cv;
using namespace open3d;

#define SAMPLE_READ_WAIT_TIMEOUT 2000 //2000ms
#define MIN_DISTANCE 200  //单位毫米
#define MAX_DISTANCE 2000 //单位毫米
#define RESOULTION_X 640.0  //标定时的分辨率
#define RESOULTION_Y 480.0  //标定时的分辨率
#define MAX_FRAME_COUNT 50

typedef struct xnIntrinsic_Params
{
	xnIntrinsic_Params() :
		c_x(320.0), c_y(240.0), f_x(480.0), f_y(480.0)
	{}

	xnIntrinsic_Params(float c_x_, float c_y_, float f_x_, float f_y_) :
		c_x(c_x_), c_y(c_y_), f_x(f_x_), f_y(f_y_)
	{}

	float c_x; //u轴上的归一化焦距
	float c_y; //v轴上的归一化焦距
	float f_x; //主点x坐标
	float f_y; //主点y坐标
}xIntrinsic_Params;


class AstraCameraD2C :public d2cSwapper
{
public:
	AstraCameraD2C();
	virtual ~AstraCameraD2C();

	//函数功能：相机初始化
	int CameraInit(int d2cType);
	//函数功能：相机反初始化
	int CameraUnInit(void);

	//函数功能：获取相机流数据
	int GetColorAndDepthImage(cv::Mat& colorImage, cv::Mat& depthImage);
	int GetColorImage(cv::Mat& colorImage);
	int GetDepthImage(cv::Mat& depthImage);

	//转换图像的像素坐标到世界坐标
	void convertDepthToCamrera(const ushort& u, const ushort& v, ushort& d, double& worldX, double& worldY, double& worldZ);
	//实时显示摄像头采集的点云数据
	void RealtimeVisualPointCloud();

private:
	int DepthInit(void);
	int DepthUnInit();
	/****start depth swapper****/
	int Depthstart(int width, int height);
	int Depthstop();
	int WaitDepthStream(VideoFrameRef &frame);

	/****start color swapper****/
	int ColorStart(int width, int height);
	int ColorStop();
	int WaitColorStream(VideoFrameRef &frame);
	
	//计算获取相机内参数
	void getCameraParams(openni::Device& Device, xIntrinsic_Params& IrParam);

	//函数功能：停止流
	int StreamStop(void);
	//函数功能: 获取相机的内外参
	int GetCameraParam(OBCameraParams& cameraParam);
	//函数功能：获取Depth分辨率
	int GetCameraResolution(int& nImageWidth, int& nImageHeight);
	//函数功能：获取设备的pid
	uint16_t GetDevicePid(void);

	bool IsLunaDevice(void);


	VideoStream m_ColorStream;
	VideoStream m_depthStream;

	bool m_bDepthInit;			//设备是否初始化
	bool m_bDepthStart;			//深度数据流是否开启
	bool m_bDepStreamCreate;	//深度数据流是否创建
	//float* m_histogram;			//
	bool m_bColorStart;			//彩色数据流是否开启
	bool m_bColorStreamCreate;	//彩色数据流是否创建

private:
	//depth data
	Device m_device;

	//存储相机内参的全局变量
	xIntrinsic_Params m_IntrinsicParam;
	double fdx, fdy, u0, v0;

	int m_ImageWidth;	//图像的宽
	int m_ImageHeight;	//图像的高

	std::shared_ptr<geometry::PointCloud> pointcloud_ptr;
	Visualizer* visualizer;	//点云显示器
	bool isAddGeometry;
};

