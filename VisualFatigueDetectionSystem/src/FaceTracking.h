/**************************************************************
* Type: .h file
* Class Name: CFaceTracking
* Function: track faces via the Kinect
* Create date: 2014.10.16
* Creator： Zhaoqiang Xia (xiazhaoqiang@gmail.com)
***************************************************************/
#pragma once
// include files for opencv
#include <opencv2/opencv.hpp> 

// include files for kinect
#include <NuiApi.h>
#include <FaceTrackLib.h>

using namespace cv;

#define COLOR_WIDTH		640
#define COLOR_HEIGHT		480
#define DEPTH_WIDTH		320
#define DEPTH_HEIGHT		240
#define SKELETON_WIDTH	640
#define SKELETON_HEIGHT	480
#define CHANNEL			3

class CFaceTracking
{
public:
	CFaceTracking();
	~CFaceTracking(){}
	/***** 成员函数声明 *****/
	// 接口函数
	int KinectFTInitialize();
	void Track(Mat& resMat);
	void KinectFTRelease();
	int GetFacialExpression(Mat* pColorDisplay=NULL);

	void DisplayColorCamera();
	void KinectRecData();
	void GetColorData();
	void GetDepthData();
	void GetSkeletonData();
	// 内部成员函数
	HRESULT VisualizeFaceModel(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
	FLOAT zoomFactor, POINT viewOffset, IFTResult* pAAMRlt, UINT32 color);
	HRESULT VisualizeFaceComponents(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
	FLOAT zoomFactor, POINT viewOffset, IFTResult* pAAMRlt, UINT32 color);

	/***** 成员变量声明 *****/
private:
	FT_VECTOR3D m_hint3D[2];		// 头和肩膀中心的坐标
	IFTFaceTracker* m_pFT;				// Kinect SDK提供的人脸跟踪接口
	IFTResult* m_pFTResult;			// 记录人脸跟踪结果
	FT_SENSOR_DATA m_sensorData;		// 传感器数据结构
	FT_CAMERA_CONFIG cameraConfig; // 彩色摄像头配置变量
	FT_CAMERA_CONFIG depthConfig;	// 深度摄像头配置变量
	BYTE DepthBuf[DEPTH_WIDTH*DEPTH_HEIGHT*CHANNEL];
	// 彩色和深度图像数据缓冲区
	IFTImage* pColorFrame;
	IFTImage* pDepthFrame;
	IFTImage* pColorDisplay; 
	// 各种内核事件和句柄
	HANDLE	m_hNextVideoFrameEvent;
	HANDLE	m_hNextDepthFrameEvent;
	HANDLE	m_hNextSkeletonEvent;
	HANDLE	m_pVideoStreamHandle;
	HANDLE	m_pDepthStreamHandle;
	HANDLE	m_hEvNuiProcessStop; // 用于结束的事件对象
	HANDLE m_hProcesss;
	// status variable
	bool	isTracked;
	
};