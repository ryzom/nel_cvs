/** \file interf_dos.cpp
 * 
 *
 * $Id: interf_gtk.cpp,v 1.1 2001/05/18 16:51:49 lecroart Exp $
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

#include "nel/misc/debug.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"

#include "nel/net/net_manager.h"

#include "datas.h"
#include "connection_as.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

#ifdef NL_OS_WINDOWS
// automatically add gtk library
#pragma comment(lib, "gtk-1.3.lib")
#pragma comment(lib, "gdk-1.3.lib")
#pragma comment(lib, "glib-1.3.lib")
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

//
// Functions
//

void cbStartService ()
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
		GtkItemFactoryEntry toto;
		toto.path = const_cast<char *>(name.c_str());
		toto.accelerator = NULL;
		toto.callback = cbStartService;
		toto.callback_action = 0;
		toto.item_type = NULL;
		gtk_item_factory_create_items (GTK_ITEM_FACTORY(aes->ItemFactory), 1, &toto, &(aes->ServiceAliasList[i]));
	}
}

void setBitmap (const string &bitmapName, void *&bitmap)
{
	GtkStyle *style = gtk_widget_get_style (RootWindow);
	GdkBitmap *mask;
	GdkBitmap *pixmap = gdk_pixmap_create_from_xpm (RootWindow->window, &mask, &style->bg[GTK_STATE_NORMAL], bitmapName.c_str());

	if (bitmap == NULL)
		bitmap = gtk_pixmap_new (pixmap, mask);
	else
		gtk_pixmap_set (GTK_PIXMAP (bitmap), pixmap, mask);
}

void setLabel (const string &text, void *&label)
{
	gtk_label_set_text (GTK_LABEL (label), text.c_str());
}

void createTreeItem (const string &bitmapName, const string &text, GtkWidget *rootTree, void *&treeItem, void *&bitmap, void *&label)
{
	nlassert (treeItem == NULL);

	treeItem = gtk_tree_item_new ();

    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
/*	GtkStyle *style = gtk_widget_get_style (RootWindow);
	GdkBitmap *mask;
	GdkBitmap *pixmap = gdk_pixmap_create_from_xpm (RootWindow->window, &mask, &style->bg[GTK_STATE_NORMAL], bitmapName.c_str());

	bitmap = gtk_pixmap_new (pixmap, mask);
*/

	bitmap = NULL;
	setBitmap (bitmapName, bitmap);

	gtk_widget_show (GTK_WIDGET(bitmap));
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(bitmap), FALSE, FALSE, 0);

	label = gtk_label_new (text.c_str());
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
/*	GtkItemFactoryEntry toto;
	toto.path = "/Services/oho";
	toto.accelerator = NULL;
	toto.callback = NULL;
	toto.callback_action = 0;
	toto.item_type = NULL;

	gtk_item_factory_create_items (item_factory, 1, &toto, NULL);
*/
	if (PopupAS->Connected)
	{
		nlwarning("already connected!!!");
		return;
	}

	printf("connecting to...%p\n", PopupAS);

//	setBitmap ("as_connecting.xpm", PopupAS->Bitmap);

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

	setBitmap ("as_connecting.xpm", PopupAS->Bitmap);

	connectionASRelease (PopupAS);

	PopupAS = NULL;
}

static GtkItemFactoryEntry ASMenuItems[] = {
	{ "/Connect", NULL, cbConnectToAS, 0, NULL },
	{ "/Disconnect", NULL, cbDisconnectToAS, 0, NULL },
	{ "/Services", NULL, NULL, 0, "<Branch>" },
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////// POPUP MENU FOR AES ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

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

/*
static void cbStartService()
{
	printf("starting service ...%p\n", PopupAES);
	PopupAES = NULL;
}
*/
static GtkItemFactoryEntry AESMenuItems[] = {
	{ "/Coucou", NULL, NULL, 0, NULL }
};

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
	string name;
	name = "AS";
	name += toString(as->Id);
	name += " '";
	name += as->ASName;
	name += "' (";
	name += as->ASAddr;
	name += ")";
	createTreeItem ("as_not_connected.xpm", name, RootSubTree, as->RootTreeItem, as->Bitmap, as->Label);

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
		icon = "as_connected.xpm";
	else
		icon = "as_not_connected.xpm";

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
		icon = "as_connected.xpm";
	else
		icon = "as_not_connected.xpm";

	createTreeItem (icon, name, GTK_WIDGET(as->RootSubTree), aes->RootTreeItem, aes->Bitmap, aes->Label);

	// AES POPUP MENU
	GtkAccelGroup *accel_group;
	gint nmenu_items = sizeof (AESMenuItems) / sizeof (AESMenuItems[0]);
	accel_group = gtk_accel_group_new ();
	aes->ItemFactory = gtk_item_factory_new (GTK_TYPE_MENU, "<main>", accel_group);
	gtk_item_factory_create_items (GTK_ITEM_FACTORY(aes->ItemFactory), nmenu_items, AESMenuItems, NULL);
	gtk_window_add_accel_group (GTK_WINDOW (RootWindow), accel_group);
	GtkWidget *menu = gtk_item_factory_get_widget (GTK_ITEM_FACTORY(aes->ItemFactory), "<main>");

	gtk_signal_connect (GTK_OBJECT (aes->RootTreeItem), "button-press-event", GTK_SIGNAL_FUNC(cbPopupAESMenu), menu);

	if (as->AdminExecutorServices.size () == 1)
	{
		// it's the first AS, we have to add the subtree
		gtk_tree_item_set_subtree (GTK_TREE_ITEM (as->RootTreeItem), (GtkWidget*)as->RootSubTree);
		gtk_tree_item_expand (GTK_TREE_ITEM (as->RootTreeItem));
	}
}

