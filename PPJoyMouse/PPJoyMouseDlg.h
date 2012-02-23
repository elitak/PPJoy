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


// PPJoyMouseDlg.h : header file
//

#if !defined(AFX_PPJOYMOUSEDLG_H__98411D01_EECB_463E_BFF9_F7EB4FFA561C__INCLUDED_)
#define AFX_PPJOYMOUSEDLG_H__98411D01_EECB_463E_BFF9_F7EB4FFA561C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPPJoyMouseDlg dialog

class CPPJoyMouseDlg : public CDialog
{
// Construction
public:
	int PopulateJoyCombo (void);
	CPPJoyMouseDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPPJoyMouseDlg)
	enum { IDD = IDD_PPJOYMOUSE_DIALOG };
	CEdit	m_ZGainEdit;
	CEdit	m_YGainEdit;
	CEdit	m_XGainEdit;
	CStatic	m_PPJoyStatus;
	CComboBox	m_JoyCombo;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPPJoyMouseDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	void LoadINI (char *Filename);
	void SaveINI (char *Filename);

	// Generated message map functions
	//{{AFX_MSG(CPPJoyMouseDlg)
    afx_msg LRESULT OnPPJoyStatus(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeJoycombo();
	virtual void OnCancel();
	afx_msg void OnChangeXgainedit();
	afx_msg void OnChangeYgainedit();
	afx_msg void OnChangeZgainedit();
	afx_msg void OnLoadini();
	afx_msg void OnSaveini();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PPJOYMOUSEDLG_H__98411D01_EECB_463E_BFF9_F7EB4FFA561C__INCLUDED_)
