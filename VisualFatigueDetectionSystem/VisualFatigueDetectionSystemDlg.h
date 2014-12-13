
// VisualFatigueDetectionSystemDlg.h : 头文件
//

#pragma once

#include "src\CvvImage.h"
#include "src\FaceTracking.h"

#define WM_UPDATEDATA (WM_USER+100)

// CVisualFatigueDetectionSystemDlg 对话框
class CVisualFatigueDetectionSystemDlg : public CDialogEx
{
// 构造
public:
	CVisualFatigueDetectionSystemDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_VISUALFATIGUEDETECTIONSYSTEM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOpendevice();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnFtstart();
	afx_msg void OnBnClickedBtnFtstop();
	afx_msg LRESULT OnUpdatedata(WPARAM wParam, LPARAM lParam);
private:
	// 保存人脸跟踪函数的线程句柄
	HANDLE m_hTreadFT;
public:
	void DrawImgtoDC(IplImage * img, UINT ID);
	CString m_statusBar;
	void SetStatusContent(string str);
	
};
