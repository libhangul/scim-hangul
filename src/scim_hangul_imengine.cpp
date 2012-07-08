/** @file scim_hangul_imengine.cpp
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (C) 2004-2006 Choe Hwanjin
 * Copyright (c) 2004-2006 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_hangul_imengine.cpp,v 1.34 2007/05/27 13:08:07 hwanjin Exp $
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <cstring>
#include <scim.h>
#include "scim_hangul_imengine.h"

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

#define scim_module_init hangul_LTX_scim_module_init
#define scim_module_exit hangul_LTX_scim_module_exit
#define scim_imengine_module_init hangul_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory hangul_LTX_scim_imengine_module_create_factory

#define SCIM_IMENGINE_HANGUL "/IMEngine/Hangul"

#define SCIM_CONFIG_PREFIX SCIM_IMENGINE_HANGUL

#define SCIM_CONFIG_SHOW_CANDIDATE_COMMENT      SCIM_CONFIG_PREFIX "/ShowCandidateComment"
#define SCIM_CONFIG_HANGUL_KEY                  SCIM_CONFIG_PREFIX "/HangulKey"
#define SCIM_CONFIG_HANJA_KEY                   SCIM_CONFIG_PREFIX "/HanjaKey"
#define SCIM_CONFIG_HANJA_MODE_KEY              SCIM_CONFIG_PREFIX "/HanjaModeKey"
#define SCIM_CONFIG_LAYOUT                      SCIM_CONFIG_PREFIX "/KeyboardLayout"
#define SCIM_CONFIG_USE_ASCII_MODE              SCIM_CONFIG_PREFIX "/UseAsciiMode"
#define SCIM_CONFIG_COMMIT_BY_WORD              SCIM_CONFIG_PREFIX "/CommitByWord"
#define SCIM_CONFIG_HANJA_MODE                  SCIM_CONFIG_PREFIX "/HanjaMode"

#define SCIM_CONFIG_PANEL_LOOKUP_TABLE_VERTICAL "/Panel/Gtk/LookupTableVertical"

#define SCIM_PROP_PREFIX         "/IMEngine/Hangul"
#define SCIM_PROP_HANGUL_MODE    SCIM_PROP_PREFIX "/HangulMode"
#define SCIM_PROP_HANJA_MODE     SCIM_PROP_PREFIX "/HanjaMode"

#ifndef SCIM_HANGUL_ICON_FILE
    #define SCIM_HANGUL_ICON_FILE           (SCIM_ICONDIR "/scim-hangul.png")
#endif

#define SCIM_HANGUL_ICON_ON      SCIM_ICONDIR "/scim-hangul-on.png"
#define SCIM_HANGUL_ICON_OFF     SCIM_ICONDIR "/scim-hangul-off.png"

static ConfigPointer _scim_config (0);

static Property hangul_mode(SCIM_PROP_HANGUL_MODE, "");
static Property hanja_mode(SCIM_PROP_HANJA_MODE, "");

extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_HANGUL_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        _scim_config.reset ();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE(1) << "Initialize Hangul Engine\n";

        _scim_config = config;

        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        HangulFactory *factory = 0;

        try {
            factory = new HangulFactory (_scim_config);
        } catch (...) {
            delete factory;
            factory = 0;
        }

        return factory;
    }
}

HangulFactory::HangulFactory (const ConfigPointer &config)
{
    m_uuid = "d75857a5-4148-4745-89e2-1da7ddaf70a9";
    m_name = _("Korean");
    m_config = config;
    m_keyboard_layout = "2";
    m_show_candidate_comment = true;
    m_use_ascii_mode = false;
    m_commit_by_word = false;

    m_hanja_table = hanja_table_load(NULL);
    m_symbol_table = NULL;

    std::string symbol_file = getenv("HOME");
    symbol_file += "/.scim/hangul/symbol.txt";
    if (access(symbol_file.c_str(), R_OK) == 0)
	m_symbol_table = hanja_table_load(symbol_file.c_str());
    if (m_symbol_table == NULL) {
	symbol_file = SCIM_HANGUL_DATADIR "/symbol.txt";
	if (access(symbol_file.c_str(), R_OK) == 0)
	    m_symbol_table = hanja_table_load(symbol_file.c_str());
    }

    set_languages ("ko");

    reload_config(m_config);

    m_reload_signal_connection = m_config->signal_connect_reload(slot(this, &HangulFactory::reload_config));
}

HangulFactory::~HangulFactory ()
{
    m_reload_signal_connection.disconnect();
    if (m_hanja_table != NULL)
	hanja_table_delete(m_hanja_table);
}

WideString
HangulFactory::get_name () const
{
    return utf8_mbstowcs (m_name);
}

WideString
HangulFactory::get_authors () const
{
    return utf8_mbstowcs (String (_("Copyright (C) 2006 Choe Hwanjin <choe.hwanjin@gmail.com>")));
}

WideString
HangulFactory::get_credits () const
{
    return WideString ();
}

WideString
HangulFactory::get_help () const
{
    const char *header =
	_("Key bindings:\n");

    String hangul_keys;
    String hanja_keys;
    scim_key_list_to_string(hangul_keys, m_hangul_keys);
    scim_key_list_to_string(hanja_keys, m_hanja_keys);

    char paragraph1[512];
    char paragraph2[512];

    snprintf(paragraph1, sizeof(paragraph1),
	_("  Hangul key: %s\n"
	  "    This key binding is to switch the input mode between the ASCII input \n"
	  "    mode and the hangul input mode.\n"), hangul_keys.c_str());
    snprintf(paragraph2, sizeof(paragraph2),
	_("  Hanja key: %s\n"
	  "    This key binding is to convert a hangul character to a hanja character.\n"), hanja_keys.c_str());

    return utf8_mbstowcs (header)
	+ utf8_mbstowcs (paragraph1)
	+ utf8_mbstowcs (paragraph2);
}

String
HangulFactory::get_uuid () const
{
    return m_uuid;
}

String
HangulFactory::get_icon_file () const
{
    return String (SCIM_HANGUL_ICON_FILE);
}

void
HangulFactory::reload_config(const ConfigPointer &config)
{
    if (config.null())
	return;

    m_show_candidate_comment = config->read(String(SCIM_CONFIG_SHOW_CANDIDATE_COMMENT),
					    m_show_candidate_comment);

    m_keyboard_layout = config->read(String(SCIM_CONFIG_LAYOUT), String("2"));

    m_use_ascii_mode = config->read(String(SCIM_CONFIG_USE_ASCII_MODE),
				    false);
    m_commit_by_word = config->read(String(SCIM_CONFIG_COMMIT_BY_WORD),
				    false);
    m_hanja_mode = config->read(String(SCIM_CONFIG_HANJA_MODE),
				    false);

    String str;
    str = config->read(String(SCIM_CONFIG_HANGUL_KEY),
		       String("Hangul,Shift+space"));
    scim_string_to_key_list(m_hangul_keys, str);

    str = config->read(String (SCIM_CONFIG_HANJA_KEY),
		       String ("Hangul_Hanja,F9"));
    scim_string_to_key_list(m_hanja_keys, str);

    str = config->read(String (SCIM_CONFIG_HANJA_MODE_KEY),
		       String (""));
    scim_string_to_key_list(m_hanja_mode_keys, str);

    m_lookup_table_vertical = config->read(String(SCIM_CONFIG_PANEL_LOOKUP_TABLE_VERTICAL),
					   false);
}

IMEngineInstancePointer
HangulFactory::create_instance (const String &encoding, int id)
{
    SCIM_DEBUG_IMENGINE(1) << "create_instance: HangulInstance.\n";
    return new HangulInstance (this, encoding, id);
}

HangulInstance::HangulInstance (HangulFactory *factory,
                                const String  &encoding,
                                int            id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_prev_key (0,0),
      m_output_mode (OUTPUT_MODE_SYLLABLE)
{
    m_hic = hangul_ic_new(factory->m_keyboard_layout.c_str());

    char label[16];
    std::vector <WideString> labels;

    for (int i = 1; i < 10; ++i) {
        snprintf (label, sizeof(label), "%d", i);
        labels.push_back (utf8_mbstowcs (label));
    }

    m_lookup_table.set_candidate_labels (labels);

    m_hangul_mode = true;
}

HangulInstance::~HangulInstance ()
{
}

bool
HangulInstance::candidate_key_event (const KeyEvent &key)
{
    switch (key.code) {
        case SCIM_KEY_Return:
        case SCIM_KEY_KP_Enter:
            select_candidate (m_lookup_table.get_cursor_pos_in_current_page ());
            break;
        case SCIM_KEY_KP_Subtract:
	    m_lookup_table.cursor_up ();
	    update_lookup_table (m_lookup_table);
	    hangul_update_aux_string ();
            break;
        case SCIM_KEY_space:
	    if (is_hanja_mode())
		return false;
        case SCIM_KEY_KP_Add:
	    m_lookup_table.cursor_down ();
	    update_lookup_table (m_lookup_table);
	    hangul_update_aux_string ();
	    break;
        case SCIM_KEY_Page_Up:
	    lookup_table_page_up();
            break;
        case SCIM_KEY_Page_Down:
        case SCIM_KEY_KP_Tab:
	    lookup_table_page_down();
            break;
        case SCIM_KEY_h:
	    if (is_hanja_mode())
		return false;
        case SCIM_KEY_Left:
	    if (m_factory->m_lookup_table_vertical) {
		lookup_table_page_up();
	    } else {
		m_lookup_table.cursor_up ();
		update_lookup_table (m_lookup_table);
		hangul_update_aux_string ();
	    }
            break;
        case SCIM_KEY_l:
	    if (is_hanja_mode())
		return false;
        case SCIM_KEY_Right:
	    if (m_factory->m_lookup_table_vertical) {
		lookup_table_page_down();
	    } else {
		m_lookup_table.cursor_down ();
		update_lookup_table (m_lookup_table);
		hangul_update_aux_string ();
	    }
            break;
        case SCIM_KEY_k:
	    if (is_hanja_mode())
		return false;
        case SCIM_KEY_Up:
	    if (m_factory->m_lookup_table_vertical) {
		m_lookup_table.cursor_up ();
		update_lookup_table (m_lookup_table);
		hangul_update_aux_string ();
	    } else {
		lookup_table_page_up();
	    }
            break;
        case SCIM_KEY_j:
	    if (is_hanja_mode())
		return false;
        case SCIM_KEY_Down:
	    if (m_factory->m_lookup_table_vertical) {
		m_lookup_table.cursor_down ();
		update_lookup_table (m_lookup_table);
		hangul_update_aux_string ();
	    } else {
		lookup_table_page_down();
	    }
            break;
        case SCIM_KEY_Escape:
            delete_candidates ();
            break;
        case SCIM_KEY_1: 
        case SCIM_KEY_2: 
        case SCIM_KEY_3: 
        case SCIM_KEY_4: 
        case SCIM_KEY_5: 
        case SCIM_KEY_6: 
        case SCIM_KEY_7: 
        case SCIM_KEY_8: 
        case SCIM_KEY_9: 
            select_candidate (key.code - SCIM_KEY_1);
            break;
        default:
	    return !is_hanja_mode();
    }

    return true;
}

bool
HangulInstance::process_key_event (const KeyEvent& rawkey)
{
    SCIM_DEBUG_IMENGINE(1) << "process_key_event.\n";

    KeyEvent key = rawkey.map_to_layout(SCIM_KEYBOARD_Default);

    m_prev_key = key;

    if (use_ascii_mode() && !is_hangul_mode()) {
	if (is_hangul_key(key)) {
	    toggle_hangul_mode();
	    return true;
	}

	return false;
    }

    /* ignore key release. */
    if (key.is_key_release ())
        return false;

    /* mode change */
    if (use_ascii_mode() && is_hangul_key(key)) {
	toggle_hangul_mode();
	return true;
    }

    /* hanja mode */
    if (is_hanja_mode_key (key)) {
	toggle_hanja_mode();
    }

    /* toggle candidate table */
    if (is_hanja_key (key)) {
	if (is_hanja_mode()) {
	    update_candidates ();
	} else {
	    if (m_lookup_table.number_of_candidates ())
		delete_candidates ();
	    else
		update_candidates ();
	}

        return true;
    }

    /* ignore shift keys */
    if (key.code == SCIM_KEY_Shift_L || key.code == SCIM_KEY_Shift_R)
        return false;

    /* flush on modifier-on keys */
    if (key.is_control_down() || key.is_alt_down()) {
	flush ();
        return false;
    }

    /* candidate keys */
    if (m_lookup_table.number_of_candidates ()) {
        if (candidate_key_event(key))
	    return true;
    }

    /* change to ascii mode on ESCAPE key, for vi users.
     * We should process this key after processing candidate keys,
     * or input mode will be changed to non-hangul mode when the user presses
     * escape key to close candidate window. */
    if (use_ascii_mode() && !is_hanja_mode()) {
	if (key.code == SCIM_KEY_Escape) {
	    toggle_hangul_mode();
	}
    }

    /* backspace */
    if (is_backspace_key(key)) {
        bool ret = hangul_ic_backspace(m_hic);
        if (ret) {
	    hangul_update_preedit_string ();
	} else if (m_preedit.length() > 0) {
	    ret = true;
	    m_preedit.erase(m_preedit.length() - 1, 1);
	    hangul_update_preedit_string();
	} else {
	    if (m_surrounding_text.length() > 0) {
		m_surrounding_text.erase(m_surrounding_text.length() - 1, 1);
		if (m_surrounding_text.empty()) {
		    delete_candidates();
		    return ret;
		}
	    }
	}

	if (is_hanja_mode() && m_lookup_table.number_of_candidates()) {
	    update_candidates();
	}

        return ret;
    }

    if (key.code >= SCIM_KEY_exclam && key.code <= SCIM_KEY_asciitilde) {
	/* main hangul composing process */
	int ascii = key.get_ascii_code();
	if (key.is_caps_lock_down()) {
	    if (isupper(ascii))
		ascii = tolower(ascii);
	    else if (islower(ascii))
		ascii = toupper(ascii);
	}

	bool ret = hangul_ic_process(m_hic, ascii);

	WideString wstr;
	wstr = get_commit_string ();
	if (wstr.length ()) {
	    /* Before commit, we set preedit string to null to work arround
	     * some buggy IM implementation, ex) Qt, Evolution */
	    hide_preedit_string ();
	    if (is_hanja_mode() || m_factory->m_commit_by_word) {
		m_preedit += wstr;
	    } else {
		commit_string(wstr);
	    }
	}

	if (is_hanja_mode() || m_factory->m_commit_by_word) {
	    if (hangul_ic_is_empty(m_hic)) {
		flush();
	    }
	}

	hangul_update_preedit_string ();

	if (is_hanja_mode()) {
	    update_candidates();
	}

	return ret;
    }

    flush();
    return false;
}

