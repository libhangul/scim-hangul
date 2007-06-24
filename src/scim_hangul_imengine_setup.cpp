/** @file scim_hangul_imengine_setup.cpp
 * implementation of Setup Module of hangul imengine module.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (C) 2004-2006 Choe Hwanjin
 * Copyright (c) 2004-2006 James Su <suzhe@turbolinux.com.cn>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * $Id: scim_hangul_imengine_setup.cpp,v 1.8 2006/10/23 12:42:47 hwanjin Exp $
 *
 */

#define Uses_SCIM_CONFIG_BASE

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include <gtk/scimkeyselection.h>

#ifdef HAVE_GETTEXT
  #include <libintl.h>
  #define _(String) dgettext(GETTEXT_PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

using namespace scim;

#define scim_module_init hangul_imengine_setup_LTX_scim_module_init
#define scim_module_exit hangul_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       hangul_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    hangul_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        hangul_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description hangul_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     hangul_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     hangul_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   hangul_imengine_setup_LTX_scim_setup_module_query_changed


#define SCIM_CONFIG_IMENGINE_HANGUL_USE_DVORAK                  "/IMEngine/Hangul/UseDvorak"
#define SCIM_CONFIG_IMENGINE_HANGUL_SHOW_CANDIDATE_COMMENT      "/IMEngine/Hangul/ShowCandidateComment"
#define SCIM_CONFIG_IMENGINE_HANGUL_HANGUL_HANJA_KEY            "/IMEngine/Hangul/HangulHanjaKey"

#define SCIM_CONFIG_SHOW_CANDIDATE_COMMENT "/IMEngine/Hangul/ShowCandidateComment"
#define SCIM_CONFIG_LAYOUT                 "/IMEngine/Hangul/KeyboardLayout"
#define SCIM_CONFIG_HANGUL_KEY             "/IMEngine/Hangul/HangulKey"
#define SCIM_CONFIG_HANJA_KEY              "/IMEngine/Hangul/HanjaKey"
#define SCIM_CONFIG_HANJA_MODE_KEY         "/IMEngine/Hangul/HanjaModeKey"
#define SCIM_CONFIG_USE_ASCII_MODE         "/IMEngine/Hangul/UseAsciiMode"
#define SCIM_CONFIG_COMMIT_BY_WORD         "/IMEngine/Hangul/CommitByWord"


static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_HANGUL_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        static GtkWidget *setup_ui = NULL;
        if (setup_ui == NULL)
            setup_ui = create_setup_window ();
        return setup_ui;
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Hangul"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("A Hangul IMEngine Module."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"

// Internal data structure
struct KeyBindingData
{
    const char *key;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *entry;
    GtkWidget  *button;
    String      data;
};

static GtkWidget *keyboard_layout_combo = NULL;
static GtkWidget *use_ascii_mode_button = NULL;
static GtkWidget *commit_by_word_button = NULL;
static GtkWidget *show_candidate_comment_button = NULL;

static bool __have_changed                 = false;

