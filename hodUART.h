// hodUART.h : main header file for the HODUART application
//

#if !defined(AFX_HODUART_H__4CF18575_815D_491E_8117_7773E613C075__INCLUDED_)
#define AFX_HODUART_H__4CF18575_815D_491E_8117_7773E613C075__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CHodUARTApp:
// See hodUART.cpp for the implementation of this class
//

class CHodUARTApp : public CWinApp
{
public:
	CHodUARTApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHodUARTApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHodUARTApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HODUART_H__4CF18575_815D_491E_8117_7773E613C075__INCLUDED_)
