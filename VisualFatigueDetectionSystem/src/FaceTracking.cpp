/**************************************************************
* Type: .cpp file
* Class Name: CFaceTracking
* Function: track faces via the Kinect
* Create date: 2014.10.16
* Creator： Zhaoqiang Xia (xiazhaoqiang@gmail.com)
* Error Code:
*     1 Succeed
*     0 No result
*    -1 Kinect cannot be initialized
*    -2 Sensor data cannot be read
*    -3 FaceTracker cann't be initialized 
*    -4 FTResult cann't be initialized
*    -5 IFTImage cann't be initialized
***************************************************************/
#include "stdafx.h"
#include "FaceTracking.h"

// 全局变量区

CFaceTracking::CFaceTracking()
{
	isTracked	= false;
}

// 获取彩色数据并显示
void CFaceTracking::DisplayColorCamera()
{
	Mat image(480, 640, CV_8UC3);
	// 1、初始化Kinect
	HRESULT hr =  NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
	if (FAILED(hr)) return;
	//2、定义事件句柄   
	// 创建读取下一帧的信号事件句柄，控制KINECT是否可以开始读取下一帧数据  
	HANDLE nextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );  
	HANDLE colorStreamHandle = NULL; // 保存图像数据流的句柄，用以提取数据   

	//3、打开KINECT设备的彩色图信息通道，并用colorStreamHandle保存该流的句柄，以便于以后读取  
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,   
		0, 2, nextColorFrameEvent, &colorStreamHandle);   
	if( FAILED( hr ) ) //判断是否提取正确   
	{   
		NuiShutdown();   
		return;   
	}  
	namedWindow("colorImage", CV_WINDOW_AUTOSIZE); 

	//4、开始读取彩色图数据   
	while(1)   
	{   
		const NUI_IMAGE_FRAME * pImageFrame = NULL;   

		//4.1、无限等待新的数据，等到后返回  
		if (WaitForSingleObject(nextColorFrameEvent, INFINITE)==0)   
		{   
			//4.2、从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于pImageFrame  
			hr = NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pImageFrame);   
			if (FAILED(hr))  
			{  
				NuiShutdown();  
				return;  
			}  

			INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;  
			NUI_LOCKED_RECT LockedRect;  

			//4.3、提取数据帧到LockedRect，它包括两个数据对象：pitch每行字节数，pBits第一个字节地址  
			//并锁定数据，这样当我们读数据的时候，kinect就不会去修改它  
			pTexture->LockRect(0, &LockedRect, NULL, 0);   
			//4.4、确认获得的数据是否有效  
			if( LockedRect.Pitch != 0 )   
			{   
				//4.5、将数据转换为OpenCV的Mat格式  
				for (int i=0; i<image.rows; i++)   
				{  
					uchar *ptr = image.ptr<uchar>(i);  //第i行的指针  

					//每个字节代表一个颜色信息，直接使用uchar  
					uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;  
					for (int j=0; j<image.cols; j++)   
					{   
						ptr[3*j] = pBuffer[4*j];  //内部数据是4个字节，0-1-2是BGR，第4个现在未使用   
						ptr[3*j+1] = pBuffer[4*j+1];   
						ptr[3*j+2] = pBuffer[4*j+2];   
					}   
				}   
				imshow("colorImage", image); //显示图像   
			}   
			else   
			{   
			}  

			//5、这帧已经处理完了，所以将其解锁  
			pTexture->UnlockRect(0);  
			//6、释放本帧数据，准备迎接下一帧   
			NuiImageStreamReleaseFrame(colorStreamHandle, pImageFrame );   
		}   
		if (cvWaitKey(20) == 'q')   break;   
	}   
	//7、关闭NUI链接   
	NuiShutdown();   
	return ;  
}

