
#define PURPLE_PLUGINS

#include <glib.h>
#include <gtk/gtk.h>

#include "gtkconv.h"
#include "gtkimhtml.h"
#include "gtkplugin.h"
#include "version.h"
#include "sound.h"
#include "prefs.h"

#define PREF_ROOT "/plugins/gtk/eionrobb-typewriter-sounds"
#define PREF_ENTER_SOUND	PREF_ROOT "/enter-sound"
#define PREF_KEY_SOUND		PREF_ROOT "/key-sound"
#define PREF_DELETE_SOUND	PREF_ROOT "/delete-sound"

static void
insert_text_cb(GtkTextBuffer *tb, GtkTextIter *pos, gchar *new_text, gint new_text_len, gpointer user_data)
{
	if (new_text_len == 1 && new_text[0] == '\n')
		purple_sound_play_file(purple_prefs_get_string(PREF_ENTER_SOUND), NULL);
	else
		purple_sound_play_file(purple_prefs_get_string(PREF_KEY_SOUND), NULL);
}

static void
delete_text_cb(GtkTextBuffer *tb, GtkTextIter *start_pos, GtkTextIter *end_pos, gpointer user_data)
{
	purple_sound_play_file(purple_prefs_get_string(PREF_DELETE_SOUND), NULL);
}

static void
detach_from_gtkconv(PidginConversation *gtkconv, gpointer user_data)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(gtkconv->entry_buffer), (GFunc)insert_text_cb, gtkconv);
	g_signal_handlers_disconnect_by_func(G_OBJECT(gtkconv->entry_buffer), (GFunc)delete_text_cb, gtkconv);
}

static void
attach_to_gtkconv(PidginConversation *gtkconv, gpointer user_data)
{
	g_signal_connect(G_OBJECT(gtkconv->entry_buffer), "insert_text", G_CALLBACK(insert_text_cb), gtkconv);
	g_signal_connect(G_OBJECT(gtkconv->entry_buffer), "delete_range", G_CALLBACK(delete_text_cb), gtkconv);
}

static void
detach_from_pidgin_window(PidginWindow *win, gpointer user_data)
{
	g_list_foreach(pidgin_conv_window_get_gtkconvs(win), (GFunc)detach_from_gtkconv, NULL);
}

static void
attach_to_pidgin_window(PidginWindow *win, gpointer user_data)
{
	g_list_foreach(pidgin_conv_window_get_gtkconvs(win), (GFunc)attach_to_gtkconv, NULL);
}

static void
detach_from_all_windows()
{
	g_list_foreach(pidgin_conv_windows_get_list(), (GFunc)detach_from_pidgin_window, NULL);
}

static void
attach_to_all_windows()
{
	g_list_foreach(pidgin_conv_windows_get_list(), (GFunc)attach_to_pidgin_window, NULL);
}

static void
conv_created_cb(PurpleConversation *conv, gpointer null)
{
	PidginConversation *gtkconv = PIDGIN_CONVERSATION(conv);

	g_return_if_fail(gtkconv != NULL);

	attach_to_gtkconv(gtkconv, NULL);
}

static void
conv_deleting_cb(PurpleConversation *conv, gpointer null)
{
	PidginConversation *gtkconv = PIDGIN_CONVERSATION(conv);

	g_return_if_fail(gtkconv != NULL);

	detach_from_gtkconv(gtkconv, NULL);
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
	attach_to_all_windows();

	purple_signal_connect(purple_conversations_get_handle(),
			"conversation-created",
			plugin, PURPLE_CALLBACK(conv_created_cb), NULL);
	purple_signal_connect(purple_conversations_get_handle(),
			"deleting-conversation",
			plugin, PURPLE_CALLBACK(conv_deleting_cb), NULL);

	return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
	detach_from_all_windows();

	return TRUE;
}

static PurplePluginPrefFrame *
plugin_config_frame(PurplePlugin *plugin)
{
	PurplePluginPrefFrame *frame;
	PurplePluginPref *pref;
	
	frame = purple_plugin_pref_frame_new();
	
	pref = purple_plugin_pref_new_with_name_and_label(PREF_ENTER_SOUND, "New line sound");
	purple_plugin_pref_frame_add(frame, pref);
	
	pref = purple_plugin_pref_new_with_name_and_label(PREF_KEY_SOUND, "Key sound");
	purple_plugin_pref_frame_add(frame, pref);
	
	pref = purple_plugin_pref_new_with_name_and_label(PREF_DELETE_SOUND, "Backspace sound");
	purple_plugin_pref_frame_add(frame, pref);
	
	return frame;
}

static void
init_plugin(PurplePlugin *plugin)
{
	purple_prefs_add_none(PREF_ROOT);
	purple_prefs_add_string(PREF_KEY_SOUND, "typewriter.wav");
	purple_prefs_add_string(PREF_ENTER_SOUND, "typewriter.wav");
	purple_prefs_add_string(PREF_DELETE_SOUND, "typewriter.wav");
}

static PurplePluginUiInfo prefs_info = {
	plugin_config_frame,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	2,
	2,
	PURPLE_PLUGIN_STANDARD,
	PIDGIN_PLUGIN_TYPE,
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,
	"eionrobb-typewriter-sound",
	"Typing Sounds",
	"0.1",
	"Make (typewriter) sounds when you're typing",
	"",
	"Eion Robb <eionrobb@gmail.com>",
	"http://pidgin-typing-sounds.googlecode.com/",
	plugin_load,
	plugin_unload,
	NULL,
	NULL,
	NULL,
	&prefs_info,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

PURPLE_INIT_PLUGIN("pidgin-typing-sounds", init_plugin, info);