void
HangulInstance::move_preedit_caret (unsigned int pos)
{
}

void
HangulInstance::select_candidate (unsigned int index)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate.\n";

    if ((int)index >= m_lookup_table.get_current_page_size ())
	return;

    WideString candidate = m_lookup_table.get_candidate_in_current_page(index);

    WideString commit_str = candidate;
    WideString preedit = get_preedit_string();
    if (is_hanja_mode() || m_factory->m_commit_by_word) {
	// prefix method
	int len = m_surrounding_text.length();
	if (len > 0)
	    delete_surrounding_text(-len, len);
	if (candidate.length() <= m_surrounding_text.length()) {
	    len = m_surrounding_text.length() - candidate.length();
	    commit_str.append(m_surrounding_text, candidate.length(), len);
	    m_surrounding_text.erase(0, candidate.length());
	} else if (candidate.length() <= m_surrounding_text.length() + preedit.length()) {
	    len = candidate.length() - m_surrounding_text.length();
	    if (len > (int)m_preedit.length()) {
		m_preedit.clear();
		hangul_ic_reset(m_hic);
	    } else {
		m_preedit.erase(0, len);
	    }
	    m_surrounding_text.clear();
	} else {
	    m_preedit.clear();
	    hangul_ic_reset(m_hic);
	    m_surrounding_text.clear();
	}
    } else {
	// suffix method
	if (candidate.length() > preedit.length()) {
	    int len = candidate.length() - preedit.length();
	    delete_surrounding_text(-len, len);
	}
	hangul_ic_reset(m_hic);
	m_surrounding_text.clear();
    }

    commit_string(commit_str);
    hangul_update_preedit_string ();

    if (is_hanja_mode()) {
	update_candidates();
    } else {
	delete_candidates();
    }
}

