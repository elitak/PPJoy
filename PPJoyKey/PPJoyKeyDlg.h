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


// PPJoyKeyDlg.h : header file
//

#if !defined(AFX_PPJOYKEYDLG_H__A65A301D_0D7E_446E_B13F_35E18E0ACD93__INCLUDED_)
#define AFX_PPJOYKEYDLG_H__A65A301D_0D7E_446E_B13F_35E18E0ACD93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPPJoyKeyDlg dialog

class CPPJoyKeyDlg : public CDialog
{
// Construction
public:
	int PopulateJoyCombo (void);
	int ScanForKeypress (void);
	CPPJoyKeyDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPPJoyKeyDlg)
	enum { IDD = IDD_PPJOYKEY_DIALOG };
	CComboBox	m_But16Combo;
	CComboBox	m_But15Combo;
	CComboBox	m_But14Combo;
	CComboBox	m_But13Combo;
	CComboBox	m_But12Combo;
	CComboBox	m_But11Combo;
	CComboBox	m_But10Combo;
	CComboBox	m_But9Combo;
	CStatic	m_PPJoyStatus;
	CComboBox	m_But8Combo;
	CComboBox	m_But7Combo;
	CComboBox	m_But6Combo;
	CComboBox	m_But5Combo;
	CComboBox	m_But4Combo;
	CComboBox	m_But3Combo;
	CComboBox	m_But2Combo;
	CComboBox	m_But1Combo;
	CComboBox	m_A4MinCombo;
	CComboBox	m_A4MaxCombo;
	CComboBox	m_A3MinCombo;
	CComboBox	m_A3MaxCombo;
	CComboBox	m_A2MinCombo;
	CComboBox	m_A1MinCombo;
	CComboBox	m_A2MaxCombo;
	CComboBox	m_A1MaxCombo;
	CComboBox	m_JoyCombo;
	CButton	m_DebugBeep;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPPJoyKeyDlg)
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
	virtual void OnCancel();

	// Generated message map functions
	//{{AFX_MSG(CPPJoyKeyDlg)
    afx_msg LRESULT OnPPJoyStatus(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDebugbeep();
	afx_msg void OnSelchangeJoycombo();
	afx_msg void OnLoadini();
	afx_msg void OnSaveini();
	afx_msg void OnSelchangeA1maxcombo();
	afx_msg void OnSelchangeA1mincombo();
	afx_msg void OnSelchangeA2maxcombo();
	afx_msg void OnSelchangeA2mincombo();
	afx_msg void OnSelchangeA3maxcombo();
	afx_msg void OnSelchangeA3mincombo();
	afx_msg void OnSelchangeA4maxcombo();
	afx_msg void OnSelchangeA4mincombo();
	afx_msg void OnSelchangeBut1combo();
	afx_msg void OnSelchangeBut2combo();
	afx_msg void OnSelchangeBut3combo();
	afx_msg void OnSelchangeBut4combo();
	afx_msg void OnSelchangeBut5combo();
	afx_msg void OnSelchangeBut6combo();
	afx_msg void OnSelchangeBut7combo();
	afx_msg void OnSelchangeBut8combo();
	afx_msg void OnA2minscan();
	afx_msg void OnA2maxscan();
	afx_msg void OnA1minscan();
	afx_msg void OnA1maxscan();
	afx_msg void OnA3minscan();
	afx_msg void OnA3maxscan();
	afx_msg void OnA4minscan();
	afx_msg void OnA4maxscan();
	afx_msg void OnBut1scan();
	afx_msg void OnBut2scan();
	afx_msg void OnBut3scan();
	afx_msg void OnBut4scan();
	afx_msg void OnBut5scan();
	afx_msg void OnBut6scan();
	afx_msg void OnBut7scan();
	afx_msg void OnBut8scan();
	afx_msg void OnBut9scan();
	afx_msg void OnBut10scan();
	afx_msg void OnBut11scan();
	afx_msg void OnBut12scan();
	afx_msg void OnBut13scan();
	afx_msg void OnBut14scan();
	afx_msg void OnBut15scan();
	afx_msg void OnBut16scan();
	afx_msg void OnSelchangeBut10combo();
	afx_msg void OnSelchangeBut11combo();
	afx_msg void OnSelchangeBut12combo();
	afx_msg void OnSelchangeBut13combo();
	afx_msg void OnSelchangeBut14combo();
	afx_msg void OnSelchangeBut15combo();
	afx_msg void OnSelchangeBut16combo();
	afx_msg void OnSelchangeBut9combo();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CScanDialog dialog

class CScanDialog : public CDialog
{
// Construction
public:
	int Scancode;
	int ScanForInput (void);
	CScanDialog(CWnd* pParent = NULL);   // standard constructor
	virtual BOOL OnInitDialog();

// Dialog Data
	//{{AFX_DATA(CScanDialog)
	enum { IDD = IDD_SCANDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScanDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScanDialog)
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PPJOYKEYDLG_H__A65A301D_0D7E_446E_B13F_35E18E0ACD93__INCLUDED_)
