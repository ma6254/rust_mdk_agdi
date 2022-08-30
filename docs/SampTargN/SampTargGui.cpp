#include "stdafx.h"
#include "agdi.h"
#include "SampTarg.h"
#include "resource.h"


extern "C"  {


class CMyDialog : public CDialog  {
	//{{AFX_MSG(CMyDialog)
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

static CMyDialog oDlg;




class CMyFormView : public CFormView  {
// this class is just here to provide access to the
// otherwise protected constructor of CFormView

  static BOOL  m_bOpen;

public:
	CMyFormView(UINT nIDTemplate) : CFormView(nIDTemplate) {}

	inline static IsOpen() { return m_bOpen; }

	//{{AFX_MSG(CMyFormView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	afx_msg	int  OnCreate(LPCREATESTRUCT lpCS);
	afx_msg	void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};





void _EXPO_ AG_UvInit()
{
	// do optional initialization work
	// this function is called before all other AG_Uv... functions
	// just after start of the session

	// function may be missing
}


void _EXPO_ AG_UvExit()
{
	// do optional termination work
	// this function is called after all other AG_Uv... functions
	// just before the end of the session

	// function may be missing

	if (oDlg.GetSafeHwnd() != NULL)
	{
		oDlg.DestroyWindow();
	}
}


HMENU _EXPO_ AG_UvGetMenu()
{
	// return the handle of the menu,
	// that contains the popup menus 
	// to be inserted into the µVision main menu bar

	// important note: if this function is missing or returns NULL, 
	// then no GUI extension will be added to µVision 

	CMenu oMenu;
	oMenu.LoadMenu(IDR_TEST);
	HMENU hMenu = oMenu.Detach();
	return hMenu;
}




HACCEL _EXPO_ AG_UvGetAccelerators()
{
	// return the handle of the accelerator tables,
	// that contains the accelerators 
	// to be added to the µVision accelerator table

	// function may be missing or return NULL

	return LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_TEST));
}




HICON _EXPO_ AG_UvGetIcon()
{
	// return the handle of the icon,
	// that should be displayed in the title bar of all MDI childs 
	// containing a view owned by this DLL 
	// (see AG_UvCreateView() below)

	// function may be missing or return NULL

	return LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_TEST));
}



static BOOL bCheck;



U32 _EXPO_ AG_UvOnUpdateMenuItem(MENUITEMINFO* psInfo)
{
	// update the status of a menu item
	// according to the WIN32 API SetMenuItemInfo()  

	// return TRUE, if the menu item is updated

	// function may be missing

	switch(psInfo->wID)
	{
		default:
			psInfo->fState = MFS_ENABLED;
			break;
		case ID_TEST_CHECK:
			psInfo->fState = (bCheck ? MFS_CHECKED : MFS_UNCHECKED);
			psInfo->fState |= (bCheck ? MFS_DISABLED : MFS_ENABLED);
			break;
		case ID_TEST_UNCHECK:
			psInfo->fState = (bCheck ? MFS_UNCHECKED : MFS_CHECKED);
			psInfo->fState |= (bCheck ? MFS_ENABLED : MFS_DISABLED);
			break;
		case ID_TEST_FORM_1:
			psInfo->fState = (CMyFormView::IsOpen() ? MFS_CHECKED : MFS_UNCHECKED);
			psInfo->fState |= (CMyFormView::IsOpen() ? MFS_DISABLED : MFS_ENABLED);
			break;
	}

	return TRUE;
}



BEGIN_MESSAGE_MAP(CMyDialog, CDialog)
	//{{AFX_MSG_MAP(CMyDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CMyDialog::OnCancel()
{
	DestroyWindow();
}



U32 _EXPO_ AG_UvOnCallMenuItem(UINT nID, LPARAM* pViewInfo)
{
	// execute a menu or accelerator command

	// Special case:
	// Setting *pViewInfo to a non-zero value causes 
	// µVision to create a new MDI child window.
	// After creation of this new MDI child µVision calls
	// AG_UvCreateView() (see below), to give this DLL the
	// opportunity to create a child window, that fills the client area
	// of the new MDI child.
	// The info stored into *pViewInfo is passed by µVision without any changes 
	// to AG_UvCreateView() and can be used to transfer custom data, 
	// that controls creation of the child window.
	// In this example *ViewInfo is filled with a number, 
	// that identifies the type of child window to be created.

	// return TRUE, if the command has been processed

	*pViewInfo = 0;

	switch(nID)
	{
		case ID_TEST_EDIT:
			*pViewInfo = 1;				// let us create an edit view
			break;
		case ID_TEST_FORM_1:
			*pViewInfo = 2;				// let us create a form view
			break;
		case ID_TEST_MESSAGEBOX:
			AfxMessageBox("Here is the Extension DLL !!");
			break;
		case ID_TEST_MODAL:
		{
			CDialog oTmpDlg(IDD_MODAL);
			oTmpDlg.DoModal();
			break;
		}
		case ID_TEST_MODELESS:
		{
			if (oDlg.GetSafeHwnd() == NULL)
			{
				oDlg.Create(IDD_MODELESS);
			}

			oDlg.ShowWindow(SW_SHOW);
			break;
		}
		case ID_TEST_CHECK:
			bCheck = TRUE;
			break;
		case ID_TEST_UNCHECK:
			bCheck = FALSE;
			break;
		case AG_EXT_CMD_DEBUG_BREAKPOINTS:
			AfxMessageBox("This menu item was hijacked by an external DLL !!");
			break;
	}
	return TRUE;
}