void
HangulInstance::update_lookup_table_page_size (unsigned int page_size)
{
    SCIM_DEBUG_IMENGINE(2) << "update_lookup_table_page_size.\n";

    m_lookup_table.set_page_size (page_size);
}

void
HangulInstance::lookup_table_page_up ()
{
    if (!m_lookup_table.number_of_candidates () || !m_lookup_table.get_current_page_start ())
        return;

    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_up.\n";

    m_lookup_table.page_up ();

    update_lookup_table (m_lookup_table);

    hangul_update_aux_string ();
}

void
HangulInstance::lookup_table_page_down ()
{
    if (m_lookup_table.number_of_candidates () <= 0 ||
        m_lookup_table.get_current_page_start () + m_lookup_table.get_current_page_size () >=
          (int)m_lookup_table.number_of_candidates ())
        return;

    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_down.\n";

    m_lookup_table.page_down ();

    update_lookup_table (m_lookup_table);

    hangul_update_aux_string ();
}

void
HangulInstance::reset()
{
    SCIM_DEBUG_IMENGINE(2) << "reset.\n";
    flush();
}

void
HangulInstance::flush()
{
    SCIM_DEBUG_IMENGINE(2) << "flush.\n";

    hide_preedit_string();

    WideString wstr = m_preedit;
    const ucschar *str = hangul_ic_flush(m_hic);
    while (*str != 0)
	wstr.push_back (*str++);

    if (wstr.length())
        commit_string(wstr);

    delete_candidates ();
    m_preedit.clear();
}

