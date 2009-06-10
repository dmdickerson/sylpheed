/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2009 Hiroyuki Yamamoto
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include "plugin.h"
#include "utils.h"
#include "folder.h"

G_DEFINE_TYPE(SylPlugin, syl_plugin, G_TYPE_OBJECT);

enum {
	PLUGIN_LOAD,
	PLUGIN_UNLOAD,
	FOLDERVIEW_MENU_POPUP,
	LAST_SIGNAL
};

#define syl_plugin_lookup_symbol(name)	g_hash_table_lookup(sym_table, name)

#define SAFE_CALL(func_ptr)		{ if (func_ptr) func_ptr(); }
#define SAFE_CALL_RET(func_ptr)		(func_ptr ? func_ptr() : NULL)
#define SAFE_CALL_ARG1(func_ptr, arg1)	{ if (func_ptr) func_ptr(arg1); }
#define SAFE_CALL_ARG1_RET(func_ptr, arg1) \
				(func_ptr ? func_ptr(arg1) : NULL)
#define SAFE_CALL_ARG2(func_ptr, arg1, arg2) \
				{ if (func_ptr) func_ptr(arg1, arg2); }
#define SAFE_CALL_ARG2_RET(func_ptr, arg1, arg2) \
				(func_ptr ? func_ptr(arg1, arg2) : NULL)
#define SAFE_CALL_ARG2_RET_VAL(func_ptr, arg1, arg2, retval) \
				(func_ptr ? func_ptr(arg1, arg2) : retval)
#define SAFE_CALL_ARG3(func_ptr, arg1, arg2, arg3) \
				{ if (func_ptr) func_ptr(arg1, arg2, arg3); }
#define SAFE_CALL_ARG3_RET(func_ptr, arg1, arg2, arg3) \
				(func_ptr ? func_ptr(arg1, arg2, arg3) : NULL)
#define SAFE_CALL_ARG4(func_ptr, arg1, arg2, arg3, arg4) \
				{ if (func_ptr) func_ptr(arg1, arg2, arg3); }
#define SAFE_CALL_ARG4_RET(func_ptr, arg1, arg2, arg3, arg4) \
				(func_ptr ? func_ptr(arg1, arg2, arg3, arg4) : NULL)

static guint plugin_signals[LAST_SIGNAL] = { 0 };

static GHashTable *sym_table = NULL;
static GSList *module_list = NULL;
static GObject *plugin_obj = NULL;

static void syl_plugin_init(SylPlugin *self)
{
}