// 读取Kinect数据
void CFaceTracking::GetColorData()
{
	const NUI_IMAGE_FRAME * pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame( m_pVideoStreamHandle, 0, &pImageFrame );
	if( FAILED( hr ) ) return;
	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
	if( LockedRect.Pitch != 0 )
	{
		BYTE * pBuffer = (BYTE*) LockedRect.pBits;
		memcpy(pColorFrame->GetBuffer(), PBYTE(LockedRect.pBits), 
			min(pColorFrame->GetBufferSize(), UINT(pTexture->BufferLen())));
		//OpenCV显示彩色视频 for debug
		//Mat temp(COLOR_HEIGHT,COLOR_WIDTH,CV_8UC4,pBuffer);
		//Mat temp2(COLOR_HEIGHT,COLOR_WIDTH,CV_8UC3);
		//cvtColor(temp,temp2,CV_BGRA2BGR);
		//imshow("ColorVideo",temp2);
		//int c = waitKey(1);//按下ESC结束
		////如果在视频界面按下ESC,q,Q都会导致整个程序退出
		//if( c == 27 || c == 'q' || c == 'Q' )
		//{
		//	SetEvent(m_hEvNuiProcessStop);
		//}
	}
	pTexture->UnlockRect(0);
	NuiImageStreamReleaseFrame( m_pVideoStreamHandle, pImageFrame );
}

void CFaceTracking::GetDepthData()
{
	const NUI_IMAGE_FRAME * pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame( m_pDepthStreamHandle, 0, &pImageFrame );
	if( FAILED( hr ) ) return;
	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
	if( LockedRect.Pitch != 0 )
	{
		USHORT * pBuff = (USHORT*) LockedRect.pBits;
		//		pDepthBuffer = pBuff;
		memcpy(pDepthFrame->GetBuffer(), PBYTE(LockedRect.pBits), 
			min(pDepthFrame->GetBufferSize(), UINT(pTexture->BufferLen())));

		for(int i=0;i<DEPTH_HEIGHT*DEPTH_WIDTH;i++)
		{
			BYTE index = pBuff[i]&0x07;
			USHORT realDepth = (pBuff[i]&0xFFF8)>>3;
			BYTE scale = 255 - (BYTE)(256*realDepth/0x0fff);
			DepthBuf[CHANNEL*i] = DepthBuf[CHANNEL*i+1] = DepthBuf[CHANNEL*i+2] = 0;
			switch( index )
			{
			case 0:
				DepthBuf[CHANNEL*i]=scale/2;
				DepthBuf[CHANNEL*i+1]=scale/2;
				DepthBuf[CHANNEL*i+2]=scale/2;
				break;
			case 1:
				DepthBuf[CHANNEL*i]=scale;
				break;
			case 2:
				DepthBuf[CHANNEL*i+1]=scale;
				break;
			case 3:
				DepthBuf[CHANNEL*i+2]=scale;
				break;
			case 4:
				DepthBuf[CHANNEL*i]=scale;
				DepthBuf[CHANNEL*i+1]=scale;
				break;
			case 5:
				DepthBuf[CHANNEL*i]=scale;
				DepthBuf[CHANNEL*i+2]=scale;
				break;
			case 6:
				DepthBuf[CHANNEL*i+1]=scale;
				DepthBuf[CHANNEL*i+2]=scale;
				break;
			case 7:
				DepthBuf[CHANNEL*i]=255-scale/2;
				DepthBuf[CHANNEL*i+1]=255-scale/2;
				DepthBuf[CHANNEL*i+2]=255-scale/2;
				break;
			}
		}
	}
	pTexture->UnlockRect(0);
	NuiImageStreamReleaseFrame( m_pDepthStreamHandle, pImageFrame );
	return;
}

