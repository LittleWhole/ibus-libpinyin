/* vim:set et ts=4 sts=4:
 *
 * ibus-pinyin - The Chinese PinYin engine for IBus
 *
 * Copyright (c) 2011 Peng Wu <alexepico@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __PY_LIB_PINYIN_BASE_EDITOR_H_
#define __PY_LIB_PINYIN_BASE_EDITOR_H_

#include <pinyin.h>
#include "PYLookupTable.h"
#include "PYEditor.h"
#include "PYPinyinParser.h"
#include "PYSpecialPhraseTable.h"

namespace PY {

class LibPinyinPhoneticEditor : public Editor {
public:
    LibPinyinPhoneticEditor (PinyinProperties & props, Config & config);

public:
    /* virtual functions */
    virtual void pageUp (void);
    virtual void pageDown (void);
    virtual void cursorUp (void);
    virtual void cursorDown (void);
    virtual void update (void);
    virtual void reset (void);
    virtual void candidateClicked (guint index, guint button, guint state);
    virtual gboolean processKeyEvent (guint keyval, guint keycode, guint modifiers);
    virtual gboolean processSpace (guint keyval, guint keycode, guint modifiers);
    virtual gboolean processFunctionKey (guint keyval, guint keycode, guint modifiers);
    virtual void updateLookupTable ();
    virtual void updateLookupTableFast ();
    virtual gboolean fillLookupTableByPage ();

protected:

    gboolean updateSpecialPhrases ();
    gboolean selectCandidate (guint i);
    gboolean selectCandidateInPage (guint i);

    void commit (const gchar *str);

    /* inline functions */

    /* pure virtual functions */
    virtual gboolean insert (gint ch) = 0;
    virtual gboolean removeCharBefore (void) = 0;
    virtual gboolean removeCharAfter (void) = 0;
    virtual gboolean removeWordBefore (void) = 0;
    virtual gboolean removeWordAfter (void) = 0;
    virtual gboolean moveCursorLeft (void) = 0;
    virtual gboolean moveCursorRight (void) = 0;
    virtual gboolean moveCursorLeftByWord (void) = 0;
    virtual gboolean moveCursorRightByWord (void) = 0;
    virtual gboolean moveCursorToBegin (void) = 0;
    virtual gboolean moveCursorToEnd (void) = 0;
    virtual void commit (void) = 0;
    virtual void updateAuxiliaryText (void) = 0;
    virtual void updatePreeditText (void) = 0;

    /* varibles */
    PinyinArray                 m_pinyin;
    guint                       m_pinyin_len;
    LookupTable                 m_lookup_table;
    String                      m_buffer;

    /* use LibPinyinBackEnd here. */
    CandidateConstraints m_constraints;
    MatchResults m_match_results;

    std::vector<std::string>    m_special_phrases;
    std::string                 m_selected_special_phrase;
};

};

#endif
