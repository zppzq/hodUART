// hodUARTDlg.cpp : implementation file
//

#include "stdafx.h"
#include "hodUART.h"
#include "hodUARTDlg.h"
#include "serial.h"
#include "TTLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

SerialPort *g_SerialPort;
//CTTLog	   *g_Log;
extern CTTLog  g_log;
HANDLE hThread;
DWORD ThreadID;
volatile BOOL m_bRun;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHodUARTDlg dialog

CHodUARTDlg::CHodUARTDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHodUARTDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHodUARTDlg)
	m_FilePath = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHodUARTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHodUARTDlg)
	DDX_Text(pDX, IDC_EDIT_FILE_PATH, m_FilePath);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHodUARTDlg, CDialog)
	//{{AFX_MSG_MAP(CHodUARTDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHodUARTDlg message handlers

BOOL CHodUARTDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHodUARTDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHodUARTDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHodUARTDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CheckCJT188Pack(unsigned char *ptr)
{
	int TotalLen, i;
	unsigned char sum;
	unsigned char *buf = ptr + 2;
	tsCJT188Pack *pack = (tsCJT188Pack *)buf;

	if (pack->head != 0x68)
	{
		return FALSE;
	}
	if (pack->len > 80)
	{
		return FALSE;
	}
	if (buf[pack->len + 12] != 0x16)
	{
		return FALSE;
	}
	if (0 != (pack->ctrl & 0x80))
	{
		return FALSE;
	}
	
	TotalLen = pack->len + 13;
	
	sum = 0;
	for (i = 0; i < TotalLen - 2; i++)
	{
		sum += buf[i];
	}
	if (buf[TotalLen - 2] != sum)
	{
		return FALSE;
	}

	return TRUE;
}

void UARTThreadFunc(LPVOID lpParam)
{
	CTime time;
	CString strTime;
	unsigned char *ptr;
	
	CHodUARTDlg *pDlg = (CHodUARTDlg*)lpParam;

	int len, i;
	char count = 0, sum;
	unsigned char recvBuf[100] = {0};
	unsigned char sendBuf[] =	 {0xFE, 0xFE, 0x68, 0x20, 0x20, 0x04, 0x33, 0x12, 0x68, 0x79, 0x72, 0x81, 0x2E, 0x1F, 0x90, 0x01, 
								  0x18, 0x06, 0x41, 0x01, 0x0B,		//结算日热量，	offset:16
								  0x18, 0x06, 0x41, 0x01, 0x0B,		//当前热量		offset:21
								  0x00, 0x00, 0x00, 0x00, 0x17,		//热功率		offset:26
								  0x00, 0x00, 0x00, 0x00, 0x35,		//流量			offset:31
								  0x00, 0x00, 0x00, 0x00, 0x29,		//累积流量		offset:36
								  0x59, 0x25, 0x00,					//供水温度		offset:39
								  0x09, 0x26, 0x00, 				//回水温度		offset:42
								  0x00, 0x00, 0x00, 				//累积工作时间  offset:45
								  0x11, 0x50, 0x10, 0x04, 0x09, 0x13, 0x20, //实时时间offset:48
								  0x00, 0x80,						//状态			offset:55
								  0xCC, 0x16};

	m_bRun=TRUE;

	while(m_bRun)
	{
		len = g_SerialPort->Read(recvBuf, sizeof(recvBuf), 10000);
//		g_SerialPort->Write(sendBuf, sizeof(sendBuf));
		if (len > 5)
		{
//			AfxMessageBox("接收到数据");
//			g_log.Write(CTTLog::ERR, recvBuf);
			if (!CheckCJT188Pack(recvBuf))
			{
				continue;
			}

			memcpy(sendBuf, recvBuf, 11);
			sendBuf[27] = ((count / 10) << 4) | (count % 10);
			sendBuf[32] = ((count / 10) << 4) | (count % 10);
			sendBuf[37] = ((count / 10) << 4) | (count % 10);
			
			ptr = recvBuf;
			while (*ptr != 0x68)
			{
				ptr++;
			}

			ptr++;
			ptr++;

			memcpy(sendBuf + 36, ptr, 2);

			count++;
			if (count >= 100)
			{
				count = 0;
			}

			sum = 0;
			for (i = 2; i < sizeof(sendBuf) - 2; i++)
			{
				sum += sendBuf[i];
			}
			sendBuf[sizeof(sendBuf) - 2] = sum;

			len = sizeof(sendBuf);
			g_SerialPort->Write(sendBuf, len, 1000);
//			sprintf((char *)recvBuf, "Sum = %02X", sum);
//			AfxMessageBox((char *)recvBuf);
		}
	}
}