void CFaceTracking::GetSkeletonData()
{
	//
	// has commented all the code snippets for displaying skeleton points
	//
	NUI_SKELETON_FRAME SkeletonFrame;
	Point pt[20]; // save 20 key points of body skeleton, defined by Microsoft
	Mat skeletonMat=Mat(SKELETON_HEIGHT,SKELETON_WIDTH,CV_8UC3,Scalar(0,0,0));
	HRESULT hr = NuiSkeletonGetNextFrame( 0, &SkeletonFrame );
	if( FAILED( hr ) ) return;

	bool bFoundSkeleton = false;
	for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
	{
		if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
		{
			bFoundSkeleton = true;
		}
	}
	// Has skeletons!
	if( bFoundSkeleton )
	{
		NuiTransformSmooth(&SkeletonFrame,NULL);
		for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
		{
			if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
			{
				for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
				{
					float fx,fy;
					NuiTransformSkeletonToDepthImage( SkeletonFrame.SkeletonData[i].SkeletonPositions[j], &fx, &fy );
					pt[j].x = (int) ( fx * SKELETON_WIDTH )/320;
					pt[j].y = (int) ( fy * SKELETON_HEIGHT )/240;
					circle(skeletonMat,pt[j],5,CV_RGB(255,0,0));
				}
				// draw the skeleton points
				cv::line(skeletonMat,pt[NUI_SKELETON_POSITION_HEAD],pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],CV_RGB(0,255,0));
				cv::line(skeletonMat,pt[NUI_SKELETON_POSITION_HAND_RIGHT],pt[NUI_SKELETON_POSITION_WRIST_RIGHT],CV_RGB(0,255,0));
				cv::line(skeletonMat,pt[NUI_SKELETON_POSITION_WRIST_RIGHT],pt[NUI_SKELETON_POSITION_ELBOW_RIGHT],CV_RGB(0,255,0));
				cv::line(skeletonMat,pt[NUI_SKELETON_POSITION_ELBOW_RIGHT],pt[NUI_SKELETON_POSITION_SHOULDER_RIGHT],CV_RGB(0,255,0));
				cv::line(skeletonMat,pt[NUI_SKELETON_POSITION_SHOULDER_RIGHT],pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],CV_RGB(0,255,0));
				cv::line(skeletonMat,pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],pt[NUI_SKELETON_POSITION_SHOULDER_LEFT],CV_RGB(0,255,0));
				cv::line(skeletonMat,pt[NUI_SKELETON_POSITION_SHOULDER_LEFT],pt[NUI_SKELETON_POSITION_ELBOW_LEFT],CV_RGB(0,255,0));
				cv::line(skeletonMat,pt[NUI_SKELETON_POSITION_ELBOW_LEFT],pt[NUI_SKELETON_POSITION_WRIST_LEFT],CV_RGB(0,255,0));
				cv::line(skeletonMat,pt[NUI_SKELETON_POSITION_WRIST_LEFT],pt[NUI_SKELETON_POSITION_HAND_LEFT],CV_RGB(0,255,0));
				// save the coordinates of shoulder and head
				m_hint3D[0].x=SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].x;
				m_hint3D[0].y=SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].y;
				m_hint3D[0].z=SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].z;
				m_hint3D[1].x=SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].x;
				m_hint3D[1].y=SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y;
				m_hint3D[1].z=SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].z;
			}
		}
	}
	return;
}

void CFaceTracking::KinectRecData()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextVideoFrameEvent, 0))
		GetColorData();
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextDepthFrameEvent, 0))
		GetDepthData();
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0))
		GetSkeletonData();
	return;
}