static KeyBindingData key_bindings[] =
{
    {
        // key
        SCIM_CONFIG_HANGUL_KEY,
        // label
        N_("Hangul keys:"),
        // title
        N_("Select Hangul keys"),
        // tooltip
        N_("The key events to change input mode between hangul and ascii."
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Hangul,"
        "Shift+space"
    },
    {
        // key
        SCIM_CONFIG_HANJA_KEY,
        // label
        N_("Hangul to Hanja keys:"),
        // title
        N_("Select Hangul to Hanja keys"),
        // tooltip
        N_("The key events to convert Hangul to Hanja character. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Hangul_Hanja,"
        "F9"
    },
    {
        // key
        SCIM_CONFIG_HANJA_MODE_KEY,
        // label
        N_("Hanja mode keys:"),
        // title
        N_("Select a key to toggle hanja mode keys"),
        // tooltip
        N_("The key events to toggle Hanja mode. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        ""
    }
};

// Declaration of internal functions.
static void
on_default_editable_changed          (GtkEditable     *editable,
                                      gpointer         user_data);

static void
on_default_toggle_button_toggled     (GtkToggleButton *togglebutton,
                                      gpointer         user_data);

static void
on_default_key_selection_clicked     (GtkButton       *button,
                                      gpointer         user_data);

static void
on_default_combo_box_changed         (GtkComboBox *combobox,
                                      gpointer     user_data);

static GtkWidget *
create_options_page(GtkTooltips *tooltip);

static GtkWidget *
create_keyboard_page(GtkTooltips *tooltip);

// Function implementations.
static GtkWidget *
create_options_page(GtkTooltips *tooltips)
{
    GtkWidget *vbox;
    GtkWidget *button;

    vbox = gtk_vbox_new (FALSE, 12);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);

    button = gtk_check_button_new_with_mnemonic (_("_Use ascii input mode"));
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    gtk_tooltips_set_tip(tooltips, button,
                          _("Whether to enable to change the input mode "
                            "between hangul and ascii mode."), NULL);
    g_signal_connect(G_OBJECT(button), "toggled",
                     G_CALLBACK(on_default_toggle_button_toggled), NULL);
    use_ascii_mode_button = button;

    button = gtk_check_button_new_with_mnemonic (_("_Show candidate comment"));
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    gtk_tooltips_set_tip(tooltips, button,
                          _("Whether to show the comment of candidates or not."), NULL);
    g_signal_connect(G_OBJECT(button), "toggled",
                     G_CALLBACK(on_default_toggle_button_toggled), NULL);
    show_candidate_comment_button = button;

    button = gtk_check_button_new_with_mnemonic (_("_Commit by word"));
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    gtk_tooltips_set_tip(tooltips, button,
                          _("Whether not to commit until any non-hangul character is inputed."), NULL);
    g_signal_connect(G_OBJECT(button), "toggled",
                     G_CALLBACK(on_default_toggle_button_toggled), NULL);
    commit_by_word_button = button;

    return vbox;
}

static GtkWidget *
create_keyboard_page(GtkTooltips *tooltips)
{
    unsigned int i;

    GtkWidget *vbox1 = gtk_vbox_new(FALSE, 12);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), 12);

    GtkWidget *label = gtk_label_new("");
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_label_set_markup(GTK_LABEL(label),
            _("<span weight=\"bold\">Keyboard layout</span>"));
    gtk_box_pack_start(GTK_BOX(vbox1), label, FALSE, TRUE, 0);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, TRUE, 0);

    label = gtk_label_new("    ");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);

    GtkWidget *vbox2 = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, TRUE, 0);

    GtkWidget *combo_box = gtk_combo_box_new_text();
    gtk_box_pack_start(GTK_BOX(vbox2), combo_box, FALSE, TRUE, 0);
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("2bul"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("3bul 2bul-shifted"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("3bul Final"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("3bul 390"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("3bul No-Shift"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("3bul Yetgeul"));
    g_signal_connect(G_OBJECT(combo_box), "changed",
                     G_CALLBACK (on_default_combo_box_changed), NULL);
    keyboard_layout_combo = combo_box;


    label = gtk_label_new("");
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_label_set_markup(GTK_LABEL(label),
            _("<span weight=\"bold\">Key bindings</span>"));
    gtk_box_pack_start(GTK_BOX(vbox1), label, FALSE, TRUE, 0);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, TRUE, 0);

    label = gtk_label_new("    ");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);

    vbox2 = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);

    GtkWidget *table = gtk_table_new (3, G_N_ELEMENTS(key_bindings), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox2), table, FALSE, TRUE, 0);

    for (i = 0; i < G_N_ELEMENTS(key_bindings); ++ i) {
        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _(key_bindings[i].label));
        gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);

        key_bindings[i].entry = gtk_entry_new ();
        gtk_table_attach (GTK_TABLE (table), key_bindings[i].entry, 1, 2, i, i+1,
                          (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
        gtk_entry_set_editable (GTK_ENTRY (key_bindings[i].entry), FALSE);
        gtk_entry_set_text (GTK_ENTRY (key_bindings[i].entry),
                            key_bindings[i].data.c_str());

        key_bindings[i].button = gtk_button_new_with_label ("...");
        gtk_table_attach (GTK_TABLE (table), key_bindings[i].button, 2, 3, i, i+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), key_bindings[i].button);

        g_signal_connect(G_OBJECT(key_bindings[i].button), "clicked",
                         G_CALLBACK (on_default_key_selection_clicked),
                         &key_bindings[i]);
        g_signal_connect(G_OBJECT(key_bindings[i].entry), "changed",
                         G_CALLBACK (on_default_editable_changed), NULL);

        gtk_tooltips_set_tip(tooltips, key_bindings[i].entry,
                              _(key_bindings[i].tooltip), NULL);
    }

    return vbox1;
}