void
HangulInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    register_all_properties();

    hangul_ic_select_keyboard(m_hic, m_factory->m_keyboard_layout.c_str());

    if (m_lookup_table.number_of_candidates ()) {
        update_lookup_table (m_lookup_table);
        show_lookup_table ();
    } else {
        hide_lookup_table ();
    }

    hangul_update_aux_string ();
}

void
HangulInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_out.\n";
    flush();
}

void
HangulInstance::trigger_property (const String &property)
{
    SCIM_DEBUG_IMENGINE(2) << "trigger_property.\n";
    if (property == SCIM_PROP_HANGUL_MODE) {
	toggle_hangul_mode();
    } else if (property == SCIM_PROP_HANJA_MODE) {
	toggle_hanja_mode();
    }
}

String
HangulInstance::get_candidate_string()
{
    int cursor = 0;
    if (m_surrounding_text.empty())
	get_surrounding_text(m_surrounding_text, cursor, 10, 0);

    int i; 
    for (i = m_surrounding_text.length() - 1; i >= 0; i--) {
	if (!hangul_is_syllable(m_surrounding_text[i]))
	    break;
    }
    if (i >= 0)
	m_surrounding_text.erase(0, i + 1);

    return utf8_wcstombs(m_surrounding_text + get_preedit_string());
}

