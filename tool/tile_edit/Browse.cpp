// Browse.cpp : implementation file
//

#include "stdafx.h"
#include "tile_edit_dll.h"
#include "Browse.h"
#include "custom.h"
#include <nel/3d/tile_bank.h>

using namespace NL3D;

extern CTileBank tileBank2;

/////////////////////////////////////////////////////////////////////////////
// Browse dialog

Browse::Browse(int nland, CWnd* pParent /*=NULL*/)
	: CDialog(Browse::IDD, pParent)
{	
	//{{AFX_DATA_INIT(Browse)
	//}}AFX_DATA_INIT
	land=nland;
	m_128x128=0;
}


void Browse::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Browse)
	DDX_Control(pDX, IDC_VIEW, m_ctrl);
	DDX_Control(pDX, IDC_INFONUM, m_infotexte);
	//DDX_Control(pDX, IDC_ZOOM1, m_rb_zoom1);
	//DDX_Control(pDX, IDC_NUM, m_rb_num);
	DDX_Control(pDX, IDC_JOUR, m_rb_jour);
	DDX_Radio(pDX, IDC_128X128, m_128x128);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Browse, CDialog)
	//{{AFX_MSG_MAP(Browse)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUMP, OnBump)
	ON_BN_CLICKED(IDC_JOUR, OnJour)
	ON_BN_CLICKED(IDC_NUIT, OnNuit)
	ON_BN_CLICKED(IDC_128X128, OnChangeVariety)
	ON_BN_CLICKED(IDC_ZOOM5, OnChangeVariety)
	ON_BN_CLICKED(IDC_ZOOM6, OnChangeVariety)
	//ON_BN_CLICKED(IDC_NUM, OnNum)
	ON_BN_CLICKED(IDC_OK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_OK2, OnUpdateTiles)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Browse message handlers

BOOL Browse::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	//this->DragAcceptFiles();

	return CDialog::PreCreateWindow(cs);
}

DWORD thread_id;
int thread_actif = 0;
Browse *pDialog;
int ccount=0;

LRESULT Browse::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{

	// TODO: Add your specialized code here and/or call the base class
	if (ccount==0 && message==WM_PAINT)
	{	
		Init();
	}
	if (message==WM_KEYUP || message==WM_KEYDOWN)
	{
		RECT parent,client; 
		GetWindowRect(&parent); m_ctrl.GetWindowRect(&client);
		if (m_ctrl.MousePos.x<(client.right - parent.left) && 
			m_ctrl.MousePos.x>0 &&
			m_ctrl.MousePos.y<(client.bottom - client.top) &&
			m_ctrl.MousePos.y>0)
		{
			m_ctrl.SendMessage(message,wParam,lParam);
		}
	}

	if (message==WM_MOUSEMOVE)
	{
		m_ctrl.MousePos.x = LOWORD(lParam);
		m_ctrl.MousePos.y = HIWORD(lParam);

		RECT client, parent; 
		
		ClientToScreen (&m_ctrl.MousePos);
		m_ctrl.ScreenToClient (&m_ctrl.MousePos);

		m_ctrl.GetWindowRect(&client);
		GetWindowRect(&parent);
		if (m_ctrl.MousePos.x<0)
			m_ctrl.MousePos.x=0;
		if (m_ctrl.MousePos.x>client.right-client.left)
			m_ctrl.MousePos.x=client.right-client.left;
		if (m_ctrl.MousePos.y<0)
			m_ctrl.MousePos.y=0;
		if (m_ctrl.MousePos.y>client.bottom-client.top)
			m_ctrl.MousePos.y=client.bottom-client.top;

		m_ctrl.ClientToScreen (&m_ctrl.MousePos);
		ScreenToClient (&m_ctrl.MousePos);
		
		if (lbutton) //on dessine le carre de selection
		{ 
			selection = 1;
			RECT current;
			SIZE size; size.cx = size.cy = 1;
			current.left = OriginalPos.x;
			current.top = OriginalPos.y;
			current.right = m_ctrl.MousePos.x;
			current.bottom = m_ctrl.MousePos.y;
			if (current.left>current.right) {int temp = current.left; current.left = current.right; current.right = temp;}
			if (current.top>current.bottom) {int temp = current.bottom; current.bottom = current.top; current.top = temp;}
			//m_ctrl.UpdateSelection(&current);
			
			CDC *pDC = GetDC();
			m_ctrl.DrawDragRect(pDC,NULL,size,&last_sel,size);			//on efface l'ancien carre
//			::ReleaseDC(*this,*pDC);			
			
			m_ctrl.UpdateSelection(&current, wParam, m_128x128);						//on affiche les modifes
			
//			pDC = GetDC();
			m_ctrl.DrawDragRect(pDC,&current,size,NULL,size);			//on affiche le nouveau carre
			::ReleaseDC(*this,*pDC);			
			
			last_sel = current;
		}
	}
	if (message==WM_DROPFILES)
	{
		m_ctrl.PostMessage(WM_DROPFILES,wParam,lParam);
	}
	if (message==WM_COMMAND && !thread_actif)
	{
		int button = LOWORD(wParam);
		if (button==IDC_ZOOM1 || button==IDC_ZOOM2 || button==IDC_ZOOM3)
		{
			m_ctrl.Zoom = button - IDC_ZOOM1 +1;
			m_ctrl.UpdateSize(m_128x128);
			m_ctrl.scrollpos = 0;
			SetScrollPos(SB_VERT,0,true);
			m_ctrl.RedrawWindow();
		}
		else if (button==IDC_INFONUM)
		{
			m_ctrl.InfoTexte = 1;
			m_ctrl.RedrawWindow();
		}			
		else if (button==IDC_FILENAME)
		{
			m_ctrl.InfoTexte =2;
			m_ctrl.RedrawWindow();
		}
		else if (button==IDC_GROUP)
		{
			m_ctrl.InfoTexte = 3;
			m_ctrl.RedrawWindow();
		}
		/*else if (button==IDC_NUM)
		{
			//SortTile = 0;
			//m_ctrl.InfoList.Sort();
			//m_ctrl.RedrawWindow();
		}
		else if (button==IDC_SORTGROUP)
		{
			SortTile = 1;
			m_ctrl.InfoList.Sort();
			m_ctrl.RedrawWindow();
		}*/			
		else if (button>=10 && button<=15) 
			m_ctrl.PostMessage(WM_COMMAND,wParam,lParam);
	}
	if (message==WM_LBUTTONDOWN)
	{
		int xPos = LOWORD(lParam);  // horizontal position of cursor 
		int yPos = HIWORD(lParam);  // vertical position of cursor 

		if (lbutton) //on dessine le carre de selection
		{ 
			selection = 1;
			RECT current;
			SIZE size; size.cx = size.cy = 1;
			current.left = OriginalPos.x;
			current.top = OriginalPos.y;
			current.right = m_ctrl.MousePos.x;
			current.bottom = m_ctrl.MousePos.y;
			if (current.left>current.right) {int temp = current.left; current.left = current.right; current.right = temp;}
			if (current.top>current.bottom) {int temp = current.bottom; current.bottom = current.top; current.top = temp;}
			//m_ctrl.UpdateSelection(&current);
			
			CDC *pDC = GetDC();
			m_ctrl.DrawDragRect(pDC,NULL,size,&last_sel,size);			//on efface l'ancien carre
//			::ReleaseDC(*this,*pDC);			
			
			m_ctrl.UpdateSelection(&current,wParam, m_128x128);						//on affiche les modifes
			
//			pDC = GetDC();
			::ReleaseDC(*this,*pDC);			
			
			last_sel = current;
		}
 
		RECT p,rect; p.left = m_ctrl.MousePos.x; p.top = m_ctrl.MousePos.y;
		ClientToScreen(&p); 
		m_ctrl.GetClientRect(&rect);
		POINT pt; pt.x = p.left; pt.y = p.top;
		m_ctrl.ScreenToClient(&pt);
		if (pt.x>=rect.left && pt.x<rect.right && pt.y>=rect.top && pt.y<rect.bottom) 
		{
			m_ctrl.SetFocus();
			int index = m_ctrl.GetIndex(&pt, m_128x128);
			if (index!=-1 && !(wParam&MK_SHIFT)/* && !(wParam&MK_CONTROL)*/)
			{
				tilelist::iterator p = m_ctrl.InfoList.Get(index, m_128x128);
				if (p!=m_ctrl.InfoList.GetLast(m_128x128))
				{
					tilelist::iterator pp = p;
					if (wParam&MK_CONTROL)
						//m_ctrl.InfoList.setSelection (index, p->Selected?0:7);
						p->Selected = p->Selected?0:7;
					else 
						//m_ctrl.InfoList.setSelection (index, 1);
						p->Selected = 1;
					CDC *pDC = NULL;
					int indexx=0;
					for (p=m_ctrl.InfoList.GetFirst(m_128x128);p!=m_ctrl.InfoList.GetLast(m_128x128);++p, indexx++)
					{
						if (p!=pp && p->Selected)
						{
							if (!(wParam&MK_CONTROL))
							{
								//m_ctrl.InfoList.setSelection (indexx, 0);
								p->Selected = 0;
								if (pDC==NULL) pDC = m_ctrl.GetDC();
								m_ctrl.DrawTile(p,pDC,1,m_128x128);
							}
						}
						else
						{
							if (p==pp)
							{
								if (pDC==NULL) pDC = m_ctrl.GetDC();
								m_ctrl.DrawTile(p,pDC,1,m_128x128);
							}
						}
					}
					if (pDC) ::ReleaseDC(*this,*pDC);
				}
			}
			else
			{
				if (!(wParam&MK_CONTROL) && !(wParam&MK_SHIFT))
				{
					tilelist::iterator p = m_ctrl.InfoList.GetFirst(m_128x128);
					CDC *pDC = NULL;
					for (int i = 0; i<m_ctrl.InfoList.GetSize(m_128x128); i++)
					{
						if (p->Selected)
						{
							if (pDC==NULL) pDC = m_ctrl.GetDC();
							//m_ctrl.InfoList.setSelection (i, 0);
							p->Selected = 0;
							m_ctrl.DrawTile(p,pDC,1,m_128x128);
						}
						p++;
					}
					if (pDC) ::ReleaseDC(*this,*pDC);			
				}
			}
			lbutton = 1;
			//CDC *pDC = GetDC();
			SIZE size; size.cx = size.cy = 1;
			/*last_sel.top = m_ctrl.MousePos.x;
			last_sel.left = m_ctrl.MousePos.y;
			last_sel.bottom = m_ctrl.MousePos.x;
			last_sel.right = m_ctrl.MousePos.y;
			OriginalPos = m_ctrl.MousePos;
			*/
			last_sel.top = xPos;
			last_sel.left = yPos;
			last_sel.bottom = xPos;
			last_sel.right = yPos;
			OriginalPos.x=xPos;
			OriginalPos.y=yPos;
		//pDC->DrawDragRect(&last_sel,size,NULL,size);
		//::ReleaseDC(*this,dc);			
		}
	}
	if (message==WM_LBUTTONUP || message==WM_NCLBUTTONUP)
	{
		RECT p; p.left = m_ctrl.MousePos.x; p.top = m_ctrl.MousePos.y;
		ClientToScreen(&p);
		POINT pt; pt.x = p.left; pt.y = p.top;
		m_ctrl.ScreenToClient(&pt);
		int index = m_ctrl.GetIndex(&pt, m_128x128);
		if (!selection && index!=-1)
		{
			int i = 0;
			CDC *pDC = NULL;
	/*		for (tilelist::iterator p=m_ctrl.InfoList.GetFirst();p!=m_ctrl.InfoList.GetLast();p++)
			{
				if (i!=index && p->Selected)
				{
					p->Selected = 0;
					if (pDC == NULL) pDC = GetDC();
					m_ctrl.DrawTile(p,pDC,1);
				}
				i++;
			}*/
			if (pDC) ::ReleaseDC(*this,*pDC);
		}
		else if (selection)
		{
			CDC *pDC = GetDC();
			CSize size; size.cx = size.cy = 1;
			m_ctrl.DrawDragRect(pDC,NULL,size,&last_sel,size);
			::ReleaseDC(*this,*pDC);
			int index=0;
			for (tilelist::iterator p = m_ctrl.InfoList.GetFirst(m_128x128);p!=m_ctrl.InfoList.GetLast(m_128x128);++p, index++)
			{
				if (p->Selected&3)
				{
					p->Selected=2;
					//m_ctrl.InfoList.setSelection (index, 2);
				}
				else 
					p->Selected = 0;
					//m_ctrl.InfoList.setSelection (index, 0);
			}
		}
/*		else if (index==-1)
		{
			int i = 0;
			CDC *pDC = NULL;
			for (tilelist::iterator p=m_ctrl.InfoList.GetFirst();p!=m_ctrl.InfoList.GetLast();p++)
			{
				if ((*p)->Selected)
				{
					(*p)->Selected = 0;
					if (pDC == NULL) pDC = GetDC();
					m_ctrl.DrawTile(p,pDC);
				}
				i++;
			}
			if (pDC) ::ReleaseDC(*this,*pDC);
		}*/
		selection =0;
		lbutton = 0;
	}
	if (message==WM_KEYDOWN)
	{
		int toto = 0;
	}
	if (message==WM_SIZE && m_ctrl.count_ )
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		
		int i = max (1, m_ctrl.GetNbTileLine()); 
		int j = max (1, m_ctrl.GetNbTileColumn());
		int pos = m_ctrl.GetScrollPos(SB_VERT);
		int hview = (m_ctrl.InfoList.GetSize(m_128x128)/i + 1)*(m_ctrl.sizeicon_y + m_ctrl.spacing_y) + m_ctrl.spacing_y;
		m_ctrl.scrollpos = (pos*hview)/SCROLL_MAX;

		RECT clientrect,rect;
		m_ctrl.GetWindowRect(&clientrect);
		InvalidateRect(NULL,false);
		GetWindowRect(&rect);
		m_ctrl.SetWindowPos(NULL, 0, 0, max (100, x - 120), y - 20, SWP_NOMOVE);
		int iFirst,iLast; 
		m_ctrl.GetVisibility(iFirst, iLast, m_128x128);
		m_ctrl.UpdateBar(iFirst, iLast, m_128x128);
		return 0;
	}
	if (message==WM_VSCROLL || message==WM_MOUSEWHEEL)
	{
		SCROLLINFO inf;
		RECT rect_scroll,rect_clip;
		int scrollcode,pos;
		inf.fMask = SIF_ALL;
		GetScrollInfo(SB_VERT,&inf);
		m_ctrl.GetClientRect(&rect_scroll);
		int i = m_ctrl.GetNbTileLine();
		int hview = (m_ctrl.InfoList.GetSize(m_128x128)/i + 2)*(m_ctrl.sizeicon_y + m_ctrl.spacing_y) + m_ctrl.spacing_y;

		if (message==WM_MOUSEWHEEL)
		{
			int inc = ((int)(short)HIWORD(wParam))/WHEEL_DELTA;
			pos = inf.nPos - inc*(((m_ctrl.sizeicon_y+m_ctrl.spacing_y)*SCROLL_MAX)/(hview - m_ctrl.spacing_y));
		}
		else 
		{
			scrollcode = LOWORD(wParam);
			pos = inf.nTrackPos;
			switch (scrollcode)
			{					
				case SB_BOTTOM:
					pos = SCROLL_MAX - inf.nPage;
					break;
				case SB_PAGEDOWN:
					pos = inf.nPos + inf.nPage;
					break;
				case SB_PAGEUP:
					pos = inf.nPos - inf.nPage;
					break;
				case SB_LINEUP:
					pos = inf.nPos - (((m_ctrl.sizeicon_y+m_ctrl.spacing_y)*SCROLL_MAX)/(hview - m_ctrl.spacing_y));
					break;
				case SB_LINEDOWN:
					pos = inf.nPos + (((m_ctrl.sizeicon_y+m_ctrl.spacing_y)*SCROLL_MAX)/(hview - m_ctrl.spacing_y));
					break;
				case SB_TOP:
					pos = 0;
					break;
				case SB_THUMBPOSITION:
				case SB_ENDSCROLL:
					pos = inf.nPos;
					break;
			}
		}

		if (pos<0) pos = 0;
		if (pos>(SCROLL_MAX - (int)inf.nPage)) 
			pos = SCROLL_MAX - inf.nPage;
		
		SetScrollPos(SB_VERT,pos,1);
		CDC *pDC = m_ctrl.GetDC();
		rect_scroll.bottom -= rect_scroll.top;
		rect_scroll.top = 0;
		rect_clip = rect_scroll;
		int scroll_pixel = m_ctrl.scrollpos;
		int old_iFV,old_iLV;
		m_ctrl.GetVisibility(old_iFV, old_iLV, m_128x128);
		m_ctrl.scrollpos = (pos*hview)/(SCROLL_MAX);
		int iFV,iLV;
		m_ctrl.GetVisibility(iFV, iLV, m_128x128);
		
		if (iFV>old_iLV || iLV<old_iFV || scrollcode==SB_PAGEDOWN || scrollcode==SB_PAGEUP)
		{
			m_ctrl.RedrawWindow();
		}
		else
		{
			scroll_pixel -= m_ctrl.scrollpos;
			if (scroll_pixel)
			{
				if (abs(scroll_pixel)>(rect_clip.bottom - rect_clip.top)) scroll_pixel = 0;
				else pDC->ScrollDC(0,scroll_pixel,&rect_scroll,&rect_clip,NULL,NULL);

				tilelist::iterator p = m_ctrl.InfoList.GetFirst(m_128x128);		
				CBrush brush;
				if (scroll_pixel<0)
				{
					rect_scroll.top = rect_scroll.bottom + scroll_pixel;
					pDC->FillRect(&rect_scroll,&brush);
					if ((iLV-i)<iFV) i = iLV - iFV;
					for (int k = 0;k<old_iLV-i;k++) p++;
					for (k=old_iLV - i;k<=iLV;k++) 
					{
						m_ctrl.DrawTile(p,pDC,0,m_128x128);
						p++;
					}
				}
				else
				{
					rect_scroll.bottom = rect_scroll.top + scroll_pixel;
					pDC->FillRect(&rect_scroll,&brush);
					for (int k = 0;k<iFV;k++) p++;
					for (k = iFV;k<(old_iFV+i);k++)
					{
						m_ctrl.DrawTile(p,pDC,0,m_128x128);
						p++;
					}
				}
			}
			m_ctrl.lastVBarPos = pos;
			::ReleaseDC(m_ctrl,*pDC);
		}
	}
	if (message==WM_CLOSE) 
	{
		ccount=0; 
		this->m_ctrl.count_=0;
		OnDestroy();
	}
	if (message==WM_LBUTTONDBLCLK)
	{
		m_ctrl.SendMessage(WM_LBUTTONDBLCLK,wParam,lParam);
	}
	pDialog=this;
	return CDialog::WindowProc(message, wParam, lParam);
}	


unsigned long Browse::MyControllingFunction( void* pParam )
{
	thread_actif = 1;
	Browse *br = (Browse*)pParam;
	br->m_ctrl.lockInsertion = 1;
	int iFV,iLV;
	br->m_ctrl.GetVisibility(iFV, iLV, br->m_128x128);
	br->m_ctrl.UpdateBar(iFV, iLV, br->m_128x128);
	tilelist::iterator p = br->m_ctrl.InfoList.GetFirst(br->m_128x128);
	tilelist::iterator plast = p;
	for (int i=0;i<br->m_ctrl.InfoList.GetSize(br->m_128x128);i++)
	{
		int *ld; 
		std::string path;
		LPBITMAPINFO pBmp; 
		std::vector<NLMISC::CRGBA>* bits;

		switch (br->m_128x128)
		{
		case 0:
			path = tileBank2.getTile (tileBank2.getTileSet (br->m_ctrl.InfoList._tileSet)->getTile128 (i))->
				getFileName ((CTile::TBitmap)(br->m_ctrl.Texture-1));
			break;
		case 1:
			path = tileBank2.getTile (tileBank2.getTileSet (br->m_ctrl.InfoList._tileSet)->getTile256 (i))->
				getFileName ((CTile::TBitmap)(br->m_ctrl.Texture-1));
			break;
		case 2:
			{
				int index=tileBank2.getTileSet (br->m_ctrl.InfoList._tileSet)->getTransition (i)->getTile();
				if (index!=-1)
					path = tileBank2.getTile (index)->getFileName ((CTile::TBitmap)(br->m_ctrl.Texture-1));
				else
					path = "";
			}
			break;
		}
		switch (br->m_ctrl.Texture)
		{
		case 1:
			ld = &p->loaded;
			pBmp = &p->BmpInfo;
			bits = &p->Bits;
			break;
		case 2:
			ld = &p->nightLoaded;
			//path = &p->nightPath;
			pBmp = &p->nightBmpInfo;
			bits = &p->nightBits;
			break;
		case 3:
			ld = &p->bumpLoaded;
			//path = &p->bumpPath;
			pBmp = &p->bumpBmpInfo;
			bits = &p->bumpBits;
			break;
		}

		if ((path!="") /*&& !*ld */&& _LoadBitmap(path, pBmp, *bits))
		{			
			*ld=1;
			int iFV,iLV; br->m_ctrl.GetVisibility(iFV, iLV, br->m_128x128);
			if (i<=iLV && i>=iFV) 
			{
				CDC *pDC = br->m_ctrl.GetDC();
				br->m_ctrl.DrawTile(p,pDC,1,br->m_128x128);
				::ReleaseDC(*br,*pDC);
			}
		}
		p++;
	}
	br->m_ctrl.lockInsertion = 0;
	thread_actif = 0;
	return 1;
}

void Browse::LoadInThread(void)
{
	if (!thread_actif)
	CreateThread(NULL, 0, MyControllingFunction, this, 0, &thread_id);
}


void Browse::Init()
{		
	UpdateData ();
	lbutton = 0;
	selection = 0;
	control = 0;
	m_ctrl.lockInsertion = 0; oldsel = -1;
	HKEY regkey; 
	unsigned long value; 
	unsigned long type; 
	int cx=-1,cy=-1,x=-1,y=-1;
	char sWindowpl[256];

	if (RegOpenKey(HKEY_CURRENT_USER,REGKEY_TILEDIT,&regkey)==ERROR_SUCCESS)
	{		
		value=256;
		type=REG_SZ;
		if (RegQueryValueEx(regkey,REGKEY_WNDPL,0,&type,(unsigned char *)&sWindowpl,&value)==ERROR_SUCCESS)
		{
			WINDOWPLACEMENT wndpl;
			sscanf(sWindowpl,"%d %d %d %d %d %d %d %d %d %d",
						&wndpl.flags,
						&wndpl.ptMaxPosition.x,&wndpl.ptMaxPosition.y,
						&wndpl.ptMinPosition.x,&wndpl.ptMinPosition.y,
						&wndpl.rcNormalPosition.bottom,&wndpl.rcNormalPosition.left,&wndpl.rcNormalPosition.right,&wndpl.rcNormalPosition.top,
						&wndpl.showCmd);
			wndpl.length = sizeof(WINDOWPLACEMENT);
			this->SetWindowPlacement(&wndpl);
		}
		value=256;
		type=REG_SZ;
		if (RegQueryValueEx(regkey,REGKEY_LASTPATH,0,&type,(unsigned char *)&sWindowpl,&value)!=ERROR_SUCCESS)
			m_ctrl.LastPath="";
		else
			m_ctrl.LastPath=(const char*)sWindowpl;
		value=4;
		type=REG_DWORD;
		if (RegQueryValueEx(regkey,REGKEY_BUTTONZOOM,0,&type,(unsigned char *)&m_ctrl.Zoom,&value)!=ERROR_SUCCESS) 
			m_ctrl.Zoom = 3;
		value=4;
		type=REG_DWORD;
		if (RegQueryValueEx(regkey,REGKEY_BUTTONVARIETY,0,&type,(unsigned char *)&m_128x128,&value)!=ERROR_SUCCESS) 
			m_128x128 = 0;
		value=4;
		type=REG_DWORD;
		if (RegQueryValueEx(regkey,REGKEY_BUTTONTEXTURE,0,&type,(unsigned char *)&m_ctrl.Texture,&value)!=ERROR_SUCCESS) 
			m_ctrl.Texture = 1;
		value=4;
		type=REG_DWORD;
		if (RegQueryValueEx(regkey,REGKEY_BUTTONTEXTINFO,0,&type,(unsigned char *)&m_ctrl.InfoTexte,&value)!=ERROR_SUCCESS) 
			m_ctrl.InfoTexte = 1;
		RegCloseKey(regkey);
	}		
	CButton *button = (CButton*)GetDlgItem(IDC_ZOOM1 + m_ctrl.Zoom -1);
	button->SetCheck(1);
	/*button = (CButton*)GetDlgItem(IDC_NUM + m_ctrl.Sort -1);
	button->SetCheck(1);*/
	button = (CButton*)GetDlgItem(IDC_JOUR + m_ctrl.Texture -1);
	button->SetCheck(1);
	button = (CButton*)GetDlgItem(IDC_INFONUM + m_ctrl.InfoTexte -1);
	button->SetCheck(1);	
	if (cx!=-1 && cy!=-1 && x!=-1 && y!=-1) SetWindowPos(0,x,y,cx,cy,0);

	m_ctrl.Init(land, m_128x128);
//	_parent = (void*)GetParent();
	SelectionTerritoire *slt = (SelectionTerritoire*)GetParent();
	ccount=1;
	
	RECT rect;
	this->GetWindowRect(&rect);
	SendMessage(WM_SIZE,rect.right - rect.left,rect.bottom - rect.top); //force resize

/*	DWORD thread_id;
	CreateThread(NULL, SCROLL_MAX, MyControllingFunction, this, 0, &thread_id);*/	
	/*CComboBox *list = (CComboBox*)GetDlgItem(IDC_LISTTYPE);		
	list->InsertString(0,"all"); list->InsertString(1,"custom...");*/
	SelectionTerritoire *parent = (SelectionTerritoire*)GetParent();

	// The land	
	CTileSet *tileSet=tileBank2.getTileSet (land);

	// 128
	m_ctrl.InfoList.theList128.resize (tileSet->getNumTile128 ());
	for (int i=0; i<tileSet->getNumTile128 (); i++)
	{
		m_ctrl.InfoList.theList128[i].id=i;
		m_ctrl.InfoList.theList128[i].Selected=0;
		m_ctrl.InfoList.theList128[i].loaded=0;
		m_ctrl.InfoList.theList128[i].nightLoaded=0;
		m_ctrl.InfoList.theList128[i].bumpLoaded=0;
	}
	m_ctrl.InfoList.Reload (0, tileSet->getNumTile128 (), 0);

	// 256
	m_ctrl.InfoList.theList256.resize (tileSet->getNumTile256 ());
	for (i=0; i<tileSet->getNumTile256 (); i++)
	{
		m_ctrl.InfoList.theList256[i].id=i;
		m_ctrl.InfoList.theList256[i].Selected=0;
		m_ctrl.InfoList.theList256[i].loaded=0;
		m_ctrl.InfoList.theList256[i].nightLoaded=0;
		m_ctrl.InfoList.theList256[i].bumpLoaded=0;
	}
	m_ctrl.InfoList.Reload (0, tileSet->getNumTile256 (), 1);

	// Transition
	for (i=0; i<CTileSet::count; i++)
	{
		m_ctrl.InfoList.theListTransition[i].id=i;
		m_ctrl.InfoList.theListTransition[i].Selected=0;
		m_ctrl.InfoList.theListTransition[i].loaded=0;
		m_ctrl.InfoList.theListTransition[i].nightLoaded=0;
		m_ctrl.InfoList.theListTransition[i].bumpLoaded=0;
	}
	m_ctrl.InfoList.Reload (0, CTileSet::count, 2);

	/*// Group name
	int j;
	for (j=0; j<pLand->getTileTypeCount(); j++)
	{
		list->InsertString(list->GetCount(), pLand->getTileType(j).c_str() );
	}
	
	// Normal tile name
	for (j=0; j<pLand->getTileCount(); j++)
	{
		CTileBankTile* pTile=pLand->getTile (j);
		const char* name=pTile->getFileName (CTileBankTile::diffuse).c_str();
		const char* nameNight=pTile->getFileName (CTileBankTile::additive).c_str();
		const char* nameBump=pTile->getFileName (CTileBankTile::bump).c_str();
		if (strcmp ( name, "")==0)
			name = NULL;
		if (strcmp ( nameNight, "")==0)
			nameNight = NULL;
		if (strcmp ( nameBump, "")==0)
			nameBump = NULL;
		m_ctrl.InfoList.Add(name, nameNight, nameBump,  pTile->getTransition(CTileBankTile::north), 
			pTile->getTransition(CTileBankTile::south), pTile->getTransition(CTileBankTile::west), 
			pTile->getTransition(CTileBankTile::east), pTile->getTypeMask ());
	}*/


	CString fullpath = parent->DefautPath + parent->CurrentTerritory;
#if 0
	char *DefautPath = parent->DefautPath.GetBuffer(256);

	char *str = fullpath.GetBuffer(256);
	char TilePath[256];
	FILE *ptr = fopen(str,"rt");
	if (!ptr) return;
	int i=0; int h,d,g,b;
	int nGroup,cursel;	
	fscanf(ptr,"%d %d\n",&nGroup,&cursel);
	for (int j=0;j<nGroup;j++)
	{
		char sName[100];
		fscanf(ptr,"%s\n",sName);
		list->InsertString(list->GetCount(),sName);
	}
	if (cursel!=-1)
	{
		list->SetCurSel(cursel);
	}
	while (1)
	{
		int g1,g2;
		tilelist::const_iterator p;
		fscanf(ptr,"%Xh%Xh %d %d %d %d\n",&g1,&g2,&h,&g,&b,&d);
		if (feof(ptr)) break;
		int s=ftell(ptr);
		fgets(TilePath,256,ptr);
		s=ftell(ptr)-s-2;
		TilePath[s]=0;
		if (!strcmp(TilePath,"void"))
		{
			m_ctrl.InfoList.Add(0,m_ctrl.Texture);
		}
		else 
		{
			if (TilePath[1]!=':') //check if it's a relative path
			{
				char temp[256];
				sprintf(temp,"%s%s%c",DefautPath,TilePath,0);
				strcpy(TilePath,temp);
			}
			if (m_ctrl.InfoList.Add(TilePath,1))
			{
				p = m_ctrl.InfoList.theList.end(); p--;
				p->groupFlag = (((__int64)g1)<<32 | g2);
				p->h = h;
				p->g = g;
				p->b = b;
				p->d = d;
			}
		}
		
		if (feof(ptr)) break;
		s=ftell(ptr);
		fgets(TilePath,256,ptr);
		s=ftell(ptr)-s-2;
		TilePath[s]=0;
		if (strcmp(TilePath,"void")) 
		{
			if (TilePath[1]!=':') //check if it's a relative path
			{
				char temp[256];
				sprintf(temp,"%s%s%c",DefautPath,TilePath,0);
				strcpy(TilePath,temp);
			}
			p = m_ctrl.InfoList.theList.end(); p--;
			p->nightPath = new char[strlen(TilePath)+1];
			strcpy(p->nightPath,TilePath);
		}
		if (feof(ptr)) break;
		s=ftell(ptr);
		fgets(TilePath,256,ptr);
		s=ftell(ptr)-s-2;
		TilePath[s]=0;
		if (strcmp(TilePath,"void")) 
		{
			if (TilePath[1]!=':') //check if it's a relative path
			{
				char temp[256];
				sprintf(temp,"%s%s%c",DefautPath,TilePath,0);
				strcpy(TilePath,temp);
			}
			p = m_ctrl.InfoList.theList.end(); p--;
			p->bumpPath = new char[strlen(TilePath)+1];
			strcpy(p->bumpPath,TilePath);
		}
		i++;
	}
	fclose(ptr);
	//m_ctrl.InsertItemInCtrlList(m_ctrl.InfoList.GetFirst(),m_ctrl.InfoList.GetLast());
#endif // 0
/*
	fullpath += EDGEFILE_EXT;
	char *str;
	str = fullpath.GetBuffer(256);
	FILE *ptr = fopen(str,"rb");
	if (!ptr) return;
	int n;
	fread(&n,1,sizeof(int),ptr);
	for (int h=0;h<n;h++)
	{
		_Edge edge;
		fread(&edge.size,1,sizeof(int),ptr);
		edge.line = new char[edge.size*3];
		fread(edge.line,1,edge.size*3,ptr);
		while (m_ctrl.smEdgeList) {}
		m_ctrl.smEdgeList = 1;
		m_ctrl.EdgeList.insert(m_ctrl.EdgeList.end(),edge);
		m_ctrl.smEdgeList = 0;
	}
	fclose(ptr);*/
	
	//OnSelchangeListtype();
	LoadInThread();
	UpdateData (FALSE);
}



void Browse::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
/*	if (!m_ctrl.count_) return;
	RECT rect,parent;
	GetWindowRect(&parent);
	m_ctrl.GetWindowRect(&rect);*/
//	m_ctrl.SetWindowPos(NULL,180,20,cx - (rect.left - parent.left) - 20,cy - (rect.top - parent.top),SWP_SHOWWINDOW | SWP_NOZORDER);


}

void Browse::OnBump() 
{
	// TODO: Add your control notification handler code here
	m_ctrl.Texture = 3;
	LoadInThread();
	m_ctrl.RedrawWindow();
}

void Browse::OnJour() 
{
	// TODO: Add your control notification handler code here
	m_ctrl.Texture = 1;
	LoadInThread();
	m_ctrl.RedrawWindow();
}

void Browse::OnNuit() 
{
	// TODO: Add your control notification handler code here
	m_ctrl.Texture = 2;
	LoadInThread();
	m_ctrl.RedrawWindow();
}

void Browse::OnNum() 
{
	// TODO: Add your control notification handler code here
	m_ctrl.Sort = 1;	
	m_ctrl.SendMessage(WM_PAINT);
}

void Browse::OnCancel() 
{
	// TODO: Add your control notification handler code here
	if (thread_actif) return;

	if (::MessageBox (NULL, "Are you sure you want to cancel?", "Cancel", MB_OK|MB_ICONQUESTION|MB_YESNO)==IDYES)
	{
		this->SendMessage(WM_CLOSE);
		CDialog::OnCancel();
		/*
		EndDialog(0);*/
	}
}

void Browse::UpdateAll(void)
{
	
}

void Browse::OnDestroy()
{
	// TODO: Add your control notification handler code here
	HKEY regkey;
	WINDOWPLACEMENT wndpl;
	this->GetWindowPlacement(&wndpl);
	char sWindowpl[256];
	sprintf(sWindowpl,"%d %d %d %d %d %d %d %d %d %d",
						wndpl.flags,
						wndpl.ptMaxPosition.x,wndpl.ptMaxPosition.y,
						wndpl.ptMinPosition.x,wndpl.ptMinPosition.y,
						wndpl.rcNormalPosition.bottom,wndpl.rcNormalPosition.left,wndpl.rcNormalPosition.right,wndpl.rcNormalPosition.top,
						wndpl.showCmd);
	if (RegCreateKey(HKEY_CURRENT_USER,REGKEY_TILEDIT,&regkey)==ERROR_SUCCESS)
	{	
		//int sel = ((CComboBox*)GetDlgItem(IDC_LISTTYPE))->GetCurSel();
		RegSetValueEx(regkey,REGKEY_WNDPL,0,REG_SZ,(const unsigned char*)sWindowpl,strlen(sWindowpl));
		RegSetValueEx(regkey,REGKEY_LASTPATH,0,REG_SZ,(const unsigned char*)m_ctrl.LastPath.c_str(),strlen(m_ctrl.LastPath.c_str()));
		RegSetValueEx(regkey,REGKEY_BUTTONZOOM,0,REG_DWORD,(const unsigned char*)&m_ctrl.Zoom,4);
		RegSetValueEx(regkey,REGKEY_BUTTONVARIETY,0,REG_DWORD,(const unsigned char*)&m_128x128,4);
		RegSetValueEx(regkey,REGKEY_BUTTONTEXTURE,0,REG_DWORD,(const unsigned char*)&m_ctrl.Texture,4);
		RegSetValueEx(regkey,REGKEY_BUTTONSORT,0,REG_DWORD,(const unsigned char*)&m_ctrl.Sort,4);
		RegSetValueEx(regkey,REGKEY_BUTTONTEXTINFO,0,REG_DWORD,(const unsigned char*)&m_ctrl.InfoTexte,4);
		//RegSetValueEx(regkey,REGKEY_LISTCOMBOBOX,0,REG_DWORD,(const unsigned char*)&sel,4);
		RegCloseKey(regkey);
	}
}

void Browse::OnOk() 
{
	// TODO: Add your control notification handler code here
	if (thread_actif) return;

	if (::MessageBox (NULL, "Are you sure you want to cancel?", "Cancel", MB_OK|MB_ICONQUESTION|MB_YESNO)==IDYES)
	{
		this->SendMessage(WM_CLOSE);
		EndDialog(1);
	}
}

void Browse::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_ctrl.PostMessage(WM_RBUTTONDOWN,point.x,point.y);
	CDialog::OnRButtonDown(nFlags, point);
}

void Browse::OnSelchangeListtype() 
{
	// TODO: Add your control notification handler code here
	/*CComboBox *list = (CComboBox*)GetDlgItem(IDC_LISTTYPE);
	int sel = list->GetCurSel();
	showNULL = 0;
	if (sel!=LB_ERR && list->GetCount()>=2 && oldsel!=sel)
	{
		sortMode = 1;
		if (sel==0)
		{
			SortTile = 0;
			flagGroupSort = 0;
			m_ctrl.InfoList.Sort();
		}
		else if (sel==1 && list->GetCount()>=2) //custom ...
		{
			Custom dialog;
			dialog.clist = (CComboBox*)GetDlgItem(IDC_LISTTYPE);
			dialog.DoModal();
			if (dialog.bOk)
			{
				flagGroupSort = dialog.flag;
				SortTile = 1;
				if (!flagGroupSort) showNULL = 1;				
				sortMode = dialog.mode;
				m_ctrl.InfoList.Sort();
			}
		}
		else if (sel>=2)
		{
			SortTile = 1;
			flagGroupSort = 1;
			for (int i = 2;i<sel;i++) flagGroupSort<<=1;
			m_ctrl.InfoList.Sort();
		}
		m_ctrl.RemoveSelection();
		LoadInThread();
		m_ctrl.RedrawWindow();
	}*/
}

void Browse::OnUpdateTiles() 
{
	// TODO: Add your control notification handler code here
	/*__int64 temp = flagGroupSort;
	flagGroupSort = 0;*/
	LoadInThread();
	//flagGroupSort = temp;
	//m_ctrl.EdgeList.clear();
	/*for (tilelist::iterator p = m_ctrl.InfoList.theList.begin();p!=m_ctrl.InfoList.theList.end();++p) 
		m_ctrl.CheckTile(&(*p));*/
}


void Browse::OnChangeVariety()
{
	UpdateData();
	m_ctrl.UpdateSize(m_128x128);
	m_ctrl.Invalidate ();
	/*int iFV,iLV;
	GetVisibility(iFV, iLV, parent->m_128x128);
	UpdateBar(iFV, iLV, parent->m_128x128);*/
	UpdateData(FALSE);
}