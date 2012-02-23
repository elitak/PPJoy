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


#if !defined(AFX_TIMINGWIZARD_H__58433E88_3DCF_4D5E_9F34_2042E64549AA__INCLUDED_)
#define AFX_TIMINGWIZARD_H__58433E88_3DCF_4D5E_9F34_2042E64549AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimingWizard.h : header file
//

#include <TCHAR.h>

#include "JoyDefs.h"
#include "PPJIOCTL.h"

class CTimingWizard;

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_FMSPage dialog

class CTimingWizard_FMSPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CTimingWizard_FMSPage)

// Construction
public:
	JOYTIMING NewTiming;
	void RestoreValues (void);
	BOOL OnInitDialog();
	CTimingWizard *TimingWizard;
	void AddToWizard(CTimingWizard *Wizard);
	CTimingWizard_FMSPage();
	~CTimingWizard_FMSPage();

// Dialog Data
	//{{AFX_DATA(CTimingWizard_FMSPage)
	enum { IDD = IDD_TIMING_FMS };
	CButton	m_ResetButton;
	CButton	m_ApplyButton;
	CEdit	m_SyncPulseEdit;
	CEdit	m_MinPulseEdit;
	CEdit	m_MaxPulseEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTimingWizard_FMSPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTimingWizard_FMSPage)
	afx_msg void OnApplySettings();
	afx_msg void OnResetdefault();
	afx_msg void OnEditChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_GenesisPage dialog

class CTimingWizard_GenesisPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CTimingWizard_GenesisPage)

// Construction
public:
	JOYTIMING NewTiming;
	void RestoreValues (void);
	BOOL OnInitDialog();
	CTimingWizard *TimingWizard;
	void AddToWizard(CTimingWizard *Wizard);
	CTimingWizard_GenesisPage();
	~CTimingWizard_GenesisPage();

// Dialog Data
	//{{AFX_DATA(CTimingWizard_GenesisPage)
	enum { IDD = IDD_TIMING_GENESIS };
	CButton	m_ResetButton;
	CButton	m_ApplyButton;
	CEdit	m_Bit6DelayEdit;
	CEdit	m_BitDelayEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTimingWizard_GenesisPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTimingWizard_GenesisPage)
	afx_msg void OnApplySettings();
	afx_msg void OnResetdefault();
	afx_msg void OnEditChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_LPTswitchPage dialog

class CTimingWizard_LPTswitchPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CTimingWizard_LPTswitchPage)

// Construction
public:
	JOYTIMING NewTiming;
	void RestoreValues (void);
	BOOL OnInitDialog();
	CTimingWizard *TimingWizard;
	void AddToWizard(CTimingWizard *Wizard);
	CTimingWizard_LPTswitchPage();
	~CTimingWizard_LPTswitchPage();

// Dialog Data
	//{{AFX_DATA(CTimingWizard_LPTswitchPage)
	enum { IDD = IDD_TIMING_LPTSWITCH };
	CButton	m_ResetButton;
	CButton	m_ApplyButton;
	CEdit	m_RowDelayEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTimingWizard_LPTswitchPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTimingWizard_LPTswitchPage)
	afx_msg void OnApplySettings();
	afx_msg void OnResetdefault();
	afx_msg void OnEditChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_PSXPage dialog

class CTimingWizard_PSXPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CTimingWizard_PSXPage)

// Construction
public:
	JOYTIMING NewTiming;
	void RestoreValues (void);
	BOOL OnInitDialog();
	CTimingWizard *TimingWizard;
	void AddToWizard(CTimingWizard *Wizard);
	CTimingWizard_PSXPage();
	~CTimingWizard_PSXPage();

// Dialog Data
	//{{AFX_DATA(CTimingWizard_PSXPage)
	enum { IDD = IDD_TIMING_PSX };
	CButton	m_ResetButton;
	CButton	m_ApplyButton;
	CEdit	m_TailDelayEdit;
	CEdit	m_SelDelayEdit;
	CEdit	m_BitDelayEdit;
	CEdit	m_AckXDelayEdit;
	CEdit	m_Ack1DelayEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTimingWizard_PSXPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTimingWizard_PSXPage)
	afx_msg void OnApplySettings();
	afx_msg void OnResetdefault();
	afx_msg void OnEditChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_SNESPage dialog