BOOL CMyFormView::m_bOpen = FALSE;



BEGIN_MESSAGE_MAP(CMyFormView, CFormView)
	//{{AFX_MSG_MAP(CSampTargApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



afx_msg	int CMyFormView::OnCreate(LPCREATESTRUCT lpCS)
{
	int nRet = CFormView::OnCreate(lpCS);
	m_bOpen = TRUE;
	return nRet;
}


afx_msg	void CMyFormView::OnDestroy()
{
	m_bOpen = FALSE;
	CFormView::OnDestroy();
}



HWND _EXPO_ AG_UvCreateView(HWND hwndParent, LPARAM nViewInfo, UINT* pnSizeInfo)
{
	// returns a handle to the window, 
	// that fills the client area of an MDI child

	// nViewInfo contains the LPARAM value filled in by the previous call of AG_UvOnCallMenuItem()

	// *pnSizeInfo is to be filled with information,
	// about the initial size of the parent MDI child.

	// *pnSizeInfo = 1 should be used by all windows of variable size (typical example: edit view).
	// the size of the window created here does not affect the initial size of the parent MDI child

	// *pnSizeInfo = 2 should be used by all windows of fix size (typical example: form view).
	// the initial size of the parent MDI child is adjusted to fit to the size of the window created here

	// Remark:
	// when the parent MDI child is resized, then it enforces the size of the window created here to fit
	// into its client area. This means: the size of the window we are about to create here follows the size 
	// of the client are of the parent MDI child. As a consequence our window here is completely responsible
	// for scrolling.
	
	// return the HWND of the new window

	CWnd* poParent = CWnd::FromHandle(hwndParent);

	CView* poView = NULL;

	CCreateContext oContext;
	oContext.m_pCurrentDoc = NULL;
	oContext.m_pCurrentFrame = NULL;
	oContext.m_pLastView = NULL;
	oContext.m_pNewDocTemplate = NULL;
	oContext.m_pNewViewClass = RUNTIME_CLASS(CEditView);

	switch (nViewInfo)
	{
		default:
		case 1:
		{	
			poView = new CEditView;
			poView->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0,0,200,200), poParent, nViewInfo, &oContext);

			poView->ModifyStyle(WS_BORDER, 0, 0);
			poView->ModifyStyle(WS_EX_CLIENTEDGE, 0, 0);

			*pnSizeInfo = 1;

			poParent->SetWindowText("Resizing View");
			break;
		}
		case 2:
		{
			poView = new CMyFormView(IDD_FORM);
			poView->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0,0,200,200), poParent, nViewInfo, &oContext);

			poView->ModifyStyle(WS_BORDER, 0, 0);
			poView->ModifyStyle(WS_EX_CLIENTEDGE, 0, 0);

			CSize sz = ((CFormView*)poView)->GetTotalSize();

			CRect rc(CPoint(0, 0), sz);
			poView->CalcWindowRect(&rc);
			poView->SetWindowPos(NULL, 0, 0, rc.Width() + 3, rc.Height() + 3, SWP_NOZORDER | SWP_NOMOVE);

			*pnSizeInfo = 2;

			poParent->SetWindowText("Static View - Propagating Initial Size to Parent");
			break;
		}
	}

	return poView->GetSafeHwnd();
}


void _EXPO_ AG_UvGetMessageString(UINT nID, char* pszString, UINT nLen)
{
	// fill a buffer with the message string that corresponds
	// to a certain command ID.
	// this function does something very similar to MFCs CFrameWnd::GetMessageString()
	// It is called among others when the user hovers over a menu item, that has been
	// added by this DLL, in order to obtain the string to be displayed in the main 
	// status bar of µVision.

	// load appropriate string:

	CString oString;
	oString.LoadString(nID);

	strncpy(pszString, oString, nLen);
	pszString[nLen] = 0;
}


};
