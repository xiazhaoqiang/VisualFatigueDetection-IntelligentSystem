
// VisualFatigueDetectionSystemDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VisualFatigueDetectionSystem.h"
#include "VisualFatigueDetectionSystemDlg.h"
#include "afxdialogex.h"
#include <string>

using namespace cv;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 全局变量区
volatile bool isRunning;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVisualFatigueDetectionSystemDlg 对话框




CVisualFatigueDetectionSystemDlg::CVisualFatigueDetectionSystemDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVisualFatigueDetectionSystemDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_statusBar = _T("显示信息");
}

void CVisualFatigueDetectionSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_statusBar);
}

BEGIN_MESSAGE_MAP(CVisualFatigueDetectionSystemDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_OPENDEVICE, &CVisualFatigueDetectionSystemDlg::OnBnClickedBtnOpendevice)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CVisualFatigueDetectionSystemDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_FTSTART, &CVisualFatigueDetectionSystemDlg::OnBnClickedBtnFtstart)
	ON_BN_CLICKED(IDC_BTN_FTSTOP, &CVisualFatigueDetectionSystemDlg::OnBnClickedBtnFtstop)
	ON_MESSAGE(WM_UPDATEDATA, &CVisualFatigueDetectionSystemDlg::OnUpdatedata)
END_MESSAGE_MAP()


// CVisualFatigueDetectionSystemDlg 消息处理程序

BOOL CVisualFatigueDetectionSystemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	GetDlgItem(IDC_BTN_FTSTOP)->EnableWindow(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CVisualFatigueDetectionSystemDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVisualFatigueDetectionSystemDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CVisualFatigueDetectionSystemDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*****************************************************
* 多线程函数 
******************************************************/
DWORD WINAPI ThreadFaceTracking (LPVOID pParam) 
{
	//
	CVisualFatigueDetectionSystemDlg* pDlg = (CVisualFatigueDetectionSystemDlg*)pParam;
	Mat resMat(480,640,CV_8UC3);
	string str;

	CFaceTracking ft;
	if(ft.KinectFTInitialize()<0){
		str=_T("未连接设备或无法读取设备数据");
		pDlg->SetStatusContent(str);
		pDlg->SendMessage(WM_UPDATEDATA,0,0);
		return 0;
	}
	int type = -1;
	while(isRunning)
	{
		ft.Track(resMat);
		//type = ft.GetFacialExpression();
		type = ft.GetFacialExpression(&resMat);
		// display the results: face model and face expression
		IplImage img = resMat;
		pDlg->DrawImgtoDC(&img,IDC_SHOWIMG);
		if(type == 0) str=_T("Neutral");
		else if(type > 0) str = _T("Expressive");
		else str=_T("No Track Result");
		pDlg->SetStatusContent(str);
		pDlg->SendMessage(WM_UPDATEDATA,0,0);
	}
	ft.KinectFTRelease();
	resMat = Mat::zeros(480,640,CV_8UC3);
	IplImage img = resMat;
	pDlg->DrawImgtoDC(&img,IDC_SHOWIMG);
	str=_T("显示信息");
	pDlg->SetStatusContent(str);
	pDlg->SendMessage(WM_UPDATEDATA,0,0);
	return 1;
}
/***************************************************
* 消息响应函数
****************************************************/
// 打开设备
void CVisualFatigueDetectionSystemDlg::OnBnClickedBtnOpendevice()
{

}

// 退出系统
void CVisualFatigueDetectionSystemDlg::OnBnClickedBtnClose()
{
	OnCancel();
}


// 人脸跟踪开启
void CVisualFatigueDetectionSystemDlg::OnBnClickedBtnFtstart()
{
	GetDlgItem(IDC_BTN_FTSTART)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_FTSTOP)->EnableWindow(TRUE);
	// 开启线程
	DWORD threadID = 1;
	isRunning = true;
	m_hTreadFT = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFaceTracking,
		this,0,&threadID);
}


// 人脸跟踪关闭
void CVisualFatigueDetectionSystemDlg::OnBnClickedBtnFtstop()
{
	 GetDlgItem(IDC_BTN_FTSTART)->EnableWindow(TRUE);
	 GetDlgItem(IDC_BTN_FTSTOP)->EnableWindow(FALSE);
	 // 关闭线程
	 isRunning = false; // 给出线程结束的信号
	 CloseHandle(m_hTreadFT); // 释放句柄
}

// 更新数据
afx_msg LRESULT CVisualFatigueDetectionSystemDlg::OnUpdatedata(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return 0;
}


/***************************************************
* 成员函数
****************************************************/
void CVisualFatigueDetectionSystemDlg::DrawImgtoDC(IplImage * img, UINT ID)
{
	// get a dc
	CDC* pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	CvvImage cvImg;
	cvImg.CopyOf(img);
	cvImg.DrawToHDC(hDC,&rect);
	ReleaseDC(pDC);
}


void CVisualFatigueDetectionSystemDlg::SetStatusContent(string str)
{
	m_statusBar.Format("%s",str.c_str());
}
