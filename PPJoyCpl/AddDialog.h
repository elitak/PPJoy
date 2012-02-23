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


#if !defined(AFX_ADDDIALOG_H__7BFF1330_606B_4C7E_B782_C558578BAE44__INCLUDED_)
#define AFX_ADDDIALOG_H__7BFF1330_606B_4C7E_B782_C558578BAE44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDialog.h : header file
//

#include "JoyBus.h"

/////////////////////////////////////////////////////////////////////////////
// CAddDialog dialog

class CAddDialog : public CDialog
{
// Construction
public:
	int VerifyInterruptSetup (UCHAR LPTNumber);
	CAddDialog(CWnd* pParent = NULL);   // standard constructor

	virtual BOOL OnInitDialog();

	ULONG	PortList[MAX_PARPORTS];
	UCHAR	UnitList[MAX_JOYUNITS];
	USHORT	VendorID;
	USHORT	ProductID;

	int FindAvailablePorts (void);
	UCHAR FindJoytypeOnPort (ULONG Port);
	int FindAvailableUnits (ULONG Port);
	int FindJoytypeUnits (UCHAR JoyType);
	int CheckPortPreReqs(UCHAR JoyType, UCHAR LPTNumber);

// Dialog Data
	//{{AFX_DATA(CAddDialog)
	enum { IDD = IDD_ADDDIALOG };
	CComboBox	m_JoySubTypeList;
	CComboBox	m_DevTypeList;
	CButton	m_AddButton;
	CComboBox	m_JoyUnitList;
	CComboBox	m_JoyTypeList;
	CComboBox	m_PortList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddDialog)
	afx_msg void OnSelchangePortlist();
	afx_msg void OnSelchangeTypelist();
	virtual void OnOK();
	afx_msg void OnSelchangeDevlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDIALOG_H__7BFF1330_606B_4C7E_B782_C558578BAE44__INCLUDED_)
