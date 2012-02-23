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


// PPJoyJoyDlg.h : header file
//

#if !defined(AFX_PPJOYJOYDLG_H__E756E15E_1B16_4983_9E33_7EFC2A64A198__INCLUDED_)
#define AFX_PPJOYJOYDLG_H__E756E15E_1B16_4983_9E33_7EFC2A64A198__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPPJoyJoyDlg dialog

class CPPJoyJoyDlg : public CDialog
{
// Construction
public:
	int PopulateJoyCombo (void);
	void StopScanLoop (void);
	int StartScanLoop (void);
	void LoadINI (char *Filename);
	void SaveINI (char *Filename);
	void UpdateButtonJoy (int Button);
	void UpdateAxisJoy (int Axis);
	void UpdateControls (void);
	CPPJoyJoyDlg(CWnd* pParent = NULL);	// standard constructor
    int ScanJoystickDevices (void);

// Dialog Data
	CComboBox	*(m_AxisJoy[8]);
	CComboBox	*(m_AxisAxis[8]);
	CComboBox	*(m_ButJoy[16]);
	CComboBox	*(m_ButBut[16]);

	//{{AFX_DATA(CPPJoyJoyDlg)
	enum { IDD = IDD_PPJOYJOY_DIALOG };
	CStatic	m_PPJoyStatus;
	CComboBox	m_JoyCombo;
	CEdit	m_IntervalEdit;
	CComboBox	m_But9Joy;
	CComboBox	m_But8Joy;
	CComboBox	m_But7Joy;
	CComboBox	m_But6Joy;
	CComboBox	m_But5Joy;
	CComboBox	m_But4Joy;
	CComboBox	m_But3Joy;
	CComboBox	m_But2Joy;
	CComboBox	m_But1Joy;
	CComboBox	m_But16Joy;
	CComboBox	m_But15Joy;
	CComboBox	m_But14Joy;
	CComboBox	m_But13Joy;
	CComboBox	m_But12Joy;
	CComboBox	m_But11Joy;
	CComboBox	m_But10Joy;
	CComboBox	m_But9But;
	CComboBox	m_But8But;
	CComboBox	m_But7But;
	CComboBox	m_But6But;
	CComboBox	m_But5But;
	CComboBox	m_But4But;
	CComboBox	m_But3But;
	CComboBox	m_But2But;
	CComboBox	m_But1But;
	CComboBox	m_But10But;
	CComboBox	m_But16But;
	CComboBox	m_But15But;
	CComboBox	m_But14But;
	CComboBox	m_But13But;
	CComboBox	m_But12But;
	CComboBox	m_But11But;
	CComboBox	m_Axis8Joy;
	CComboBox	m_Axis7Joy;
	CComboBox	m_Axis6Joy;
	CComboBox	m_Axis5Joy;
	CComboBox	m_Axis4Joy;
	CComboBox	m_Axis3Joy;
	CComboBox	m_Axis2Joy;
	CComboBox	m_Axis1Joy;
	CComboBox	m_Axis8Axis;
	CComboBox	m_Axis7Axis;
	CComboBox	m_Axis6Axis;
	CComboBox	m_Axis5Axis;
	CComboBox	m_Axis4Axis;
	CComboBox	m_Axis3Axis;
	CComboBox	m_Axis2Axis;
	CComboBox	m_Axis1Axis;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPPJoyJoyDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPPJoyJoyDlg)
    afx_msg LRESULT OnPPJoyStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRescanSticks(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeAxis1joy();
	afx_msg void OnSelchangeAxis2joy();
	afx_msg void OnSelchangeAxis3joy();
	afx_msg void OnSelchangeAxis4joy();
	afx_msg void OnSelchangeAxis5joy();
	afx_msg void OnSelchangeAxis6joy();
	afx_msg void OnSelchangeAxis7joy();
	afx_msg void OnSelchangeAxis8joy();
	afx_msg void OnSelchangeAxis1axis();
	afx_msg void OnSelchangeAxis2axis();
	afx_msg void OnSelchangeAxis3axis();
	afx_msg void OnSelchangeAxis4axis();
	afx_msg void OnSelchangeAxis5axis();
	afx_msg void OnSelchangeAxis6axis();
	afx_msg void OnSelchangeAxis7axis();
	afx_msg void OnSelchangeAxis8axis();
	afx_msg void OnSelchangeBut1joy();
	afx_msg void OnSelchangeBut2joy();
	afx_msg void OnSelchangeBut3joy();
	afx_msg void OnSelchangeBut4joy();
	afx_msg void OnSelchangeBut5joy();
	afx_msg void OnSelchangeBut6joy();
	afx_msg void OnSelchangeBut7joy();
	afx_msg void OnSelchangeBut8joy();
	afx_msg void OnSelchangeBut9joy();
	afx_msg void OnSelchangeBut10joy();
	afx_msg void OnSelchangeBut11joy();
	afx_msg void OnSelchangeBut12joy();
	afx_msg void OnSelchangeBut13joy();
	afx_msg void OnSelchangeBut14joy();
	afx_msg void OnSelchangeBut15joy();
	afx_msg void OnSelchangeBut16joy();
	afx_msg void OnSelchangeBut1but();
	afx_msg void OnSelchangeBut2but();
	afx_msg void OnSelchangeBut3but();
	afx_msg void OnSelchangeBut4but();
	afx_msg void OnSelchangeBut5but();
	afx_msg void OnSelchangeBut6but();
	afx_msg void OnSelchangeBut7but();
	afx_msg void OnSelchangeBut8but();
	afx_msg void OnSelchangeBut9but();
	afx_msg void OnSelchangeBut10but();
	afx_msg void OnSelchangeBut11but();
	afx_msg void OnSelchangeBut12but();
	afx_msg void OnSelchangeBut13but();
	afx_msg void OnSelchangeBut14but();
	afx_msg void OnSelchangeBut15but();
	afx_msg void OnSelchangeBut16but();
	afx_msg void OnLoadini();
	afx_msg void OnSaveini();
	afx_msg void OnSetinterval();
	afx_msg void OnRescan();
	afx_msg void OnSelchangeJoycombo();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PPJOYJOYDLG_H__E756E15E_1B16_4983_9E33_7EFC2A64A198__INCLUDED_)