class CTimingWizard_SNESPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CTimingWizard_SNESPage)

// Construction
public:
	JOYTIMING NewTiming;
	void RestoreValues (void);
	BOOL OnInitDialog();
	CTimingWizard *TimingWizard;
	void AddToWizard(CTimingWizard *Wizard);
	CTimingWizard_SNESPage();
	~CTimingWizard_SNESPage();

// Dialog Data
	//{{AFX_DATA(CTimingWizard_SNESPage)
	enum { IDD = IDD_TIMING_SNES };
	CButton	m_ResetButton;
	CButton	m_ApplyButton;
	CEdit	m_SetupDelayEdit;
	CEdit	m_BitDelayEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTimingWizard_SNESPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTimingWizard_SNESPage)
	afx_msg void OnApplySettings();
	afx_msg void OnResetdefault();
	afx_msg void OnEditChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_StartPage dialog

class CTimingWizard_StartPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CTimingWizard_StartPage)

// Construction
public:
	BOOL OnInitDialog();
	CTimingWizard *TimingWizard;
	void AddToWizard(CTimingWizard *Wizard);
	CTimingWizard_StartPage();
	~CTimingWizard_StartPage();

// Dialog Data
	//{{AFX_DATA(CTimingWizard_StartPage)
	enum { IDD = IDD_TIMING_START };
	CStatic	m_HelpText;
	CStatic	m_TypeCaption;
	CStatic	m_JoystickCaption;
	CButton	m_ModTypeTimingRadio;
	CButton	m_DelTypeTimingRadio;
	CButton	m_ModJoyTimingRadio;
	CButton	m_DelJoyTimingRadio;
	CButton	m_AddTypeTimingRadio;
	CButton	m_AddJoyTimingRadio;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTimingWizard_StartPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTimingWizard_StartPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_DelTimingPage dialog

class CTimingWizard_DelTimingPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CTimingWizard_DelTimingPage)

// Construction
public:
	BOOL OnInitDialog();
	CTimingWizard *TimingWizard;
	void AddToWizard(CTimingWizard *Wizard);
	CTimingWizard_DelTimingPage();
	~CTimingWizard_DelTimingPage();

// Dialog Data
	//{{AFX_DATA(CTimingWizard_DelTimingPage)
	enum { IDD = IDD_TIMING_DEL };
	CStatic	m_HelpText;
	CStatic	m_DelCaption;
	CButton	m_YesRadio;
	CButton	m_NoRadio;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTimingWizard_DelTimingPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTimingWizard_DelTimingPage)
	afx_msg void OnDeleteYes();
	afx_msg void OnDeleteNo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard

class CTimingWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CTimingWizard)

// Construction
public:
	CTimingWizard();
//	CTimingWizard(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
//	CTimingWizard(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	CTimingWizard_DelTimingPage	DelTimingPage;
	CTimingWizard_FMSPage		FMSPage;
	CTimingWizard_GenesisPage	GenesisPage;
	CTimingWizard_LPTswitchPage	LPTswitchPage;
	CTimingWizard_PSXPage		PSXPage;
	CTimingWizard_SNESPage		SNESPage;
	CTimingWizard_StartPage		StartPage;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimingWizard)
	public:
	virtual int DoModal();
	//}}AFX_VIRTUAL

// Implementation
public:
	int DeleteTiming (void);
	BOOL WriteNewTiming (PJOYTIMING Timing);
	PJOYTIMING GetTiming (void);
	virtual ~CTimingWizard();
	int				JoyIndex;
	BOOL ChangeTypeTiming;
	TCHAR			*IFTypeName;
	TCHAR			*DevTypeName;
	int				LPTNumber;
	int				JoyType;
	int				UnitNumber;
	PJOYTIMING		DefaultTiming;
	PJOYTIMING		DriverTiming;
	PJOYTIMING		DeviceTiming;

	// Generated message map functions
protected:
	//{{AFX_MSG(CTimingWizard)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMINGWIZARD_H__58433E88_3DCF_4D5E_9F34_2042E64549AA__INCLUDED_)