static void syl_plugin_class_init(SylPluginClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	plugin_signals[PLUGIN_LOAD] =
		g_signal_new("plugin-load",
			     G_TYPE_FROM_CLASS(gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET(SylPluginClass, plugin_load),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__POINTER,
			     G_TYPE_NONE,
			     1,
			     G_TYPE_POINTER);
	plugin_signals[PLUGIN_UNLOAD] =
		g_signal_new("plugin-unload",
			     G_TYPE_FROM_CLASS(gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET(SylPluginClass, plugin_unload),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__POINTER,
			     G_TYPE_NONE,
			     1,
			     G_TYPE_POINTER);
	plugin_signals[FOLDERVIEW_MENU_POPUP] =
		g_signal_new("folderview-menu-popup",
			     G_TYPE_FROM_CLASS(gobject_class),
			     G_SIGNAL_RUN_FIRST,
			     G_STRUCT_OFFSET(SylPluginClass,
					     folderview_menu_popup),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__POINTER,
			     G_TYPE_NONE,
			     1,
			     G_TYPE_POINTER);
}

void syl_plugin_signal_connect(const gchar *name, GCallback callback,
			       gpointer data)
{
	g_signal_connect(plugin_obj, name, callback, data);
}

void syl_plugin_signal_disconnect(gpointer func, gpointer data)
{
	g_signal_handlers_disconnect_by_func(plugin_obj, func, data);
}

void syl_plugin_signal_emit(const gchar *name, ...)
{
	guint signal_id;

	if (g_signal_parse_name(name, G_TYPE_FROM_INSTANCE(plugin_obj), &signal_id, NULL, FALSE)) {
		va_list var_args;
		va_start(var_args, name);
		g_signal_emit_valist(plugin_obj, signal_id, 0, var_args);
		va_end(var_args);
	} else
		g_warning("%s: signal '%s' not found", G_STRLOC, name);
}

gint syl_plugin_init_lib(void)
{
	if (!g_module_supported()) {
		g_warning("Plug-in is not supported.");
		return -1;
	}

	if (!sym_table) {
		sym_table = g_hash_table_new(g_str_hash, g_str_equal);
	}

	if (!plugin_obj) {
		plugin_obj = g_object_new(SYL_TYPE_PLUGIN, NULL);
	}

	return 0;
}

gint syl_plugin_load(const gchar *name)
{
	GModule *module;
	SylPluginLoadFunc load_func = NULL;
	gchar *file;

	g_return_val_if_fail(name != NULL, -1);

	debug_print("syl_plugin_load: loading %s\n", name);

	if (!g_path_is_absolute(name))
		file = g_strconcat(PLUGINDIR, G_DIR_SEPARATOR_S, name, NULL);
	else
		file = g_strdup(name);

	module = g_module_open(file, G_MODULE_BIND_LAZY);
	if (!module) {
		g_warning("Cannot open module: %s: %s", name, g_module_error());
		g_free(file);
		return -1;
	}
	if (g_slist_find(module_list, module)) {
		g_warning("Module %s is already loaded", name);
		g_free(file);
		return -1;
	}

	if (g_module_symbol(module, "plugin_load", (gpointer *)&load_func)) {
		if (!syl_plugin_check_version(module)) {
#if 0
			g_warning("Version check failed. Skipping: %s", name);
			g_module_close(module);
			g_free(file);
			return -1;
#endif
		}

		debug_print("calling plugin_load() in %s\n",
			    g_module_name(module));
		load_func();
		module_list = g_slist_prepend(module_list, module);
		g_signal_emit(plugin_obj, plugin_signals[PLUGIN_LOAD], 0, module);
	} else {
		g_warning("Cannot get symbol: %s: %s", name, g_module_error());
		g_module_close(module);
		g_free(file);
		return -1;
	}

	g_free(file);

	return 0;
}

gint syl_plugin_load_all(const gchar *dir)
{
	GDir *d;
	const gchar *dir_name;
	gchar *path;
	gint count = 0;

	if ((d = g_dir_open(dir, 0, NULL)) == NULL) {
		g_warning("failed to open directory: %s", dir);
		return -1;
	}

	while ((dir_name = g_dir_read_name(d)) != NULL) {
		if (!g_str_has_suffix(dir_name, "." G_MODULE_SUFFIX))
			continue;
		path = g_strconcat(dir, G_DIR_SEPARATOR_S, dir_name, NULL);
		if (syl_plugin_load(path) == 0)
			count++;
		g_free(path);
	}

	g_dir_close(d);

	return count;
}

void syl_plugin_unload_all(void)
{
	GSList *cur;

	for (cur = module_list; cur != NULL; cur = cur->next) {
		GModule *module = (GModule *)cur->data;
		SylPluginUnloadFunc unload_func = NULL;

		if (g_module_symbol(module, "plugin_unload",
				    (gpointer *)&unload_func)) {
			g_signal_emit(plugin_obj, plugin_signals[PLUGIN_UNLOAD],
				      0, module);
			debug_print("calling plugin_unload() in %s\n",
				    g_module_name(module));
			unload_func();
		} else {
			g_warning("Cannot get symbol: %s", g_module_error());
		}
		if (!g_module_close(module)) {
			g_warning("Module unload failed: %s", g_module_error());
		}
	}

	g_slist_free(module_list);
	module_list = NULL;
}

GSList *syl_plugin_get_module_list(void)
{
	return module_list;
}

SylPluginInfo *syl_plugin_get_info(GModule *module)
{
	SylPluginInfo * (*plugin_info_func)(void);

	g_return_val_if_fail(module != NULL, NULL);

	debug_print("getting plugin information of %s\n",
		    g_module_name(module));

	if (g_module_symbol(module, "plugin_info",
			    (gpointer *)&plugin_info_func)) {
		debug_print("calling plugin_info() in %s\n",
			    g_module_name(module));
		return plugin_info_func();
	} else {
		g_warning("Cannot get symbol: %s: %s", g_module_name(module),
			  g_module_error());
		return NULL;
	}
}

gboolean syl_plugin_check_version(GModule *module)
{
	gint (*version_func)(void);
	gint ver;

	g_return_val_if_fail(module != NULL, FALSE);

	if (g_module_symbol(module, "plugin_interface_version",
			    (gpointer *)&version_func)) {
		debug_print("calling plugin_interface_version() in %s\n",
			    g_module_name(module));
		ver = version_func();
	} else {
		g_warning("Cannot get symbol: %s: %s", g_module_name(module),
			  g_module_error());
		return FALSE;
	}

	if (ver == SYL_PLUGIN_INTERFACE_VERSION) {
		debug_print("Version OK: plugin: %d, app: %d\n",
			    ver, SYL_PLUGIN_INTERFACE_VERSION);
		return TRUE;
	} else {
		g_warning("Plugin interface version mismatch: plugin: %d, app: %d", ver, SYL_PLUGIN_INTERFACE_VERSION);
		return FALSE;
	}
}

gint syl_plugin_add_symbol(const gchar *name, gpointer sym)
{
	g_hash_table_insert(sym_table, (gpointer)name, sym);
	return 0;
}

const gchar *syl_plugin_get_prog_version(void)
{
	gpointer sym;

	sym = syl_plugin_lookup_symbol("prog_version");
	return (gchar *)sym;
}

gpointer syl_plugin_main_window_get(void)
{
	gpointer (*func)(void);

	func = syl_plugin_lookup_symbol("main_window_get");
	return SAFE_CALL_RET(func);
}

void syl_plugin_main_window_popup(gpointer mainwin)
{
	void (*func)(gpointer);

	func = syl_plugin_lookup_symbol("main_window_popup");
	SAFE_CALL_ARG1(func, mainwin);
}

void syl_plugin_app_will_exit(gboolean force)
{
	void (*func)(gboolean);

	func = syl_plugin_lookup_symbol("app_will_exit");
	SAFE_CALL_ARG1(func, force);
}

static GtkItemFactory *get_item_factory(const gchar *path)
{
	GtkItemFactory *ifactory;

	if (!path)
		return NULL;

	if (strncmp(path, "<Main>", 6) == 0)
		ifactory = syl_plugin_lookup_symbol("main_window_menu_factory");
	else if (strncmp(path, "<MailFolder>", 12) == 0)
		ifactory = syl_plugin_lookup_symbol("folderview_mail_popup_factory");
	else
		ifactory = syl_plugin_lookup_symbol("main_window_menu_factory");

	return ifactory;
}

gint syl_plugin_add_menuitem(const gchar *parent, const gchar *label,
			     SylPluginCallbackFunc func, gpointer data)
{
	GtkItemFactory *ifactory;
	GtkWidget *menu;
	GtkWidget *menuitem;

	if (!parent)
		return -1;

	ifactory = get_item_factory(parent);
	if (!ifactory)
		return -1;

	menu = gtk_item_factory_get_widget(ifactory, parent);
	if (!menu)
		return -1;

	if (label)
		menuitem = gtk_menu_item_new_with_label(label);
	else {
		menuitem = gtk_menu_item_new();
		gtk_widget_set_sensitive(menuitem, FALSE);
	}
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	if (func)
		g_signal_connect(G_OBJECT(menuitem), "activate",
				 G_CALLBACK(func), data);

	return 0;
}

gint syl_plugin_add_factory_item(const gchar *parent, const gchar *label,
				 SylPluginCallbackFunc func, gpointer data)
{
	GtkItemFactory *ifactory;
	GtkItemFactoryEntry entry = {NULL, NULL, NULL, 0, NULL};

	if (!parent)
		return -1;

	ifactory = get_item_factory(parent);
	if (!ifactory)
		return -1;

	if (label) {
		entry.path = (gchar *)label;
		if (g_str_has_suffix(label, "/---"))
			entry.item_type = "<Separator>";
		else
			entry.item_type = NULL;
	} else {
		entry.path = "/---";
		entry.item_type = "<Separator>";
	}
	entry.callback = func;
	g_print("entry.path = %s\n", entry.path);

	gtk_item_factory_create_item(ifactory, &entry, data, 2);

	return 0;
}

void syl_plugin_menu_set_sensitive(const gchar *path, gboolean sensitive)
{
	GtkItemFactory *ifactory;
	GtkWidget *widget;

	g_return_if_fail(path != NULL);

	ifactory = get_item_factory(path);
	if (!ifactory)
		return;

	widget = gtk_item_factory_get_item(ifactory, path);
	gtk_widget_set_sensitive(widget, sensitive);
}

void syl_plugin_menu_set_sensitive_all(GtkMenuShell *menu_shell,
				       gboolean sensitive)
{
	GList *cur;

	for (cur = menu_shell->children; cur != NULL; cur = cur->next)
		gtk_widget_set_sensitive(GTK_WIDGET(cur->data), sensitive);
}

void syl_plugin_menu_set_active(const gchar *path, gboolean is_active)
{
	GtkItemFactory *ifactory;
	GtkWidget *widget;

	g_return_if_fail(path != NULL);

	ifactory = get_item_factory(path);
	if (!ifactory)
		return;

	widget = gtk_item_factory_get_item(ifactory, path);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), is_active);
}