void
HangulInstance::update_candidates ()
{
    m_lookup_table.clear ();
    m_candidate_comments.clear ();

    HanjaList* list = NULL;

    // search for symbol character
    // key string for symbol character is like:
    //  'ㄱ', 'ㄴ', 'ㄷ', etc
    WideString preeditw = get_preedit_string();
    if (preeditw.length() == 1) {
	String key = utf8_wcstombs(preeditw);
	list = hanja_table_match_suffix(m_factory->m_symbol_table, key.c_str());
    }

    // search for hanja
    if (list == NULL) {
	String str = get_candidate_string();
	SCIM_DEBUG_IMENGINE(1) << "candidate string: " << str << "\n";

	if (str.length() > 0) {
	    if (is_hanja_mode() || m_factory->m_commit_by_word) {
		list = hanja_table_match_prefix(m_factory->m_hanja_table,
						    str.c_str());
	    } else {
		list = hanja_table_match_suffix(m_factory->m_hanja_table,
						    str.c_str());
	    }
	}
    } 
    
    if (list != NULL) {
	int n = hanja_list_get_size(list);
	for (int i = 0; i < n; ++i) {
	    const char* value = hanja_list_get_nth_value(list, i);
	    const char* comment = hanja_list_get_nth_comment(list, i);
	    WideString candidate = utf8_mbstowcs(value, -1);
	    m_lookup_table.append_candidate(candidate);
	    m_candidate_comments.push_back(String(comment));
	}

	m_lookup_table.set_page_size (9);
	m_lookup_table.show_cursor ();

	update_lookup_table (m_lookup_table);
	show_lookup_table ();

	hangul_update_aux_string ();

	hanja_list_delete(list);
    }

    if (m_lookup_table.number_of_candidates() <= 0) {
	delete_candidates();
    }
}