// 初始化人脸跟踪类
int CFaceTracking::KinectFTInitialize()
{
	// 初始化NUI
	//HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH|NUI_INITIALIZE_FLAG_USES_COLOR);
	HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX|NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_SKELETON);
	if( hr != S_OK ) return -1;

	// 打开KINECT设备的彩色、深度图以及骨骼信息通道
	m_hNextVideoFrameEvent	= CreateEvent( NULL, TRUE, FALSE, NULL );
	m_pVideoStreamHandle	= NULL;
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR,NUI_IMAGE_RESOLUTION_640x480, 0, 2, m_hNextVideoFrameEvent, &m_pVideoStreamHandle);
	if( FAILED( hr ) ) return -2;
	m_hNextDepthFrameEvent	= CreateEvent( NULL, TRUE, FALSE, NULL );
	m_pDepthStreamHandle	= NULL;
	hr = NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240, 0, 2, m_hNextDepthFrameEvent, &m_pDepthStreamHandle);
	if( FAILED( hr ) ) return -2;
	m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	hr = NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 
		NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE|NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT);
	if( FAILED( hr ) ) return -2;

	m_hEvNuiProcessStop = CreateEvent(NULL,TRUE,FALSE,NULL); // 用于结束的事件对象

	///////////////////////////////////////////////
	m_hint3D[0] = FT_VECTOR3D(0, 0, 0);
	m_hint3D[1] = FT_VECTOR3D(0, 0, 0);

	// 创建Kinect SDK的Face tracker
	m_pFT = FTCreateFaceTracker();
	if(!m_pFT) return -3;// Handle errors

	//初始化人脸跟踪所需要的数据数据
	cameraConfig.Height = COLOR_HEIGHT; cameraConfig.Width = COLOR_WIDTH;
	cameraConfig.FocalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS; // width, height, focal length
	depthConfig.Height = DEPTH_HEIGHT; depthConfig.Width = DEPTH_WIDTH;
	depthConfig.FocalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;

	//IFTFaceTracker的初始化
	hr = m_pFT->Initialize(&cameraConfig, &depthConfig, NULL, NULL);
	if( FAILED(hr) ) return -3;// Handle errors

	// Create IFTResult to hold a face tracking result
	m_pFTResult = NULL;
	hr = m_pFT->CreateFTResult(&m_pFTResult);
	if(FAILED(hr)) return -4;

	//申请内存空间
	pColorFrame = FTCreateImage();
	pDepthFrame = FTCreateImage();
	pColorDisplay = FTCreateImage();
	if(!pColorFrame || !pDepthFrame) return -5;// Handle errors
	pColorDisplay->Allocate(COLOR_WIDTH, COLOR_HEIGHT, FTIMAGEFORMAT_UINT8_B8G8R8X8); // width, height
	hr = pColorFrame->Allocate(COLOR_WIDTH, COLOR_HEIGHT, FTIMAGEFORMAT_UINT8_B8G8R8X8);
	if (FAILED(hr)) return hr;
	hr = pDepthFrame->Allocate(DEPTH_WIDTH, DEPTH_HEIGHT, FTIMAGEFORMAT_UINT16_D13P3);
	if (FAILED(hr)) return hr;

	//填充FT_SENSOR_DATA结构
	POINT point;
	point.x = 0; point.y = 0;
	m_sensorData.ZoomFactor	= 1.0f;
	m_sensorData.ViewOffset	= point;

	isTracked = false;

	Sleep(20);

	return 1;
}

// 跟踪
void CFaceTracking::Track(Mat& resMat)
{
	// 读取数据
	KinectRecData();
	m_sensorData.pVideoFrame = pColorFrame;
	m_sensorData.pDepthFrame = pDepthFrame;
	// 跟踪人脸
	HRESULT hr;
	if(!isTracked)
	{
		// 初次检测人脸会耗费较多cpu计算资源
		hr = m_pFT->StartTracking(&m_sensorData, NULL, m_hint3D, m_pFTResult);
		if(SUCCEEDED(hr) && SUCCEEDED(m_pFTResult->GetStatus()))
			isTracked = true;
		else
			isTracked = false;
	}
	else
	{
		// 继续追踪，它一般使用一个已大概知晓的人脸模型，所以它的调用不会消耗多少cpu计算
		hr = m_pFT->ContinueTracking(&m_sensorData, m_hint3D, m_pFTResult);
		if(FAILED(hr) || FAILED (m_pFTResult->GetStatus())) isTracked = false;
	}
	if(isTracked)
	{
		IFTModel*	ftModel;
		HRESULT		hr = m_pFT->GetFaceModel(&ftModel);
		FLOAT*		pSU = NULL;
		UINT		numSU;
		BOOL		suConverged;
		m_pFT->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);
		POINT viewOffset = {0, 0};
		pColorFrame->CopyTo(pColorDisplay,NULL,0,0);
		//hr = VisualizeFaceModel(pColorDisplay, ftModel, &cameraConfig, pSU, 1.0, viewOffset, m_pFTResult, 0x00FFFF00);
		hr = VisualizeFaceComponents(pColorDisplay, ftModel, &cameraConfig, pSU, 1.0, viewOffset, m_pFTResult, 0x0000FF00);
		if(FAILED(hr)) return;
		Mat tempMat(480,640,CV_8UC4,pColorDisplay->GetBuffer());
		// 从BGRA转换为BGR
		cvtColor(tempMat,resMat,CV_BGRA2BGR);
	}
	else
	{
		m_pFTResult->Reset();
		//pColorFrame->CopyTo(pColorDisplay,NULL,0,0);
		Mat tempMat(480,640,CV_8UC4,pColorFrame->GetBuffer());
		cvtColor(tempMat,resMat,CV_BGRA2BGR);
		//imshow("faceTracking",tempMat);
		//int c = waitKey(1);//按下ESC结束
		//if(m_hEvNuiProcessStop!=NULL)
		//{
		//	if( c == 27 || c == 'q' || c == 'Q' )
		//	{
		//		SetEvent(m_hEvNuiProcessStop);
		//	}
		//}
	}

}

