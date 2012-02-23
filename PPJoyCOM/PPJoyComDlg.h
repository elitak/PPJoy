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


// PPJoyComDlg.h : header file
//

#if !defined(AFX_PPJOYCOMDLG_H__CBB40FF6_F6B1_48FD_B551_5B514983E8E4__INCLUDED_)
#define AFX_PPJOYCOMDLG_H__CBB40FF6_F6B1_48FD_B551_5B514983E8E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPPJoyComDlg dialog

class CPPJoyComDlg : public CDialog
{
// Construction
public:
	int PopulateCOMCombo (void);
	int PopulateJoyCombo (void);
	int StartScanLoop (void);
	void StopScanLoop (void);
	void LoadComboValues (void);
	void LoadINI (char *Filename);
	void SaveINI (char *Filename);
	CPPJoyComDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPPJoyComDlg)
	enum { IDD = IDD_PPJOYCOM_DIALOG };
	CButton	m_LogButton;
	CComboBox	m_RTSCombo;
	CComboBox	m_DTRCombo;
	CComboBox	m_DSRCombo;
	CComboBox	m_CTSCombo;
	CComboBox	m_BitsCombo;
	CComboBox	m_StopCombo;
	CComboBox	m_BaudCombo;
	CComboBox	m_SerialCombo;
	CComboBox	m_ProtocolCombo;
	CComboBox	m_ParityCombo;
	CComboBox	m_JoyCombo;
	CButton	m_DebugBeep;
	CStatic	m_SerialStatus;
	CStatic	m_PPJoyStatus;
	CStatic	m_StreamStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPPJoyComDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPPJoyComDlg)
    afx_msg LRESULT OnPPJoyStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSerialStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnStreamStatus(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDebugbeep();
	afx_msg void OnSelchangeParitycombo();
	afx_msg void OnSelchangeJoycombo();
	afx_msg void OnLoadini();
	afx_msg void OnSaveini();
	afx_msg void OnSelchangeBitscombo();
	afx_msg void OnSelchangeBaudcombo();
	afx_msg void OnSelchangeProtocolcombo();
	afx_msg void OnSelchangeSerialcombo();
	afx_msg void OnSelchangeStopcombo();
	afx_msg void OnSelchangeRtscombo();
	afx_msg void OnSelchangeDtrcombo();
	afx_msg void OnSelchangeDsrcombo();
	afx_msg void OnSelchangeCtscombo();
	afx_msg void OnLogfile();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PPJOYCOMDLG_H__CBB40FF6_F6B1_48FD_B551_5B514983E8E4__INCLUDED_)