void
HangulInstance::delete_candidates ()
{
    m_surrounding_text.clear();
    m_lookup_table.clear ();
    m_candidate_comments.clear ();
    hide_lookup_table ();
    hide_aux_string ();
}

void
HangulInstance::hangul_update_aux_string ()
{
    if (!m_factory->m_show_candidate_comment || !m_lookup_table.number_of_candidates ()) {
        hide_aux_string ();
        return;
    }

    size_t cursor = m_lookup_table.get_cursor_pos ();

    if (cursor >= m_candidate_comments.size ()) {
        hide_aux_string ();
        return;
    }

    update_aux_string (m_lookup_table.get_candidate (cursor) + utf8_mbstowcs (String (" : ") + m_candidate_comments [cursor]));
    show_aux_string ();
}

void
HangulInstance::hangul_update_preedit_string ()
{
    WideString wstr = get_preedit_string ();

    if (wstr.length ()) {
        AttributeList attrs;
        attrs.push_back(Attribute(0, m_preedit.length(), SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_UNDERLINE));
        attrs.push_back(Attribute(m_preedit.length(), wstr.length() - m_preedit.length(), SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));
        show_preedit_string ();
        update_preedit_string (wstr, attrs);
        update_preedit_caret (wstr.length());
    } else {
        hide_preedit_string ();
    }
}

bool
HangulInstance::match_key_event (const KeyEventList &keys, const KeyEvent &key) const
{
    KeyEventList::const_iterator kit; 

    for (kit = keys.begin (); kit != keys.end (); ++kit) {
	if (!key.is_key_release()) {
	    if (key.code == kit->code) {
		int mask = key.mask;
		// we should ignore capslock and numlock
		mask &= ~SCIM_KEY_CapsLockMask;
		mask &= ~SCIM_KEY_NumLockMask;
		if (mask == kit->mask)
		    return true;
	    }
	}
    }
    return false;
}

void
HangulInstance::toggle_hangul_mode()
{
    m_hangul_mode = !m_hangul_mode;
    flush();

    if (m_hangul_mode) {
	hangul_mode.set_label("한");
    } else {
	hangul_mode.set_label("Ａ");
    }

    update_property(hangul_mode);
}

void
HangulInstance::toggle_hanja_mode()
{
    m_factory->m_hanja_mode = !m_factory->m_hanja_mode;

    if (m_factory->m_hanja_mode) {
	hanja_mode.set_icon(SCIM_HANGUL_ICON_ON);
    } else {
	hanja_mode.set_icon(SCIM_HANGUL_ICON_OFF);
    }

    update_property(hanja_mode);

    m_factory->m_config->write(String(SCIM_CONFIG_HANJA_MODE), m_factory->m_hanja_mode);
}

void
HangulInstance::register_all_properties()
{
    PropertyList proplist;

    if (use_ascii_mode()) {
	if (m_hangul_mode) {
	    hangul_mode.set_label("한");
	} else {
	    hangul_mode.set_label("Ａ");
	}
	proplist.push_back(hangul_mode);
    }

    if (m_factory->m_hanja_mode) {
	hanja_mode.set_icon(SCIM_HANGUL_ICON_ON);
    } else {
	hanja_mode.set_icon(SCIM_HANGUL_ICON_OFF);
    }
    hanja_mode.set_label(_("Hanja Lock"));
    proplist.push_back(hanja_mode);

    register_properties(proplist);
}

//vim: ts=4:nowrap:ai:expandtab:
