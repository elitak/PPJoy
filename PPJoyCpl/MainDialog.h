/***************************************************************************
 *   PPJoy Virtual Joystick for Microsoft Windows                          *
 *   Copyright (C) 2011 Deon van der Westhuysen                            *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 ***************************************************************************/


#include "afxwin.h"
#if !defined(AFX_MAINDIALOG_H__204A0C8F_4115_4AD2_9FF3_515E159E46B4__INCLUDED_)
#define AFX_MAINDIALOG_H__204A0C8F_4115_4AD2_9FF3_515E159E46B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MainDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMainDialog dialog

class CMainDialog : public CDialog
{
// Construction
public:
	CMainDialog(CWnd* pParent = NULL);   // standard constructor

	virtual BOOL OnInitDialog();
	int RefreshJoystickList (void);

// Dialog Data
	//{{AFX_DATA(CMainDialog)
	enum { IDD = IDD_MAINDIALOG };
	CStatic m_MainWindowCaption;
	CButton	m_OptionsButton;
	CButton	m_TimingButton;
	CButton	m_MappingButton;
	CButton	m_DelButton;
	CButton	m_AddButton;
	CListBox	m_JoyList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainDialog)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMainDialog)
	afx_msg void OnAddbutton();
	afx_msg void OnSelchangeJoylist();
	afx_msg void OnDelbutton();
	afx_msg void OnMapbutton();
	afx_msg void OnTimingbutton();
	afx_msg void OnOptionsbutton();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINDIALOG_H__204A0C8F_4115_4AD2_9FF3_515E159E46B4__INCLUDED_)