void interfUpdateS (CService *s)
{
	string name;
	name = "S";
	name += toString(s->Id);
	name += " '";
	name += s->ServiceAlias;
	name += "' '";
	name += s->ShortName;
	name += "' '";
	name += s->LongName;
	name += "'";

	string icon;
	if (s->Connected)
		icon = "as_connected.xpm";
	else
		icon = "as_not_connected.xpm";

	// check if we already create widgets
	nlassert (s->RootTreeItem != NULL);

	setBitmap (icon, s->Bitmap);
	setLabel (name, s->Label);
}

void interfAddS (CAdminExecutorService *aes, CService *s)
{
	if (aes->Services.size () == 1)
	{
		// it's the first AS, we have to add the subtree
		aes->RootSubTree = gtk_tree_new();
	}

	string name;
	name = "S";
	name += toString(s->Id);
	name += " '";
	name += s->ServiceAlias;
	name += "' '";
	name += s->ShortName;
	name += "' '";
	name += s->LongName;
	name += "'";

	string icon;
	if (aes->Connected)
		icon = "as_connected.xpm";
	else
		icon = "as_not_connected.xpm";

	// create the item
	createTreeItem (icon, name, GTK_WIDGET(aes->RootSubTree), s->RootTreeItem, s->Bitmap, s->Label);

	if (aes->Services.size () == 1)
	{
		// it's the first AS, we have to add the subtree
		gtk_tree_item_set_subtree (GTK_TREE_ITEM (aes->RootTreeItem), (GtkWidget*)aes->RootSubTree);
		gtk_tree_item_expand (GTK_TREE_ITEM (aes->RootTreeItem));
	}
}

void removeSubTree (CAdminService *as)
{
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
gint delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) { gtk_main_quit(); return FALSE; }

/*gint cbConnectToAS (GtkWidget *widget, gpointer data)
{
	nlassert (data != NULL);
	printf("connecting to %p\n", ASSelected);
	return TRUE;
}*/



/* Interface is something like this
	-----------
	| toolbar |
	-----------
	|    |    |
	|tree|    |
	-----------
*/
/*
void initToolbar (GtkWidget *container)
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


    GtkWidget *hrootbox = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (vrootbox), hrootbox);


	GtkWidget *scrolled_win = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (hrootbox), scrolled_win, TRUE, TRUE, 0);
	gtk_widget_set_usize (scrolled_win, 400, 400);
	gtk_widget_show (scrolled_win);

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

///////////////

/////// TREE

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
	createTreeItem ("world.xpm", "Internet", RootTree, (void *&)RootTreeItem, btm, lbl);

	/*
	GtkWidget *item_subtree = gtk_tree_new();
	GtkWidget *root_item2 = gtk_tree_item_new_with_label("toto");
	gtk_tree_append (GTK_TREE (item_subtree), root_item2);
    gtk_tree_item_set_subtree (GTK_TREE_ITEM (root_item), item_subtree);
	gtk_widget_show (root_item2);
    */

////////

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

static CLog logstdout;
static CStdDisplayer dispstdout;

void runInterf ()
{
	logstdout.addDisplayer (&dispstdout);
	// todo virer ca pour pas que ca connecte automatiquement
	ICommand::execute ("connect 1", logstdout);
	
    gtk_timeout_add (500, updateInterf, NULL);
    gtk_main ();
}




#endif // INTERF_GTK
