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


#if !defined(AFX_DELDIALOG_H__1609583A_2D27_49AB_9289_E2C48F174B69__INCLUDED_)
#define AFX_DELDIALOG_H__1609583A_2D27_49AB_9289_E2C48F174B69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DelDialog.h : header file
//

// Define registry key names that saves the checkbox states for the Delete Joystick dialog
#define	REMOVE_DRIVER_VALUE	_T("RemoveDriver")
#define	REMOVE_DINPUT_VALUE	_T("RemoveDirectInput")

#include <atlbase.h>

/////////////////////////////////////////////////////////////////////////////
// CDelDialog dialog

class CDelDialog : public CDialog
{
// Construction
public:
	CDelDialog(CWnd* pParent = NULL);   // standard constructor

	virtual BOOL OnInitDialog();

	int		DelIndex;
	CRegKey	RegKey;

// Dialog Data
	//{{AFX_DATA(CDelDialog)
	enum { IDD = IDD_DELDIALOG };
	CButton	m_RemoveDInputCheck;
	CStatic	m_RemoveText;
	CButton	m_RemoveDriverCheck;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDelDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDelDialog)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DELDIALOG_H__1609583A_2D27_49AB_9289_E2C48F174B69__INCLUDED_)
