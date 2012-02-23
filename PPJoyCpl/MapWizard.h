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


#if !defined(AFX_MAPWIZARD_H__FB6FF8FB_98DB_4536_AAE0_97C9B95825B6__INCLUDED_)
#define AFX_MAPWIZARD_H__FB6FF8FB_98DB_4536_AAE0_97C9B95825B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapWizard.h : header file
//

#include "Mapping.h"
#include "JoyDefs.h"

#include <TCHAR.h>

#define	DINPUT_MAXBUTTONS	32
#define	MAPWIZ_MAXAXIS		8
#define	MAPWIZ_MAXHATS		2
#define	MAPWIZ_MAXBUTTONS	16
#define	MAPWIZ_MAXCONFIGS	4

class CMapWizard;

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_StartPage dialog

class CMapWizard_StartPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMapWizard_StartPage)

// Construction
public:
	CMapWizard_StartPage();
	~CMapWizard_StartPage();

// Dialog Data
	//{{AFX_DATA(CMapWizard_StartPage)
	enum { IDD = IDD_MAPWIZ_START };
	CStatic	m_TypeCaption;
	CStatic	m_JoystickCaption;
	CStatic	m_HelpText;
	CButton	m_ModTypeMapRadio;
	CButton	m_ModJoyMapRadio;
	CButton	m_DelTypeMapRadio;
	CButton	m_DelJoyMapRadio;
	CButton	m_AddTypeMapRadio;
	CButton	m_AddJoyMapRadio;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMapWizard_StartPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	public:
		CMapWizard		*MapWizard;
		void AddToWizard (CMapWizard *Wizard);
		virtual BOOL OnInitDialog();

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMapWizard_StartPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_AxisMapPage dialog

class CMapWizard_AxisMapPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMapWizard_AxisMapPage)

// Construction
public:
	CMapWizard_AxisMapPage();
	~CMapWizard_AxisMapPage();

// Dialog Data
	//{{AFX_DATA(CMapWizard_AxisMapPage)
	enum { IDD = IDD_MAPWIZ_AXISMAP };
	CStatic	m_ConfigName;
	//}}AFX_DATA
	CComboBox	m_AxisMinMap[MAPWIZ_MAXAXIS];
	CComboBox	m_AxisMaxMap[MAPWIZ_MAXAXIS];
	CButton		m_AxisMinScan[MAPWIZ_MAXAXIS];
	CButton		m_AxisMaxScan[MAPWIZ_MAXAXIS];
	CStatic		m_AxisName[MAPWIZ_MAXAXIS];

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMapWizard_AxisMapPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	public:
		void ScanAxis (int Axis, int ScanFlag);
		void UpdateControls (void);
		void UpdateAxisMap (int Axis);
		void EnableControls (int NumAxes);
		CMapWizard		*MapWizard;
		void AddToWizard (CMapWizard *Wizard);
		virtual BOOL OnInitDialog();

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMapWizard_AxisMapPage)
	afx_msg void OnA1minscan();
	afx_msg void OnA2minscan();
	afx_msg void OnA3minscan();
	afx_msg void OnA4minscan();
	afx_msg void OnA5minscan();
	afx_msg void OnA6minscan();
	afx_msg void OnA7minscan();
	afx_msg void OnA8minscan();
	afx_msg void OnA1maxscan();
	afx_msg void OnA2maxscan();
	afx_msg void OnA3maxscan();
	afx_msg void OnA4maxscan();
	afx_msg void OnA5maxscan();
	afx_msg void OnA6maxscan();
	afx_msg void OnA7maxscan();
	afx_msg void OnA8maxscan();
	afx_msg void OnSelchangeA1maxcombo();
	afx_msg void OnSelchangeA1mincombo();
	afx_msg void OnSelchangeA2maxcombo();
	afx_msg void OnSelchangeA2mincombo();
	afx_msg void OnSelchangeA3maxcombo();
	afx_msg void OnSelchangeA3mincombo();
	afx_msg void OnSelchangeA4maxcombo();
	afx_msg void OnSelchangeA4mincombo();
	afx_msg void OnSelchangeA5maxcombo();
	afx_msg void OnSelchangeA5mincombo();
	afx_msg void OnSelchangeA6maxcombo();
	afx_msg void OnSelchangeA6mincombo();
	afx_msg void OnSelchangeA7maxcombo();
	afx_msg void OnSelchangeA7mincombo();
	afx_msg void OnSelchangeA8maxcombo();
	afx_msg void OnSelchangeA8mincombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_ButtonMapPage dialog

class CMapWizard_ButtonMapPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMapWizard_ButtonMapPage)

// Construction
public:
	CMapWizard_ButtonMapPage();
	~CMapWizard_ButtonMapPage();

// Dialog Data
	//{{AFX_DATA(CMapWizard_ButtonMapPage)
	enum { IDD = IDD_MAPWIZ_BUTMAP };
	CStatic	m_ConfigName;
	//}}AFX_DATA
	CComboBox	m_ButtonMap[MAPWIZ_MAXBUTTONS];
	CButton		m_ButtonScan[MAPWIZ_MAXBUTTONS];
	CStatic		m_ButtonName[MAPWIZ_MAXBUTTONS];

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMapWizard_ButtonMapPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	public:
		int StartOffset;
		void ScanButton (int Button);
		void UpdateButtonMap (int Button);
		void EnableControls (int NumButtons);
		CMapWizard		*MapWizard;
		void AddToWizard (CMapWizard *Wizard);
		virtual BOOL OnInitDialog();

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMapWizard_ButtonMapPage)
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
	afx_msg void OnSelchangeBut1combo();
	afx_msg void OnSelchangeBut2combo();
	afx_msg void OnSelchangeBut3combo();
	afx_msg void OnSelchangeBut4combo();
	afx_msg void OnSelchangeBut5combo();
	afx_msg void OnSelchangeBut6combo();
	afx_msg void OnSelchangeBut7combo();
	afx_msg void OnSelchangeBut8combo();
	afx_msg void OnSelchangeBut9combo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_AxisDefPage dialog

class CMapWizard_AxisDefPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMapWizard_AxisDefPage)

// Construction
public:
	CMapWizard_AxisDefPage();
	~CMapWizard_AxisDefPage();

// Dialog Data
	//{{AFX_DATA(CMapWizard_AxisDefPage)
	enum { IDD = IDD_MAPWIZ_AXISDEF };
	CComboBox	m_NumHatsCombo;
	CComboBox	m_NumButtonsCombo;
	CComboBox	m_NumAxesCombo;
	//}}AFX_DATA
	CComboBox	m_AxisDefine[MAPWIZ_MAXAXIS];
	CStatic		m_AxisName[MAPWIZ_MAXAXIS];

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMapWizard_AxisDefPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	public:
		void SetNextState();
		void UpdateAxisMapping (int Axis);
		void EnableControls (int NumAxes);
		CMapWizard		*MapWizard;
		void AddToWizard (CMapWizard *Wizard);
		virtual BOOL OnInitDialog();

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMapWizard_AxisDefPage)
	afx_msg void OnSelchangeAxis8type();
	afx_msg void OnSelchangeAxis7type();
	afx_msg void OnSelchangeAxis6type();
	afx_msg void OnSelchangeAxis5type();
	afx_msg void OnSelchangeAxis4type();
	afx_msg void OnSelchangeAxis3type();
	afx_msg void OnSelchangeAxis2type();
	afx_msg void OnSelchangeAxis1type();
	afx_msg void OnSelchangeNumaxescombo();
	afx_msg void OnSelchangeNumbuttonscombo();
	afx_msg void OnSelchangeNumpovhatscombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_DonePage dialog

class CMapWizard_DonePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMapWizard_DonePage)

// Construction
public:
	CMapWizard_DonePage();
	~CMapWizard_DonePage();

// Dialog Data
	//{{AFX_DATA(CMapWizard_DonePage)
	enum { IDD = IDD_MAPWIZ_DONE };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA
	public:
		CMapWizard		*MapWizard;
		void AddToWizard (CMapWizard *Wizard);


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMapWizard_DonePage)
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
	//{{AFX_MSG(CMapWizard_DonePage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_DelMapPage dialog

class CMapWizard_DelMapPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMapWizard_DelMapPage)

// Construction
public:
	CMapWizard_DelMapPage();
	~CMapWizard_DelMapPage();

// Dialog Data
	//{{AFX_DATA(CMapWizard_DelMapPage)
	enum { IDD = IDD_MAPWIZ_DELMAP };
	CStatic	m_HelpText;
	CStatic	m_DelCaption;
	CButton	m_YesRadio;
	CButton	m_NoRadio;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMapWizard_DelMapPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	public:
		CMapWizard		*MapWizard;
		void AddToWizard (CMapWizard *Wizard);
		virtual BOOL OnInitDialog();

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMapWizard_DelMapPage)
	afx_msg void OnDeleteYes();
	afx_msg void OnDeleteNo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_HatMapPage dialog

class CMapWizard_HatMapPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMapWizard_HatMapPage)

// Construction
public:
	CMapWizard_HatMapPage();
	~CMapWizard_HatMapPage();

// Dialog Data
	//{{AFX_DATA(CMapWizard_HatMapPage)
	enum { IDD = IDD_MAPWIZ_HATMAP };
	CButton	m_DigitalRadio2;
	CButton	m_DigitalRadio1;
	CButton	m_AnalogRadio2;
	CButton	m_AnalogRadio1;
	CButton	m_WestScan2;
	CButton	m_WestScan1;
	CStatic	m_WestName2;
	CStatic	m_WestName1;
	CComboBox	m_WestCombo2;
	CComboBox	m_WestCombo1;
	CButton	m_SouthScan2;
	CButton	m_SouthScan1;
	CStatic	m_SouthName2;
	CStatic	m_SouthName1;
	CComboBox	m_SouthCombo2;
	CComboBox	m_SouthCombo1;
	CButton	m_NorthScan2;
	CButton	m_NorthScan1;
	CStatic	m_NorthName2;
	CStatic	m_NorthName1;
	CComboBox	m_NorthCombo2;
	CComboBox	m_NorthCombo1;
	CButton	m_EastScan2;
	CButton	m_EastScan1;
	CStatic	m_EastName2;
	CStatic	m_EastName1;
	CComboBox	m_EastCombo2;
	CComboBox	m_EastCombo1;
	CButton	m_DialScan2;
	CButton	m_DialScan1;
	CStatic	m_DialName2;
	CStatic	m_DialName1;
	CComboBox	m_DialCombo2;
	CComboBox	m_DialCombo1;
	CStatic	m_ConfigName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMapWizard_HatMapPage)
	public:
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	public:
		void ScanDir (CComboBox *combo);
		void ScanDial (CComboBox *combo);
		BYTE DirSelToMapping (int Sel);
		BYTE DialSelToMapping (int Sel);
		void EnableControls (int NumHats);
		void UpdateControls (void);
		CMapWizard		*MapWizard;
		void AddToWizard (CMapWizard *Wizard);

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMapWizard_HatMapPage)
	afx_msg void OnSelchangeDialcombo1();
	afx_msg void OnSelchangeDialcombo2();
	afx_msg void OnSelchangeEastcombo1();
	afx_msg void OnSelchangeEastcombo2();
	afx_msg void OnSelchangeNorthcombo1();
	afx_msg void OnSelchangeNorthcombo2();
	afx_msg void OnSelchangeSouthcombo1();
	afx_msg void OnSelchangeSouthcombo2();
	afx_msg void OnSelchangeWestcombo1();
	afx_msg void OnSelchangeWestcombo2();
	afx_msg void OnNorthscan1();
	afx_msg void OnNorthscan2();
	afx_msg void OnEastscan1();
	afx_msg void OnEastscan2();
	afx_msg void OnWestscan1();
	afx_msg void OnWestscan2();
	afx_msg void OnSouthscan1();
	afx_msg void OnSouthscan2();
	afx_msg void OnDialscan1();
	afx_msg void OnDialscan2();
	afx_msg void OnDigitalpovradio1();
	afx_msg void OnAnalogpovradio1();
	afx_msg void OnDigitalpovradio2();
	afx_msg void OnAnalogpovradio2();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CMapWizard

class CMapWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CMapWizard)

// Construction
public:
	CMapWizard();
//	CMapWizard(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
//	CMapWizard(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	CMapWizard_StartPage		StartPage;
	CMapWizard_DelMapPage		DelMapPage;
	CMapWizard_AxisMapPage		AxisMapPage;
	CMapWizard_ButtonMapPage	ButtonMapPage;
	CMapWizard_HatMapPage		HatMapPage;
	CMapWizard_AxisDefPage		AxisDefPage;
	CMapWizard_DonePage			DonePage;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapWizard)
	public:
	virtual int DoModal();
	//}}AFX_VIRTUAL

// Implementation
public:
	TCHAR MapModeDescription[160*sizeof(TCHAR)];
	TCHAR* GetMapModeDescription (int MapMode);
	BOOL ChangeTypeMapping;
	int DeleteMapping (int Scope);
	int WriteUnpackedMapping (int Scope);
	void UnpackMapping (int Scope);

	TCHAR			*IFTypeName;
	TCHAR			*DevTypeName;
	int				JoyIndex;
	int				LPTNumber;
	int				JoyType;
	int				UnitNumber;
	int				ConfigCount;
	int				NumAxes;
	int				NumButtons;
	int				NumHats;
	int				NumConfigs;
	JOYSTICK_MAP	*DefaultMapping;
	JOYSTICK_MAP	*DriverMapping;
	JOYSTICK_MAP	*DeviceMapping;
	HIDUSAGE		AxisDefinition[MAPWIZ_MAXAXIS];
	HIDUSAGE		ButtonDefinition[DINPUT_MAXBUTTONS];
	HIDUSAGE		POVHatDefinition[MAPWIZ_MAXHATS];
	SAxisMapRec		AxisMapping[MAPWIZ_MAXCONFIGS][MAPWIZ_MAXAXIS];
	SButtonMapRec	ButtonMapping[MAPWIZ_MAXCONFIGS][DINPUT_MAXBUTTONS];
	SPOVHatMapRec	POVHatMapping[MAPWIZ_MAXCONFIGS][MAPWIZ_MAXHATS];

	virtual ~CMapWizard();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMapWizard)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CWaitDialog dialog

class CWaitDialog : public CDialog
{
// Construction
public:
	CWaitDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWaitDialog)
	enum { IDD = IDD_PLEASEWAIT };
	CStatic	m_ProgressText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaitDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWaitDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPWIZARD_H__FB6FF8FB_98DB_4536_AAE0_97C9B95825B6__INCLUDED_)
