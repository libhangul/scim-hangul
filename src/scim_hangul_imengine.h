/** @file scim_hangul_imengine.h
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
 * $Id: scim_hangul_imengine.h,v 1.17 2007/05/27 13:08:07 hwanjin Exp $
 */

#if !defined (__SCIM_HANGUL_IMENGINE_H)
#define __SCIM_HANGUL_IMENGINE_H

#include <hangul.h>

using namespace scim;

enum HangulInputMode
{
    INPUT_MODE_DIRECT,
    INPUT_MODE_HANGUL,
    INPUT_MODE_HANJA
};

enum HangulOutputMode
{
    OUTPUT_MODE_SYLLABLE = 0,
    OUTPUT_MODE_JAMO     = 1 << 1,
    OUTPUT_MODE_JAMO_EXT = 1 << 2
};

class HangulFactory : public IMEngineFactoryBase
{
    String                   m_uuid;
    String                   m_name;

    ConfigPointer            m_config;

    String                   m_keyboard_layout;

    bool                     m_always_use_jamo;

    bool                     m_show_candidate_comment;
    bool                     m_lookup_table_vertical;
    bool                     m_use_ascii_mode;
    bool                     m_commit_by_word;
    bool                     m_hanja_mode;

    KeyEventList             m_hangul_keys;
    KeyEventList             m_hanja_keys;
    KeyEventList             m_hanja_mode_keys;

    Connection               m_reload_signal_connection;

    HanjaTable*              m_hanja_table;
    HanjaTable*              m_symbol_table;

    friend class HangulInstance;

public:
    HangulFactory (const ConfigPointer &config);

    virtual ~HangulFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);

private:
    void reload_config (const ConfigPointer &config);
};

class HangulInstance : public IMEngineInstanceBase
{
    HangulFactory       *m_factory;

    CommonLookupTable    m_lookup_table;
    std::vector<String>  m_candidate_comments;
    WideString           m_preedit;
    WideString           m_surrounding_text;

    KeyEvent             m_prev_key;

    HangulInputContext  *m_hic;

    bool                 m_hangul_mode;
    int                  m_output_mode;

public:
    HangulInstance (HangulFactory *factory,
                    const String  &encoding,
                    int            id = -1);

    virtual ~HangulInstance ();

    virtual bool process_key_event (const KeyEvent& key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void select_candidate (unsigned int item);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up ();
    virtual void lookup_table_page_down ();
    virtual void reset ();
    virtual void flush ();
    virtual void focus_in ();
    virtual void focus_out ();
    virtual void trigger_property (const String &property);

private:
    bool is_backspace_key (const KeyEvent &key) const {
        return (key.code == SCIM_KEY_BackSpace);
    }

    bool is_hangul_key (const KeyEvent &key) const {
        return match_key_event (m_factory->m_hangul_keys, key);
    }

    bool is_hanja_key (const KeyEvent &key) const {
        return match_key_event (m_factory->m_hanja_keys, key);
    }

    bool is_hanja_mode_key (const KeyEvent &key) const {
        return match_key_event (m_factory->m_hanja_mode_keys, key);
    }

    /* preedit string */
    WideString get_preedit_string () {
        WideString wstr = m_preedit;
        const ucschar *str = hangul_ic_get_preedit_string(m_hic);
        while (*str != 0)
            wstr.push_back (*str++);
        return wstr;
    }

    WideString get_commit_string () {
        WideString wstr;
        const ucschar *str = hangul_ic_get_commit_string(m_hic);
        while (*str != L'\0')
            wstr.push_back (*str++);
        return wstr;
    }

    void hangul_update_preedit_string ();

    bool   use_ascii_mode() {
        return m_factory->m_use_ascii_mode;
    }

    bool   is_hangul_mode() {
        return m_hangul_mode;
    }

    bool   is_hanja_mode() {
        return m_factory->m_hanja_mode;
    }

    void   toggle_hangul_mode();
    void   toggle_hanja_mode();
    void   change_keyboard_layout(const String &layout);

    /* property handling */
    void   register_all_properties();

    /* aux string */
    void hangul_update_aux_string ();

    /* candidate functions */
    String get_candidate_string();
    void   update_candidates ();
    void   delete_candidates ();

    /* candidate keys */
    bool   candidate_key_event (const KeyEvent &key);

    /* match key event */
    bool   match_key_event (const KeyEventList &keys, const KeyEvent &key) const;
};
#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
