// SetVirtualJoyStateDlg.h : header file
//

#if !defined(AFX_SETVIRTUALJOYSTATEDLG_H__436E61FF_AB13_4C97_9E35_3662402BDE67__INCLUDED_)
#define AFX_SETVIRTUALJOYSTATEDLG_H__436E61FF_AB13_4C97_9E35_3662402BDE67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSetVirtualJoyStateDlg dialog

class CSetVirtualJoyStateDlg : public CDialog
{
// Construction
public:
	int PopulateJoyCombo (void);
	CSetVirtualJoyStateDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSetVirtualJoyStateDlg)
	enum { IDD = IDD_SETVIRTUALJOYSTATE_DIALOG };
	CEdit	m_DigitalEdit8;
	CEdit	m_DigitalEdit7;
	CEdit	m_DigitalEdit6;
	CEdit	m_DigitalEdit5;
	CEdit	m_DigitalEdit4;
	CEdit	m_DigitalEdit2;
	CEdit	m_DigitalEdit3;
	CEdit	m_DigitalEdit1;
	CEdit	m_AnalogEdit8;
	CEdit	m_AnalogEdit7;
	CEdit	m_AnalogEdit6;
	CEdit	m_AnalogEdit5;
	CEdit	m_AnalogEdit4;
	CEdit	m_AnalogEdit3;
	CEdit	m_AnalogEdit2;
	CEdit	m_AnalogEdit1;
	CComboBox	m_JoyCombo;
	CStatic	m_PPJoyStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetVirtualJoyStateDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSetVirtualJoyStateDlg)
    afx_msg LRESULT OnPPJoyStatus(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnSelchangeJoycombo();
	afx_msg void OnUpdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETVIRTUALJOYSTATEDLG_H__436E61FF_AB13_4C97_9E35_3662402BDE67__INCLUDED_)
