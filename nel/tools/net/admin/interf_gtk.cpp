/** \file interf_dos.cpp
 * 
 *
 * $Id: interf_gtk.cpp,v 1.3 2001/06/07 16:18:17 lecroart Exp $
 *
 *
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifdef INTERF_GTK

#include <string>
#include <conio.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "nel/misc/debug.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"

#include "nel/net/net_manager.h"

#include "datas.h"
#include "connection_as.h"
#include "interf.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

#ifdef NL_OS_WINDOWS
// automatically add gtk library
#pragma comment(lib, "gtk-1.3.lib")
#pragma comment(lib, "gdk-1.3.lib")
#pragma comment(lib, "glib-1.3.lib")
#pragma comment(lib, "gthread-1.3.lib")
#endif

//
// Variables
//

GtkWidget *RootWindow;

GtkWidget *RootTree;
GtkWidget *RootTreeItem;
GtkWidget *RootSubTree;

GtkWidget *Toolbar;
GtkWidget *ConnectButton;

GtkWidget *OutputText;
GtkWidget *InputText;

static CLog logstdout;

class CGtkDisplayer;
static CGtkDisplayer *GtkDisplayer;

//
// Functions
//

/*void cbStartService ()
{
	nlinfo("staring service ...");
}

void removeServiceAliasPopup (CAdminExecutorService *aes)
{
	for (uint i = 0; i < aes->ServiceAliasList.size(); i++)
	{
		string name = "/Services/";
		name += aes->ServiceAliasList[i];
		gtk_item_factory_delete_item (GTK_ITEM_FACTORY(aes->ItemFactory), name.c_str());
	}
}

void addServiceAliasPopup (CAdminExecutorService *aes)
{
	for (uint i = 0; i < aes->ServiceAliasList.size(); i++)
	{
		string name = "/Services/";
		name += aes->ServiceAliasList[i];
		GtkItemFactoryEntry ife;
		ife.path = const_cast<char *>(name.c_str());
		ife.accelerator = NULL;
		ife.callback = cbStartService;
		ife.callback_action = 0;
		ife.item_type = NULL;
		gtk_item_factory_create_items (GTK_ITEM_FACTORY(aes->ItemFactory), 1, &ife, &(aes->ServiceAliasList[i]));
	}
}
*/

static GdkColor DisplayerColors[] =
{
	{ 0, 0x0000, 0x0000, 0x0000 }, //"black" },
	{ 0, 0xFFFF, 0xFFFF, 0xFFFF }, //"white" },
	{ 0, 0xFFFF, 0x0000, 0x0000 }, //"red" },
	{ 0, 0x0000, 0xFFFF, 0x0000 }, //"green" },
	{ 0, 0x0000, 0x0000, 0xFFFF }, //"blue" }, 
	{ 0, 0x0000, 0xFFFF, 0xFFFF }, //"cyan" },
	{ 0, 0xFFFF, 0x0000, 0xFFFF }, //"magenta" },
	{ 0, 0xFFFF, 0xFFFF, 0x0000 }, //"yellow" }
};

class CGtkDisplayer : virtual public IDisplayer
{
public:
	CGtkDisplayer (uint threadid) : _ThreadId(threadid) { }
private:
	uint _ThreadId;
protected:

	/// Display the string to stdout and OutputDebugString on Windows
	virtual void doDisplay ( const TDisplayInfo& args, const char *message )
	{
		// can't display stuffs of other thread
		if (_ThreadId != getThreadId ())
			return;

		GtkAdjustment *Adj = (GTK_TEXT(OutputText))->vadj;
		bool Bottom = (Adj->value >= Adj->upper - Adj->page_size);

		sint colnum;
		switch (args.LogType)
		{
		case CLog::LOG_INFO: colnum = 0; break;
		case CLog::LOG_WARNING: colnum = 2; break;
		case CLog::LOG_ERROR: colnum = 2; break;
		default: colnum = 0; break;
		}

		gtk_text_freeze (GTK_TEXT (OutputText));
		gtk_text_insert (GTK_TEXT (OutputText), NULL, &DisplayerColors[colnum], NULL, message, -1);
		gtk_text_thaw (GTK_TEXT (OutputText));

		if (Bottom)
		{
			gtk_adjustment_set_value(Adj,Adj->upper-Adj->page_size);
		}

	}
};


void setBitmap (const string &bitmapName, void *&bitmap)
{
	GtkStyle *style = gtk_widget_get_style (RootWindow);
	GdkPixmap *mask;
	//GdkPixmap *pixmap = gdk_pixmap_create_from_xpm (RootWindow->window, &mask, &style->bg[GTK_STATE_NORMAL], bitmapName.c_str());

	// use this to avoid warning when the window is not realized/shown
	GdkPixmap *pixmap = gdk_pixmap_colormap_create_from_xpm (NULL, gtk_widget_get_colormap(RootWindow), &mask, NULL, bitmapName.c_str());

	if (pixmap == NULL) nlerror ("bitmap '%s' not found", bitmapName.c_str());

	if (bitmap == NULL)
		bitmap = gtk_pixmap_new (pixmap, mask);
	else
		gtk_pixmap_set (GTK_PIXMAP (bitmap), pixmap, mask);
}

void setLabel (const string &text, void *&label)
{
	gtk_label_set_text (GTK_LABEL (label), text.c_str());
}

void createTreeItem (GtkWidget *rootTree, void *&treeItem, void *&bitmap, void *&label)
{
	nlassert (treeItem == NULL);

	treeItem = gtk_tree_item_new ();

    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);

	bitmap = NULL;
	setBitmap ("empty.xpm", bitmap);

	gtk_widget_show (GTK_WIDGET(bitmap));
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(bitmap), FALSE, FALSE, 0);

	label = gtk_label_new ("<Unknown>");
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show (GTK_WIDGET(label));
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(label), FALSE, FALSE, 0);

	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (treeItem), hbox);

	gtk_tree_append (GTK_TREE (rootTree), GTK_WIDGET(treeItem));

	gtk_widget_show ((GtkWidget *)treeItem);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////// POPUP MENU FOR AS /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


CAdminService *PopupAS = NULL;

// POPUP MENU
GtkWidget *ASMenu;
gint cbPopupASMenu (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS && event->button.button == 3)
	{
		GdkEventButton *bevent = (GdkEventButton *) event; 
		gtk_menu_popup (GTK_MENU (data), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
		gtk_signal_emit_stop_by_name (GTK_OBJECT(widget), "button_press_event");

		ASIT asit;
		for (asit = AdminServices.begin(); asit != AdminServices.end(); asit++)
		{
			if ((*asit).RootTreeItem == widget)
			{
				PopupAS = &(*asit);
				return TRUE;
			}
		}
		nlstop;
	}
	return TRUE;
}


static void cbConnectToAS()
{
/*	GtkItemFactoryEntry ife;
	ife.path = "/Services/oho";
	ife.accelerator = NULL;
	ife.callback = NULL;
	ife.callback_action = 0;
	ife.item_type = NULL;

	gtk_item_factory_create_items (item_factory, 1, &ife, NULL);
*/
	if (PopupAS->Connected)
	{
		nlwarning("already connected!!!");
		return;
	}

	printf("connecting to...%p\n", PopupAS);

	connectionASInit (PopupAS);

	PopupAS = NULL;
}

static void cbDisconnectToAS()
{
	if (!PopupAS->Connected)
	{
		nlwarning("not connected!!!");
		return;
	}

	printf("disconnecting to...%p\n", PopupAS);

	setBitmap ("as_dcnt.xpm", PopupAS->Bitmap);

	connectionASRelease (PopupAS);

	PopupAS = NULL;
}

static void cbStartAllServices()
{
	nlassert (PopupAS != NULL);
	
	string cmd = "startall ";
	cmd += toString (PopupAS->Id);
	ICommand::execute (cmd, logstdout);

	PopupAS = NULL;
}

static void cbStopAllServices()
{
	nlassert (PopupAS != NULL);
	
	string cmd = "stopall ";
	cmd += toString (PopupAS->Id);
	ICommand::execute (cmd, logstdout);

	PopupAS = NULL;
}

static GtkItemFactoryEntry ASMenuItems[] = {
	{ "/Connect", NULL, cbConnectToAS, 0, NULL },
	{ "/Disconnect", NULL, cbDisconnectToAS, 0, NULL },
	{ "/Start All Services", NULL, cbStartAllServices, 0, NULL },
	{ "/Stop All Services", NULL, cbStopAllServices, 0, NULL },
//	{ "/Services", NULL, NULL, 0, "<Branch>" },
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////// POPUP MENU FOR S //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


CService *PopupS = NULL;

// POPUP MENU
//GtkWidget *SMenu;
gint cbPopupSMenu (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS && event->button.button == 3)
	{
		GdkEventButton *bevent = (GdkEventButton *) event; 
		gtk_menu_popup (GTK_MENU (data), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
		gtk_signal_emit_stop_by_name (GTK_OBJECT(widget), "button_press_event");

		ASIT asit;
		for (asit = AdminServices.begin(); asit != AdminServices.end(); asit++)
		{
			AESIT aesit;
			for (aesit = (*asit).AdminExecutorServices.begin(); aesit != (*asit).AdminExecutorServices.end(); aesit++)
			{
				SIT sit;
				for (sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
				{
					if ((*sit).RootTreeItem == widget)
					{
						PopupS = &(*sit);
						return TRUE;
					}
				}
			}
		}
		nlstop;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////// POPUP MENU FOR AES ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/*
CAdminExecutorService *PopupAES = NULL;

gint cbPopupAESMenu (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS && event->button.button == 3)
	{
		GdkEventButton *bevent = (GdkEventButton *) event; 
		gtk_menu_popup (GTK_MENU (data), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
		gtk_signal_emit_stop_by_name (GTK_OBJECT(widget), "button_press_event");

		for (ASIT asit = AdminServices.begin(); asit != AdminServices.end(); asit++)
		{
			for (AESIT aesit = (*asit).AdminExecutorServices.begin(); aesit != (*asit).AdminExecutorServices.end(); aesit++)
			{
				if ((*aesit).RootTreeItem == widget)
				{
					PopupAES = &(*aesit);
					return TRUE;
				}
			}
		}
		nlstop;
	}
	return FALSE;
}
*/
/*
static void cbStartAllServices()
{
	nlassert (PopupAES != NULL);
	
	string cmd = "startall ";
	cmd += toString (AES->Id);
	ICommand::execute (cmd, logstdout);

	PopupAES = NULL;
}
*/
/*static GtkItemFactoryEntry AESMenuItems[] = {
	{ "/Start All Services", NULL, cbStartAllServices, 0, NULL }
};
*/

/*
CAdminService *ASSelected = NULL;
void cbASSelected (GtkWidget *widget, gpointer data) { ASSelected = (CAdminService *) data; printf("sel\n"); gtk_widget_set_sensitive (ConnectButton, TRUE); }
void cbASDeselected (GtkWidget *widget, gpointer data) { ASSelected = NULL; printf("desel\n"); gtk_widget_set_sensitive (ConnectButton, FALSE); }
*/
void interfAddAS (CAdminService *as)
{
	if (AdminServices.size () == 1)
	{
		// it's the first AS, we have to add the subtree
		RootSubTree = gtk_tree_new();
	}

	// create the item
	createTreeItem (RootSubTree, as->RootTreeItem, as->Bitmap, as->Label);
	
	setBitmap ("as_dcnt.xpm", as->Bitmap);
	
	string name;
	name = "AS";
	name += toString(as->Id);
	name += " '";
	name += as->ASName;
	name += "' (";
	name += as->ASAddr;
	name += ")";
	setLabel (name, as->Label);

//	gtk_signal_connect (GTK_OBJECT (as->RootTreeItem), "select", GTK_SIGNAL_FUNC(cbASSelected), as);
//	gtk_signal_connect (GTK_OBJECT (as->RootTreeItem), "deselect", GTK_SIGNAL_FUNC(cbASDeselected), as);
	gtk_signal_connect (GTK_OBJECT (as->RootTreeItem), "button-press-event", GTK_SIGNAL_FUNC(cbPopupASMenu), ASMenu);

	if (AdminServices.size () == 1)
	{
		// it's the first AS, we have to add the subtree
		gtk_tree_item_set_subtree (GTK_TREE_ITEM (RootTreeItem), RootSubTree);
		gtk_tree_item_expand (GTK_TREE_ITEM (RootTreeItem));
	}
}

void interfUpdateAES (CAdminExecutorService *aes)
{
	string name;
	name = "AES";
	name += toString(aes->Id);
	name += " '";
	name += aes->ServerAlias;
	name += "' (";
	name += aes->ServerAddr;
	name += ")";

	string icon;
	if (aes->Connected)
		icon = "aes_cnt.xpm";
	else
		icon = "aes_dcnt.xpm";

	// check if we already create widgets
	nlassert (aes->RootTreeItem != NULL);

	setBitmap (icon, aes->Bitmap);
	setLabel (name, aes->Label);
}


void interfAddAES (CAdminService *as, CAdminExecutorService *aes)
{
	if (as->AdminExecutorServices.size () == 1)
	{
		// it's the first AS, we have to add the subtree
		as->RootSubTree = gtk_tree_new();
	}

	// create the item
	createTreeItem (GTK_WIDGET(as->RootSubTree), aes->RootTreeItem, aes->Bitmap, aes->Label);
	interfUpdateAES (aes);
/*
	// AES POPUP MENU
	GtkAccelGroup *accel_group;
	gint nmenu_items = sizeof (AESMenuItems) / sizeof (AESMenuItems[0]);
	accel_group = gtk_accel_group_new ();
	aes->ItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<main>", accel_group);
	gtk_item_factory_create_items (GTK_ITEM_FACTORY(aes->ItemFactory), nmenu_items, AESMenuItems, NULL);
	gtk_window_add_accel_group (GTK_WINDOW (RootWindow), accel_group);
	GtkWidget *menu = gtk_item_factory_get_widget (GTK_ITEM_FACTORY(aes->ItemFactory), "<main>");

	gtk_signal_connect (GTK_OBJECT (aes->RootTreeItem), "button-press-event", GTK_SIGNAL_FUNC(cbPopupAESMenu), menu);
*/
	if (as->AdminExecutorServices.size () == 1)
	{
		// it's the first AS, we have to add the subtree
		gtk_tree_item_set_subtree (GTK_TREE_ITEM (as->RootTreeItem), (GtkWidget*)as->RootSubTree);
		gtk_tree_item_expand (GTK_TREE_ITEM (as->RootTreeItem));
	}
}


static void cbStartService()
{
	if (PopupS->Connected)
	{
		nlwarning("already connected!!!");
		return;
	}

	string cmd = "start";
	cmd += " ";
	cmd += toString (PopupS->AES->AS->Id);
	cmd += " ";
	cmd += toString (PopupS->AES->Id);
	cmd += " ";
	cmd += PopupS->AliasName;

	ICommand::execute (cmd, logstdout);

//	connectionASInit (PopupAS);

	PopupS = NULL;
}

static void cbStopService()
{
	if (!PopupS->Connected)
	{
		nlwarning("not connected!!!");
		return;
	}

	printf("disconnecting to...%p\n", PopupS);

	string cmd = "stop";
	cmd += " ";
	cmd += toString (PopupS->AES->AS->Id);
	cmd += " ";
	cmd += toString (PopupS->AES->Id);
	cmd += " ";
	cmd += toString (PopupS->Id);

	ICommand::execute (cmd, logstdout);

//	connectionASRelease (PopupAS);

	PopupS = NULL;
}

static GtkItemFactoryEntry SMenuItems[] = {
	{ "/Start Service", NULL, cbStartService, 0, NULL },
	{ "/Stop Service", NULL, cbStopService, 0, NULL },
	{ "/Commands", NULL, NULL, 0, "<Branch>" },
};

void cbDoIt ()
{
	// todo connecter la bonne fonction, retrouver tout le bordel (bon courage...)
	nlinfo("execute command...");
}

void interfUpdateS (CService *s)
{
	string name;
	name = "S";
	name += toString(s->Id);
	name += " '";
	name += s->AliasName;
	name += "' '";
	name += s->ShortName;
	name += "' '";
	name += s->LongName;
	name += "' (U";
	name += toString(s->Unknown);
	name += ", C";
	name += toString(s->Connected);
	name += ", I";
	name += toString(s->InConfig);
	name += ", R";
	name += toString(s->Ready);
	name += ")";

	string icon;
	if (s->Unknown)
		icon = "s_ukn.xpm";
	else if (s->Connected)
		icon = "s_cnt.xpm";
	else
		icon = "s_dcnt.xpm";

	// check if we already create widgets
	nlassert (s->RootTreeItem != NULL);

	setBitmap (icon, s->Bitmap);
	setLabel (name, s->Label);

	if (!s->Commands.empty() && !s->MenuCreated)
	{
		// add new commands on the menu
		for (uint i = 0; i < s->Commands.size(); i++)
		{
			string name = "/Commands/";
			name += s->Commands[i];
			GtkItemFactoryEntry ife;
			ife.path = const_cast<char *>(name.c_str());
			ife.accelerator = NULL;
			ife.callback = cbDoIt;
			ife.callback_action = 0;
			ife.item_type = NULL;
			gtk_item_factory_create_items (GTK_ITEM_FACTORY(s->ItemFactory), 1, &ife, &(s->Commands[i]));
		}
		s->MenuCreated = true;
	}
	else if (!s->Connected && s->MenuCreated)
	{
		// delete all commands
		for (uint i = 0; i < s->Commands.size(); i++)
		{
			string name = "/Commands/";
			name += s->Commands[i];
			gtk_item_factory_delete_item (GTK_ITEM_FACTORY(s->ItemFactory), name.c_str());
		}
		s->Commands.clear ();
		s->MenuCreated = false;
	}
}

void interfAddS (CAdminExecutorService *aes, CService *s)
{
	if (aes->Services.size () == 1)
	{
		// it's the first AS, we have to add the subtree
		aes->RootSubTree = gtk_tree_new();
	}

	//
	// Create the item
	//

	createTreeItem (GTK_WIDGET(aes->RootSubTree), s->RootTreeItem, s->Bitmap, s->Label);

	//
	// Create the popupmenu
	//

	gint nmenu_items = sizeof (SMenuItems) / sizeof (SMenuItems[0]);
	GtkAccelGroup *accel_group = gtk_accel_group_new ();
	s->ItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<main>", accel_group);
	gtk_item_factory_create_items (GTK_ITEM_FACTORY(s->ItemFactory), nmenu_items, SMenuItems, NULL);
	gtk_window_add_accel_group (GTK_WINDOW (RootWindow), accel_group);
	GtkWidget *menu = gtk_item_factory_get_widget (GTK_ITEM_FACTORY(s->ItemFactory), "<main>");

	gtk_signal_connect (GTK_OBJECT (s->RootTreeItem), "button-press-event", GTK_SIGNAL_FUNC(cbPopupSMenu), menu);

	interfUpdateS (s);

	if (aes->Services.size () == 1)
	{
		// it's the first AS, we have to add the subtree
		gtk_tree_item_set_subtree (GTK_TREE_ITEM (aes->RootTreeItem), (GtkWidget*)aes->RootSubTree);
		gtk_tree_item_expand (GTK_TREE_ITEM (aes->RootTreeItem));
	}
}

void removeSubTree (CAdminService *as)
{
	// bug todo kan on kill un admin service et que on a selectionner un sous fils du subtree
	GList *l = GTK_TREE_SELECTION(RootTree);
	if (l != NULL)
	{
		GtkWidget *g = GTK_WIDGET (l->data);
		gtk_tree_item_deselect (GTK_TREE_ITEM(g));
	}
	gtk_tree_item_remove_subtree (GTK_TREE_ITEM(as->RootTreeItem));
}

void interfRemoveS (CService *s)
{
	gtk_container_remove (GTK_CONTAINER(GTK_WIDGET(s->RootTreeItem)->parent), GTK_WIDGET(s->RootTreeItem));
}

void interfRemoveAES (CAdminExecutorService *aes)
{
	gtk_container_remove (GTK_CONTAINER(GTK_WIDGET(aes->RootTreeItem)->parent), GTK_WIDGET(aes->RootTreeItem));
}

void interfRemoveAS (CAdminService *as)
{
	gtk_container_remove (GTK_CONTAINER(GTK_WIDGET(as->RootTreeItem)->parent), GTK_WIDGET(as->RootTreeItem));
}

// windows delete event => quit
gint delete_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_main_quit();

	return FALSE;
}



vector<string> CommandHistory;
uint32 CommandHistoryPos = 0;

gint cbValidateCommand (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	string cmd = gtk_entry_get_text (GTK_ENTRY(widget));
	CommandHistory.push_back (cmd);
	// execute the command
	ICommand::execute (cmd, logstdout);
	// clear the input text
	gtk_entry_set_text (GTK_ENTRY(widget), "");
	CommandHistoryPos = CommandHistory.size();
	return TRUE;
}

void updateInput ()
{
	gtk_widget_grab_focus (InputText);
}

gint KeyIn(GtkWidget *Widget, GdkEventKey *Event, gpointer *Data)
{
	switch (Event->keyval)
	{
	case GDK_Escape : gtk_entry_set_text (GTK_ENTRY(Widget), ""); break;
	case GDK_Up : if (CommandHistoryPos > 0) { CommandHistoryPos--; gtk_entry_set_text (GTK_ENTRY(Widget), CommandHistory[CommandHistoryPos].c_str()); } break;
	case GDK_Down : if (CommandHistoryPos < CommandHistory.size() - 1) { CommandHistoryPos++; gtk_entry_set_text (GTK_ENTRY(Widget), CommandHistory[CommandHistoryPos].c_str()); } break;
	case GDK_KP_Enter : gtk_signal_emit_by_name(GTK_OBJECT(Widget),"activate"); return FALSE; break;
	default : return FALSE;
	}
	gtk_signal_emit_stop_by_name(GTK_OBJECT(Widget),"key_press_event");
	return TRUE;
}

gint KeyOut(GtkWidget *Widget, GdkEventKey *Event, gpointer *Data)
{
	updateInput();
	gtk_signal_emit_stop_by_name(GTK_OBJECT(Widget),"key_press_event");
	return TRUE;
}


/* Interface is something like this
	-------------------------
	|          |            |
	| RootTree | OutputText |
	|          |            |
	-------------------------
	| InputText             |
	-------------------------
*/

/*gint cbConnectToAS (GtkWidget *widget, gpointer data)
{
	nlassert (data != NULL);
	printf("connecting to %p\n", ASSelected);
	return TRUE;
}*/
/*void initToolbar (GtkWidget *container)
{
	GtkWidget *button;

	Toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_TEXT);
	gtk_container_set_border_width (GTK_CONTAINER (Toolbar), 5);
	gtk_toolbar_set_space_size (GTK_TOOLBAR (Toolbar), 5);
	gtk_container_add (GTK_CONTAINER (container), Toolbar);
	gtk_toolbar_set_space_style (GTK_TOOLBAR (Toolbar), GTK_TOOLBAR_SPACE_LINE);
	gtk_toolbar_set_button_relief (GTK_TOOLBAR (Toolbar), GTK_RELIEF_NONE);

	// connect button
	ConnectButton = gtk_toolbar_append_item (GTK_TOOLBAR (Toolbar), "Connect", "Closes this app", "Private", NULL, NULL, NULL);
    gtk_signal_connect (GTK_OBJECT (ConnectButton), "clicked", GTK_SIGNAL_FUNC (cbConnectToAS), NULL);
	gtk_widget_set_sensitive (ConnectButton, FALSE);
	gtk_widget_show (ConnectButton);

	gtk_toolbar_append_space (GTK_TOOLBAR (Toolbar));

	// ?? button
	button = gtk_toolbar_append_item (GTK_TOOLBAR (Toolbar), "???", "???", "Private", NULL, NULL, NULL);
	gtk_widget_show (button);

	gtk_widget_show (Toolbar);
}
*/


void initInterf ()
{
	gtk_init (NULL, NULL);

	RootWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (RootWindow), "Admin client for NeL Shard administration ("__DATE__" "__TIME__")");
	gtk_signal_connect (GTK_OBJECT (RootWindow), "delete_event", GTK_SIGNAL_FUNC (delete_event), NULL);

	GtkWidget *vrootbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (RootWindow), vrootbox);
	
//	initToolbar (vrootbox);


//	GtkWidget *hrootbox = gtk_hbox_new (FALSE, 0);
//	gtk_container_add (GTK_CONTAINER (vrootbox), hrootbox);

	GtkWidget *hrootbox = gtk_hpaned_new ();
	gtk_container_add (GTK_CONTAINER (vrootbox), hrootbox);


	// AS POPUP MENU
	{
		GtkItemFactory *item_factory;
		GtkAccelGroup *accel_group;
		gint nmenu_items = sizeof (ASMenuItems) / sizeof (ASMenuItems[0]);
		accel_group = gtk_accel_group_new ();
		item_factory = gtk_item_factory_new (GTK_TYPE_MENU, "<main>", accel_group);
		gtk_item_factory_create_items (item_factory, nmenu_items, ASMenuItems, NULL);
		gtk_window_add_accel_group (GTK_WINDOW (RootWindow), accel_group);
		ASMenu = gtk_item_factory_get_widget (item_factory, "<main>");
	}

	// S POPUP MENU (service menu)
/*	{
		GtkItemFactory *item_factory;
		GtkAccelGroup *accel_group;
		gint nmenu_items = sizeof (SMenuItems) / sizeof (SMenuItems[0]);
		accel_group = gtk_accel_group_new ();
		item_factory = gtk_item_factory_new (GTK_TYPE_MENU, "<main>", accel_group);
		gtk_item_factory_create_items (item_factory, nmenu_items, SMenuItems, NULL);
		gtk_window_add_accel_group (GTK_WINDOW (RootWindow), accel_group);
		SMenu = gtk_item_factory_get_widget (item_factory, "<main>");
	}
*/
/////// TREE
	GtkWidget *scrolled_win = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
//*	gtk_box_pack_start (GTK_BOX (hrootbox), scrolled_win, TRUE, TRUE, 0);
	gtk_paned_add1 (GTK_PANED (hrootbox), scrolled_win);

	gtk_widget_set_usize (scrolled_win, 300, 400);
	gtk_widget_show (scrolled_win);

	RootTree = gtk_tree_new();
//	gtk_signal_connect (GTK_OBJECT (root_tree), "selection_changed", (GtkSignalFunc)cb_tree_changed, (gpointer)NULL);
//	gtk_object_set_user_data (GTK_OBJECT (root_tree), tree_buttons);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_win), RootTree);
	gtk_tree_set_selection_mode (GTK_TREE (RootTree), GTK_SELECTION_SINGLE);
	gtk_tree_set_view_lines (GTK_TREE (RootTree), TRUE);
	gtk_tree_set_view_mode (GTK_TREE (RootTree), GTK_TREE_VIEW_LINE);
	gtk_widget_show (RootTree);
////////////

	void *btm, *lbl;
	createTreeItem (RootTree, (void *&)RootTreeItem, btm, lbl);
	setBitmap ("internet.xpm", btm);
	setLabel ("Internet", lbl);

////////

	// OUTPUT TEXT

	GtkWidget *scrolled_win2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win2), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
//*	gtk_box_pack_start (GTK_BOX (hrootbox), scrolled_win2, TRUE, TRUE, 0);
	gtk_paned_add2 (GTK_PANED (hrootbox), scrolled_win2);
	gtk_widget_set_usize (scrolled_win2, 600, 400);
	gtk_widget_show (scrolled_win2);

	OutputText = gtk_text_new (NULL, NULL);
	gtk_signal_connect(GTK_OBJECT(OutputText),"key_press_event",GTK_SIGNAL_FUNC(KeyOut),NULL);
	gtk_text_set_editable (GTK_TEXT (OutputText), FALSE);
	gtk_container_add (GTK_CONTAINER (scrolled_win2), OutputText);
//	gtk_widget_grab_focus (OutputText);
	gtk_widget_show (OutputText);

	GtkDisplayer = new CGtkDisplayer(getThreadId());
//	DebugLog->addDisplayer (GtkDisplayer);
	WarningLog->addDisplayer (GtkDisplayer);
	InfoLog->addDisplayer (GtkDisplayer);
	ErrorLog->addDisplayer (GtkDisplayer);

	// INPUT TEXT

	InputText = gtk_entry_new ();
//	gtk_entry_set_text (GTK_ENTRY (InputText), "hello world");
//	gtk_editable_select_region (GTK_EDITABLE (InputText), 0, 5);
	gtk_signal_connect (GTK_OBJECT(InputText), "activate", GTK_SIGNAL_FUNC(cbValidateCommand), NULL);
	gtk_signal_connect(GTK_OBJECT(InputText),"key_press_event",GTK_SIGNAL_FUNC(KeyIn),NULL);
	gtk_box_pack_start (GTK_BOX (vrootbox), InputText, TRUE, TRUE, 0);
	gtk_widget_show (InputText);








/*
	gtk_text_freeze (GTK_TEXT (OutputText));

	GdkFont *font = gdk_font_load ("-adobe-courier-medium-r-normal--*-120-*-*-*-*-*-*");
*/
	gtk_widget_show (hrootbox);
	gtk_widget_show (vrootbox);
    gtk_widget_show (RootWindow);
}