gpointer syl_plugin_folderview_get(void)
{
	gpointer sym;

	sym = syl_plugin_lookup_symbol("folderview");
	return sym;
}

FolderItem *syl_plugin_folderview_get_selected_item(void)
{
	FolderItem * (*func)(gpointer);
	gpointer folderview;

	folderview = syl_plugin_folderview_get();
	if (folderview) {
		func = syl_plugin_lookup_symbol("folderview_get_selected_item");
		return SAFE_CALL_ARG1_RET(func, folderview);
	}

	return NULL;
}

gpointer syl_plugin_summary_view_get(void)
{
	gpointer sym;

	sym = syl_plugin_lookup_symbol("summaryview");
	return sym;
}

void syl_plugin_sumary_select_by_msgnum(guint msgnum)
{
	void (*func)(gpointer, guint);
	gpointer summary;

	summary = syl_plugin_summary_view_get();
	if (summary) {
		func = syl_plugin_lookup_symbol("summary_select_by_msgnum");
		SAFE_CALL_ARG2(func, summary, msgnum);
	}
}

gboolean syl_plugin_summary_select_by_msginfo(MsgInfo *msginfo)
{
	gboolean (*func)(gpointer, MsgInfo *);
	gpointer summary;

	summary = syl_plugin_summary_view_get();
	if (summary) {
		func = syl_plugin_lookup_symbol("summary_select_by_msginfo");
		return SAFE_CALL_ARG2_RET_VAL(func, summary, msginfo, FALSE);
	}

	return FALSE;
}

