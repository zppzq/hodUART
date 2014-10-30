// hodUARTDlg.h : header file
//

#if !defined(AFX_HODUARTDLG_H__5F325977_D005_4113_8E83_C6EAB319C4DC__INCLUDED_)
#define AFX_HODUARTDLG_H__5F325977_D005_4113_8E83_C6EAB319C4DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CHodUARTDlg dialog
typedef struct
{
    unsigned char head;             //=68H
	unsigned char DeviceType;
	unsigned char MeterAddr[7];     /*HOD后3个字节是厂商代码*/
	unsigned char ctrl;             //控制域C
	unsigned char len;
    unsigned char DI[2];
    unsigned char SER;
	unsigned char data[70];
} tsCJT188Pack;

class CHodUARTDlg : public CDialog
{
// Construction
public:
	CHodUARTDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CHodUARTDlg)
	enum { IDD = IDD_HODUART_DIALOG };
	CString	m_FilePath;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHodUARTDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
	BSTR CHodUARTDlg::TraversalDirectory(LPCTSTR Path);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CHodUARTDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonOpen();
	afx_msg void OnButtonSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HODUARTDLG_H__5F325977_D005_4113_8E83_C6EAB319C4DC__INCLUDED_)