void CHodUARTDlg::OnButtonOpen() 
{
	// TODO: Add your control notification handler code here
	g_SerialPort = new class SerialPort;
//	g_Log		 = new class CTTLog;

	if (!g_SerialPort->Open("COM6", 4800, 8, 2, 'E'))
	{
		AfxMessageBox("串口打开失败!");
		return;
	}
	g_log.SetLogPath("E:\\TestMBus");
	g_log.SetLogParam("E:\\TestMBus", "20130712.txt", CTTLog::INFO);
	g_log.Write(CTTLog::INFO, "fdsa");
	
	hThread=CreateThread(NULL,
						0,
						(LPTHREAD_START_ROUTINE)UARTThreadFunc,
						this,
						0,
						&ThreadID);
}

void CHodUARTDlg::OnButtonSave() 
{
	// TODO: Add your control notification handler code here
/*	CFileDialog dlg(FALSE,//TRUE是创建打开文件对话框，FALSE则创建的是保存文件对话框 
                    ".txt",//默认的打开文件的类型 
                    NULL,//默认打开的文件名 
                    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,//打开只读文件 
                    "文本文件(*.txt)|*.txt|所有文件 (*.*)|*.*||");//所有可以打开的文件类型
	if(dlg.DoModal()==IDOK)   
	{
		m_FilePath = dlg.GetPathName();////////取出文件路径 
		UpdateData(FALSE);
	}*/
	TraversalDirectory("E:\\FLASH");
}

BSTR Second(LPCTSTR Path, LPCTSTR Dir) 
{
	CString strResult;
	// TODO: Add your dispatch handler code here
	if (Path == NULL)
	{
		return NULL;
	}

	char szFind[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	strcpy(szFind,Path);
	strcat(szFind,"\\*.*");
	HANDLE hFind=::FindFirstFile(szFind,&FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)
		return NULL;
	while(TRUE)
	{
		if(FindFileData.dwFileAttributes
		   &FILE_ATTRIBUTE_DIRECTORY)
		{
			if(FindFileData.cFileName[0]!='.')
			{

			}
		}
		else
		{
//			cout << FindFileData.cFileName;
			strResult += Dir;strResult += "\\";
			strResult += FindFileData.cFileName;
			strResult += "/";
		}
		if(!FindNextFile(hFind,&FindFileData))
			break;
	}
	FindClose(hFind);

	return strResult.AllocSysString();
}

BSTR CHodUARTDlg::TraversalDirectory(LPCTSTR Path) 
{
	CString strResult;
	// TODO: Add your dispatch handler code here
	if (Path == NULL)
	{
		return NULL;
	}

	char szFind[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	strcpy(szFind,Path);
	strcat(szFind,"\\*.*");
	HANDLE hFind=::FindFirstFile(szFind,&FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)
		return NULL;
	while(TRUE)
	{
		if(FindFileData.dwFileAttributes
		   &FILE_ATTRIBUTE_DIRECTORY)
		{
			if(FindFileData.cFileName[0]!='.')
			{
				strcpy(szFind,Path);
				strcat(szFind,"\\");
				strcat(szFind,FindFileData.cFileName);
				strResult += Second(szFind, FindFileData.cFileName);
			}
		}
		else
		{
//			cout << FindFileData.cFileName;
			strResult += FindFileData.cFileName;
			strResult += "/";
		}
		if(!FindNextFile(hFind,&FindFileData))
			break;
	}
	FindClose(hFind);

	return strResult.AllocSysString();
}

/*	unsigned char sendBuf[] =	 {0x68, 0x20,						//帧头
								  0x20, 0x04, 0x33, 0x12,			//表号
								  0x68, 0x79, 0x72,					//厂家编码
								  0x81,								//控制码
								  0x2E,								//数据域长度
								  0x1F, 0x90, 0x01,					//DI和SER
								  0x18, 0x06, 0x41, 0x01, 0x0B,		//结算日热量，	offset:16
								  0x18, 0x06, 0x41, 0x01, 0x0B,		//当前热量		offset:21
								  0x12, 0x34, 0x56, 0x78, 0x17,		//热功率		offset:26
								  0x12, 0x34, 0x56, 0x78, 0x35,		//流量			offset:31
								  0x12, 0x34, 0x56, 0x78, 0x29,		//累积流量		offset:36
								  0x59, 0x25, 0x00,					//供水温度		offset:39
								  0x09, 0x26, 0x00, 				//回水温度		offset:42
								  0x12, 0x34, 0x56, 				//累积工作时间  offset:45
								  0x11, 0x50, 0x10, 0x04, 0x09, 0x13, 0x20, //实时时间offset:48
								  0x00, 0x00,						//状态			offset:55
								  0xCC, 0x16};



								  0x68, 0x20,						//帧头
								  0x20, 0x04, 0x33, 0x12,			//表号
								  0x68, 0x79, 0x72,					//厂家编码
								  0x01,								//控制码
								  0x03,								//数据域长度
								  0x1F, 0x90, 0x01,					//DI和SER
								  0xF8, 0x16};						//校验和和帧尾
*/