// 获取人脸表情
int CFaceTracking::GetFacialExpression(Mat* pColorDisplay)
{
	if( SUCCEEDED(m_pFTResult->GetStatus()) )
	{
		int reValue = 0;
		FLOAT *pAUs;
		UINT auCount;
		m_pFTResult->GetAUCoefficients(&pAUs,&auCount);
		for(int i =0;i<(int)auCount;i++)
		{
			if(pAUs[i]>0.4||pAUs[i]<-0.4){
				reValue = i+1;
				if(pColorDisplay != NULL){
					imwrite(_T("./data/expressive.jpg"),*pColorDisplay);
					putText( *pColorDisplay, "Expressive",Point( 25, 50 ),FONT_HERSHEY_SIMPLEX,  2,  Scalar( 255, 0, 0 ) );  
				}
				return reValue;
			}
		}
		return 0;
	}
	else return 0;
}

// 释放资源
void CFaceTracking::KinectFTRelease()
{
	// Clean up
	if(m_pFTResult) m_pFTResult->Release();
	if(pColorFrame) pColorFrame->Release();
	if(pDepthFrame) pDepthFrame->Release();
	if(m_pFT) m_pFT->Release();
	// turn off the kinect
	NuiShutdown();
	return;
}

// 绘制人脸模型
HRESULT CFaceTracking::VisualizeFaceModel(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
	FLOAT zoomFactor, POINT viewOffset, IFTResult* pAAMRlt, UINT32 color)
{
	if (!pColorImg || !pModel || !pCameraConfig || !pSUCoef || !pAAMRlt)
	{
		return E_POINTER;
	}

	HRESULT hr = S_OK;
	UINT vertexCount = pModel->GetVertexCount();
	FT_VECTOR2D* pPts2D = reinterpret_cast<FT_VECTOR2D*>(_malloca(sizeof(FT_VECTOR2D) * vertexCount));
	if (pPts2D)
	{
		FLOAT *pAUs;
		UINT auCount;
		hr = pAAMRlt->GetAUCoefficients(&pAUs, &auCount);
		if (SUCCEEDED(hr))
		{
			//rotationXYZ人脸旋转角度！
			FLOAT scale, rotationXYZ[3], translationXYZ[3];
			hr = pAAMRlt->Get3DPose(&scale, rotationXYZ, translationXYZ);
			if (SUCCEEDED(hr))
			{
				hr = pModel->GetProjectedShape(pCameraConfig, zoomFactor, viewOffset, pSUCoef, pModel->GetSUCount(), pAUs, auCount, 
					scale, rotationXYZ, translationXYZ, pPts2D, vertexCount);
				//这里获取了vertexCount个面部特征点，存放在pPts2D指针数组中
				if (SUCCEEDED(hr))
				{
					POINT* p3DMdl   = reinterpret_cast<POINT*>(_malloca(sizeof(POINT) * vertexCount));
					if (p3DMdl)
					{
						for (UINT i = 0; i < vertexCount; ++i)
						{
							p3DMdl[i].x = LONG(pPts2D[i].x + 0.5f);
							p3DMdl[i].y = LONG(pPts2D[i].y + 0.5f);
						}

						FT_TRIANGLE* pTriangles;
						UINT triangleCount;
						hr = pModel->GetTriangles(&pTriangles, &triangleCount);
						if (SUCCEEDED(hr))
						{
							struct EdgeHashTable
							{
								UINT32* pEdges;
								UINT edgesAlloc;

								void Insert(int a, int b) 
								{
									UINT32 v = (min(a, b) << 16) | max(a, b);
									UINT32 index = (v + (v << 8)) * 49157, i;
									for (i = 0; i < edgesAlloc - 1 && pEdges[(index + i) & (edgesAlloc - 1)] && v != pEdges[(index + i) & (edgesAlloc - 1)]; ++i)
									{
									}
									pEdges[(index + i) & (edgesAlloc - 1)] = v;
								}
							} eht;

							eht.edgesAlloc = 1 << UINT(log(2.f * (1 + vertexCount + triangleCount)) / log(2.f));
							eht.pEdges = reinterpret_cast<UINT32*>(_malloca(sizeof(UINT32) * eht.edgesAlloc));
							if (eht.pEdges)
							{
								ZeroMemory(eht.pEdges, sizeof(UINT32) * eht.edgesAlloc);
								for (UINT i = 0; i < triangleCount; ++i)
								{ 
									eht.Insert(pTriangles[i].i, pTriangles[i].j);
									eht.Insert(pTriangles[i].j, pTriangles[i].k);
									eht.Insert(pTriangles[i].k, pTriangles[i].i);
								}
								for (UINT i = 0; i < eht.edgesAlloc; ++i)
								{
									if(eht.pEdges[i] != 0)
									{
										pColorImg->DrawLine(p3DMdl[eht.pEdges[i] >> 16], p3DMdl[eht.pEdges[i] & 0xFFFF], color, 1);
									}
								}
								_freea(eht.pEdges);
							}

							// Render the face rect in magenta
							RECT rectFace;
							hr = pAAMRlt->GetFaceRect(&rectFace);
							if (SUCCEEDED(hr))
							{
								POINT leftTop = {rectFace.left, rectFace.top};
								POINT rightTop = {rectFace.right - 1, rectFace.top};
								POINT leftBottom = {rectFace.left, rectFace.bottom - 1};
								POINT rightBottom = {rectFace.right - 1, rectFace.bottom - 1};
								UINT32 nColor = 0xff00ff;
								SUCCEEDED(hr = pColorImg->DrawLine(leftTop, rightTop, nColor, 1)) &&
									SUCCEEDED(hr = pColorImg->DrawLine(rightTop, rightBottom, nColor, 1)) &&
									SUCCEEDED(hr = pColorImg->DrawLine(rightBottom, leftBottom, nColor, 1)) &&
									SUCCEEDED(hr = pColorImg->DrawLine(leftBottom, leftTop, nColor, 1));
							}
						}

						_freea(p3DMdl); 
					}
					else
					{
						hr = E_OUTOFMEMORY;
					}
				}
			}
		}
		_freea(pPts2D);
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}
	return hr;
}