static GtkWidget *
create_setup_window ()
{
    GtkWidget *notebook;
    GtkWidget *label;
    GtkWidget *page;
    GtkTooltips *tooltips;

    tooltips = gtk_tooltips_new ();

    // Create the Notebook.
    notebook = gtk_notebook_new ();

    // Create the first page.
    page = create_keyboard_page(tooltips);
    label = gtk_label_new (_("Keyboard"));
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

    // Create the second page.
    page = create_options_page(tooltips);
    label = gtk_label_new (_("Options"));
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

    gtk_notebook_set_current_page(GTK_NOTEBOOK (notebook), 0);

    gtk_widget_show_all(notebook);

    return notebook;
}

static void
load_config (const ConfigPointer &config)
{
    if (config.null())
        return;

    // keyboard layout option
    String layout = config->read(String(SCIM_CONFIG_LAYOUT), String("2"));
    int no = -1;
    if (layout == "2") {
        no = 0;
    } else if (layout == "32") {
        no = 1;
    } else if (layout == "3f") {
        no = 2;
    } else if (layout == "39") {
        no = 3;
    } else if (layout == "3s") {
        no = 4;
    } else if (layout == "3y") {
        no = 5;
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(keyboard_layout_combo), no);

    // key bindings
    for (unsigned int i = 0; i < G_N_ELEMENTS(key_bindings); ++ i) {
        String text = 
            config->read (String (key_bindings[i].key), key_bindings[i].data);
        gtk_entry_set_text(GTK_ENTRY(key_bindings[i].entry), text.c_str());
    }

    bool stat;

    // use ascii input mode
    stat = config->read(String(SCIM_CONFIG_USE_ASCII_MODE), false);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_ascii_mode_button), stat);

    // show candidate comment option
    stat = config->read(String(SCIM_CONFIG_SHOW_CANDIDATE_COMMENT), true);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_candidate_comment_button), stat);

    // commit by word or not
    stat = config->read(String(SCIM_CONFIG_COMMIT_BY_WORD), false);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(commit_by_word_button), stat);

    __have_changed = false;
}

void
save_config (const ConfigPointer &config)
{
    if (config.null())
        return;

    int no = gtk_combo_box_get_active(GTK_COMBO_BOX(keyboard_layout_combo));
    String layout;
    if (no == 0) {
        layout = "2";
    } else if (no == 1) {
        layout = "32";
    } else if (no == 2) {
        layout = "3f";
    } else if (no == 3) {
        layout = "39";
    } else if (no == 4) {
        layout = "3s";
    } else if (no == 5) {
        layout = "3y";
    }
    config->write(String(SCIM_CONFIG_LAYOUT), layout);

    for (unsigned int i = 0; i < G_N_ELEMENTS(key_bindings); ++ i) {
        String text = gtk_entry_get_text(GTK_ENTRY(key_bindings[i].entry));
        config->write(String (key_bindings[i].key), text);
    }

    gboolean stat;

    // use ascii input mode
    stat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_ascii_mode_button));
    config->write(String(SCIM_CONFIG_USE_ASCII_MODE), (bool)stat);

    // show candidate comment
    stat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_candidate_comment_button));
    config->write(String(SCIM_CONFIG_SHOW_CANDIDATE_COMMENT), (bool)stat);

    // commit by word or not
    stat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(commit_by_word_button));
    config->write(String(SCIM_CONFIG_COMMIT_BY_WORD), (bool)stat);

    __have_changed = false;
}

bool
query_changed ()
{
    return __have_changed;
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    __have_changed = true;
}

static void
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    __have_changed = true;
}

static void
on_default_combo_box_changed(GtkComboBox *combobox,
                             gpointer     user_data)
{
    __have_changed = true;
}

static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    KeyBindingData *data = static_cast<KeyBindingData *> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (GTK_ENTRY (data->entry)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (GTK_ENTRY (data->entry))) != 0)
                gtk_entry_set_text (GTK_ENTRY (data->entry), keys);
        }

        gtk_widget_destroy (dialog);
    }
}

/*
vi:ts=4:nowrap:expandtab
*/
