// SetVirtualJoyState.h : main header file for the SETVIRTUALJOYSTATE application
//

#if !defined(AFX_SETVIRTUALJOYSTATE_H__4264AA82_5EB7_46DC_BE28_E4DC66B197E6__INCLUDED_)
#define AFX_SETVIRTUALJOYSTATE_H__4264AA82_5EB7_46DC_BE28_E4DC66B197E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSetVirtualJoyStateApp:
// See SetVirtualJoyState.cpp for the implementation of this class
//

class CSetVirtualJoyStateApp : public CWinApp
{
public:
	CSetVirtualJoyStateApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetVirtualJoyStateApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSetVirtualJoyStateApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETVIRTUALJOYSTATE_H__4264AA82_5EB7_46DC_BE28_E4DC66B197E6__INCLUDED_)