// 绘制人脸主要区域
HRESULT CFaceTracking::VisualizeFaceComponents(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
	FLOAT zoomFactor, POINT viewOffset, IFTResult* pAAMRlt, UINT32 color)
{
	if (!pColorImg || !pModel || !pCameraConfig || !pSUCoef || !pAAMRlt)
	{
		return E_POINTER;
	}

	HRESULT hr = S_OK;
	FT_VECTOR2D* pLandmarks;
	UINT landmarksCount;
	hr = pAAMRlt->Get2DShapePoints(&pLandmarks,&landmarksCount);

	UINT lineWidth = 2;
	UINT i = 0;
	if (SUCCEEDED(hr))
	{
		POINT startPoint, endPoint;
		// draw the right eye
		for (i = 0; i < 7; i++)
		{
			startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
			endPoint.x = LONG(pLandmarks[i+1].x+0.5f); endPoint.y = LONG(pLandmarks[i+1].y+0.5f);
			SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		}
		startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
		endPoint.x = LONG(pLandmarks[0].x+0.5f); endPoint.y = LONG(pLandmarks[0].y+0.5f);
		SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		// draw the left eye
		for (i = 8; i < 15; i++)
		{
			startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
			endPoint.x = LONG(pLandmarks[i+1].x+0.5f); endPoint.y = LONG(pLandmarks[i+1].y+0.5f);
			SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		}
		startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
		endPoint.x = LONG(pLandmarks[8].x+0.5f); endPoint.y = LONG(pLandmarks[8].y+0.5f);
		SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		// draw the right brow
		for (i = 16; i < 25; i++)
		{
			startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
			endPoint.x = LONG(pLandmarks[i+1].x+0.5f); endPoint.y = LONG(pLandmarks[i+1].y+0.5f);
			SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		}
		startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
		endPoint.x = LONG(pLandmarks[16].x+0.5f); endPoint.y = LONG(pLandmarks[16].y+0.5f);
		SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		// draw the left brow
		for (i = 26; i < 35; i++)
		{
			startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
			endPoint.x = LONG(pLandmarks[i+1].x+0.5f); endPoint.y = LONG(pLandmarks[i+1].y+0.5f);
			SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		}
		startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
		endPoint.x = LONG(pLandmarks[26].x+0.5f); endPoint.y = LONG(pLandmarks[26].y+0.5f);
		SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		// draw the nose
		for (i = 36; i < 47; i++)
		{
			startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
			endPoint.x = LONG(pLandmarks[i+1].x+0.5f); endPoint.y = LONG(pLandmarks[i+1].y+0.5f);
			SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		}
		// draw the outer mouth
		for (i =48; i < 59; i++)
		{
			startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
			endPoint.x = LONG(pLandmarks[i+1].x+0.5f); endPoint.y = LONG(pLandmarks[i+1].y+0.5f);
			SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		}
		startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
		endPoint.x = LONG(pLandmarks[48].x+0.5f); endPoint.y = LONG(pLandmarks[48].y+0.5f);
		SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		// draw the inner mouth
		for (i = 60; i < 67; i++)
		{
			startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
			endPoint.x = LONG(pLandmarks[i+1].x+0.5f); endPoint.y = LONG(pLandmarks[i+1].y+0.5f);
			SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;
		}
		startPoint.x = LONG(pLandmarks[i].x+0.5f); startPoint.y = LONG(pLandmarks[i].y+0.5f);
		endPoint.x = LONG(pLandmarks[60].x+0.5f); endPoint.y = LONG(pLandmarks[60].y+0.5f);
		SUCCEEDED(hr = pColorImg->DrawLine(startPoint, endPoint, color, lineWidth)) ;


		// Render the face rect in magenta
		RECT rectFace;
		hr = pAAMRlt->GetFaceRect(&rectFace);
		if (SUCCEEDED(hr))
		{
			POINT leftTop = {rectFace.left, rectFace.top};
			POINT rightTop = {rectFace.right - 1, rectFace.top};
			POINT leftBottom = {rectFace.left, rectFace.bottom - 1};
			POINT rightBottom = {rectFace.right - 1, rectFace.bottom - 1};
			UINT32 nColor = 0xff00ff;
			SUCCEEDED(hr = pColorImg->DrawLine(leftTop, rightTop, nColor, 1)) &&
				SUCCEEDED(hr = pColorImg->DrawLine(rightTop, rightBottom, nColor, 1)) &&
				SUCCEEDED(hr = pColorImg->DrawLine(rightBottom, leftBottom, nColor, 1)) &&
				SUCCEEDED(hr = pColorImg->DrawLine(leftBottom, leftTop, nColor, 1));
		}
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}
	return hr;
}