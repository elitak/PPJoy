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


// PPJoyDLLDlg.h : header file
//

#if !defined(AFX_PPJOYDLLDLG_H__697A26A9_4F79_4BB4_AB74_F74E29E549A6__INCLUDED_)
#define AFX_PPJOYDLLDLG_H__697A26A9_4F79_4BB4_AB74_F74E29E549A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPPJoyDLLDlg dialog

class CPPJoyDLLDlg : public CDialog
{
// Construction
public:
	int PopulateJoyCombo (void);
	void EnableControls (void);
	void LoadComboValues (void);
	void LoadINI (char *Filename);
	void SaveINI (char *Filename);
	int StartScanLoop (void);
	void StopScanLoop (void);
	CPPJoyDLLDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPPJoyDLLDlg)
	enum { IDD = IDD_PPJOYDLL_DIALOG };
	CStatic	m_IntervalText2;
	CStatic	m_IntervalText1;
	CButton	m_SetIntervalButton;
	CEdit	m_DLLNameEdit;
	CComboBox	m_JoyCombo;
	CEdit	m_UpdateInterval;
	CStatic	m_DLLStatus;
	CStatic	m_PPJoyStatus;
	CButton	m_DebugBeep;
	CComboBox	m_DLLTypeCombo;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPPJoyDLLDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPPJoyDLLDlg)
    afx_msg LRESULT OnPPJoyStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDLLStatus(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLoaddll();
	afx_msg void OnLoadini();
	afx_msg void OnSaveini();
	virtual void OnCancel();
	afx_msg void OnSelchangeJoycombo();
	afx_msg void OnSelchangeDlltypecombo();
	afx_msg void OnSetinterval();
	afx_msg void OnDebugbeep();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PPJOYDLLDLG_H__697A26A9_4F79_4BB4_AB74_F74E29E549A6__INCLUDED_)