void syl_plugin_open_message(const gchar *folder_id, guint msgnum)
{
	FolderItem *item;
	MsgInfo *msginfo;

	item = folder_find_item_from_identifier(folder_id);
	msginfo = folder_item_get_msginfo(item, msgnum);

	if (msginfo) {
		if (!syl_plugin_summary_select_by_msginfo(msginfo)) {
			syl_plugin_open_message_by_new_window(msginfo);
		}
		procmsg_msginfo_free(msginfo);
	}
}

gpointer syl_plugin_messageview_create_with_new_window(void)
{
	gpointer (*func)(void);

	func = syl_plugin_lookup_symbol("messageview_create_with_new_window");
	return SAFE_CALL_RET(func);
}

void syl_plugin_open_message_by_new_window(MsgInfo *msginfo)
{
	gpointer msgview;
	gpointer (*func)(gpointer, MsgInfo *, gboolean);

	msgview = syl_plugin_messageview_create_with_new_window();
	if (msgview) {
		func = syl_plugin_lookup_symbol("messageview_show");
		SAFE_CALL_ARG3(func, msgview, msginfo, FALSE);
	}
}

FolderItem *syl_plugin_folder_sel(Folder *cur_folder, gint sel_type,
				  const gchar *default_folder)
{
	FolderItem * (*func)(Folder *, gint, const gchar *);

	func = syl_plugin_lookup_symbol("foldersel_folder_sel");
	return SAFE_CALL_ARG3_RET(func, cur_folder, sel_type, default_folder);
}

FolderItem *syl_plugin_folder_sel_full(Folder *cur_folder, gint sel_type,
				       const gchar *default_folder,
				       const gchar *message)
{
	FolderItem * (*func)(Folder *, gint, const gchar *, const gchar *);

	func = syl_plugin_lookup_symbol("foldersel_folder_sel_full");
	return SAFE_CALL_ARG4_RET(func, cur_folder, sel_type, default_folder,
				  message);
}

gchar *syl_plugin_input_dialog(const gchar *title, const gchar *message,
			       const gchar *default_string)
{
	gchar * (*func)(const gchar *, const gchar *, const gchar *);

	func = syl_plugin_lookup_symbol("input_dialog");
	return SAFE_CALL_ARG3_RET(func, title, message, default_string);
}

gchar *syl_plugin_input_dialog_with_invisible(const gchar *title,
					      const gchar *message,
					      const gchar *default_string)
{
	gchar * (*func)(const gchar *, const gchar *, const gchar *);

	func = syl_plugin_lookup_symbol("input_dialog_with_invisible");
	return SAFE_CALL_ARG3_RET(func, title, message, default_string);
}

void syl_plugin_manage_window_set_transient(GtkWindow *window)
{
	void (*func)(GtkWindow *);

	func = syl_plugin_lookup_symbol("manage_window_set_transient");
	SAFE_CALL_ARG1(func, window);
}

void syl_plugin_manage_window_signals_connect(GtkWindow *window)
{
	void (*func)(GtkWindow *);

	func = syl_plugin_lookup_symbol("manage_window_signals_connect");
	SAFE_CALL_ARG1(func, window);
}

GtkWidget *syl_plugin_manage_window_get_focus_window(void)
{
	GtkWidget * (*func)(void);

	func = syl_plugin_lookup_symbol("manage_window_get_focus_window");
	return SAFE_CALL_RET(func);
}

void syl_plugin_inc_mail(void)
{
	void (*func)(gpointer);

	func = syl_plugin_lookup_symbol("inc_mail");
	SAFE_CALL_ARG1(func, syl_plugin_main_window_get());
}

void syl_plugin_inc_lock(void)
{
	void (*func)(void);

	func = syl_plugin_lookup_symbol("inc_lock");
	SAFE_CALL(func);
}

void syl_plugin_inc_unlock(void)
{
	void (*func)(void);

	func = syl_plugin_lookup_symbol("inc_unlock");
	SAFE_CALL(func);
}