gint updateInterf (gpointer data)
{
//	g_print("ok\n");

//	AdminServices.push_back (CAdminService(NULL));
//	CAdminService *as = &(AdminServices.back());
//	interfAddAS(as);

	CNetManager::update();

	return TRUE;
}

static CStdDisplayer dispstdout;

void runInterf ()
{
	logstdout.addDisplayer (&dispstdout);
	logstdout.addDisplayer (GtkDisplayer);

	// todo virer ca pour pas que ca connecte automatiquement
	ICommand::execute ("connect 1", logstdout);
	
    gtk_timeout_add (500, updateInterf, NULL);
//	gdk_threads_enter ();
	gtk_main ();
//	gdk_threads_leave ();
	
//	DebugLog->removeDisplayer (GtkDisplayer);
	WarningLog->removeDisplayer (GtkDisplayer);
	InfoLog->removeDisplayer (GtkDisplayer);
	ErrorLog->removeDisplayer (GtkDisplayer);
}


// COMMANDS

NLMISC_COMMAND (quit, "quit", "")
{
	if(args.size() != 0) return false;

	gtk_main_quit ();
	
	return true;
}


#endif // INTERF_GTK

// todo reflechir a ce qu on propose sur les services a administrer (variables (once, autoupdate, fonction, etc..)
