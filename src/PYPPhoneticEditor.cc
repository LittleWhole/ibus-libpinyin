/* vim:set et ts=4 sts=4:
 *
 * ibus-libpinyin - Intelligent Pinyin engine based on libpinyin for IBus
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "PYPPhoneticEditor.h"
#include "PYConfig.h"
#include "PYPinyinProperties.h"
#include "PYSimpTradConverter.h"

using namespace PY;

/* init static members */
LibPinyinPhoneticEditor::LibPinyinPhoneticEditor (PinyinProperties &props,
                                                  Config &config):
    Editor (props, config),
    m_pinyin_len (0),
    m_lookup_table (m_config.pageSize ())
{
    m_candidates = g_array_new(FALSE, TRUE, sizeof(lookup_candidate_t));
}

LibPinyinPhoneticEditor::~LibPinyinPhoneticEditor (){
    pinyin_free_candidates (m_instance, m_candidates);
    g_array_free (m_candidates, TRUE);
    m_candidates = NULL;
}

gboolean
LibPinyinPhoneticEditor::processSpace (guint keyval, guint keycode,
                                       guint modifiers)
{
    if (!m_text)
        return FALSE;
    if (cmshm_filter (modifiers) != 0)
        return TRUE;

    if (m_lookup_table.size () != 0) {
        selectCandidate (m_lookup_table.cursorPos ());
        update ();
    }
    else {
        commit ();
    }

    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::processFunctionKey (guint keyval, guint keycode, guint modifiers)
{
    if (m_text.empty ())
        return FALSE;

    /* ignore numlock */
    modifiers = cmshm_filter (modifiers);

    if (modifiers != 0 && modifiers != IBUS_CONTROL_MASK)
        return TRUE;

    /* process some cursor control keys */
    if (modifiers == 0) {  /* no modifiers. */
        switch (keyval) {
        case IBUS_Return:
        case IBUS_KP_Enter:
            commit (m_text.c_str ());
            reset ();
            return TRUE;

        case IBUS_BackSpace:
            removeCharBefore ();
            return TRUE;

        case IBUS_Delete:
        case IBUS_KP_Delete:
            removeCharAfter ();
            return TRUE;

        case IBUS_Left:
        case IBUS_KP_Left:
            moveCursorLeft ();
            return TRUE;

        case IBUS_Right:
        case IBUS_KP_Right:
            moveCursorRight ();
            return TRUE;

        case IBUS_Home:
        case IBUS_KP_Home:
            moveCursorToBegin ();
            return TRUE;

        case IBUS_End:
        case IBUS_KP_End:
            moveCursorToEnd ();
            return TRUE;

        case IBUS_Up:
        case IBUS_KP_Up:
            cursorUp ();
            return TRUE;

        case IBUS_Down:
        case IBUS_KP_Down:
            cursorDown ();
            return TRUE;

        case IBUS_Page_Up:
        case IBUS_KP_Page_Up:
            pageUp ();
            return TRUE;

        case IBUS_Page_Down:
        case IBUS_KP_Page_Down:
        case IBUS_Tab:
            pageDown ();
            return TRUE;

        case IBUS_Escape:
            reset ();
            return TRUE;
        default:
            return TRUE;
        }
    } else { /* ctrl key pressed. */
        switch (keyval) {
        case IBUS_BackSpace:
            removeWordBefore ();
            return TRUE;

        case IBUS_Delete:
        case IBUS_KP_Delete:
            removeWordAfter ();
            return TRUE;

        case IBUS_Left:
        case IBUS_KP_Left:
            moveCursorLeftByWord ();
            return TRUE;

        case IBUS_Right:
        case IBUS_KP_Right:
            moveCursorToEnd ();
            return TRUE;

        default:
            return TRUE;
        }
    }
    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::processKeyEvent (guint keyval, guint keycode, guint modifiers)
{
    return FALSE;
}

void
LibPinyinPhoneticEditor::updateLookupTableFast (void)
{
    Editor::updateLookupTableFast (m_lookup_table, TRUE);
}

void
LibPinyinPhoneticEditor::updateLookupTable (void)
{
    m_lookup_table.clear ();

    fillLookupTableByPage ();
    if (m_lookup_table.size()) {
        Editor::updateLookupTable (m_lookup_table, TRUE);
    } else {
        hideLookupTable ();
    }
}

gboolean
LibPinyinPhoneticEditor::fillLookupTableByPage (void)
{

    guint filled_nr = m_lookup_table.size ();
    guint page_size = m_lookup_table.pageSize ();

    /* fill lookup table by libpinyin get candidates. */
    guint need_nr = MIN (page_size, m_candidates->len - filled_nr);
    g_assert (need_nr >=0);
    if (need_nr == 0)
        return FALSE;

    String word;
    for (guint i = filled_nr; i < filled_nr + need_nr; i++) {
        if (i >= m_candidates->len)  /* no more candidates */
            break;

        lookup_candidate_t * candidate = &g_array_index
            (m_candidates, lookup_candidate_t, i);

        const gchar * phrase_string = candidate->m_phrase_string;

        /* show get candidates. */
        if (G_LIKELY (m_props.modeSimp ())) {
            word = phrase_string;
        } else { /* Traditional Chinese */
            word.truncate (0);
            SimpTradConverter::simpToTrad (phrase_string, word);
        }

        Text text (word);
        m_lookup_table.appendCandidate (text);
    }

    return TRUE;
}

void
LibPinyinPhoneticEditor::pageUp (void)
{
    if (G_LIKELY (m_lookup_table.pageUp ())) {
        updateLookupTableFast ();
        updatePreeditText ();
        updateAuxiliaryText ();
    }
}

void
LibPinyinPhoneticEditor::pageDown (void)
{
    if (G_LIKELY((m_lookup_table.pageDown ()) ||
                 (fillLookupTableByPage () && m_lookup_table.pageDown()))) {
        updateLookupTableFast ();
        updatePreeditText ();
        updateAuxiliaryText ();
    }
}

void
LibPinyinPhoneticEditor::cursorUp (void)
{
    if (G_LIKELY (m_lookup_table.cursorUp ())) {
        updateLookupTableFast ();
        updatePreeditText ();
        updateAuxiliaryText ();
    }
}

void
LibPinyinPhoneticEditor::cursorDown (void)
{
    if (G_LIKELY ((m_lookup_table.cursorPos () == m_lookup_table.size() - 1) &&
                  (fillLookupTableByPage () == FALSE))) {
        return;
    }

    if (G_LIKELY (m_lookup_table.cursorDown ())) {
        updateLookupTableFast ();
        updatePreeditText ();
        updateAuxiliaryText ();
    }
}

void
LibPinyinPhoneticEditor::candidateClicked (guint index, guint button, guint state)
{
    selectCandidateInPage (index);
}

void
LibPinyinPhoneticEditor::reset (void)
{
    m_pinyin_len = 0;
    m_lookup_table.clear ();

    pinyin_free_candidates (m_instance, m_candidates);
    pinyin_reset (m_instance);

    Editor::reset ();
}

void
LibPinyinPhoneticEditor::update (void)
{
    guint lookup_cursor = getLookupCursor ();
    pinyin_get_candidates (m_instance, lookup_cursor, m_candidates);

    updateLookupTable ();
    updatePreeditText ();
    updateAuxiliaryText ();
}

void
LibPinyinPhoneticEditor::commit (const gchar *str)
{
    StaticText text(str);
    commitText (text);
}

guint
LibPinyinPhoneticEditor::getPinyinCursor ()
{
    /* Translate cursor position to pinyin position. */
    PinyinKeyPosVector & pinyin_poses = m_instance->m_pinyin_key_rests;
    guint pinyin_cursor = pinyin_poses->len;

    guint16 prev_end = 0, cur_end;
    for (size_t i = 0; i < pinyin_poses->len; ++i) {
        PinyinKeyPos *pos = &g_array_index
            (pinyin_poses, PinyinKeyPos, i);
        cur_end = pos->m_raw_end;

        if (prev_end <= m_cursor && m_cursor < cur_end)
            pinyin_cursor = i;

        prev_end = cur_end;
    }

    g_assert (pinyin_cursor >= 0);
    return pinyin_cursor;
}

guint
LibPinyinPhoneticEditor::getLookupCursor (void)
{
    PinyinKeyVector & pinyins = m_instance->m_pinyin_keys;
    guint lookup_cursor = getPinyinCursor ();

    /* show candidates when pinyin cursor is at end. */
    if (lookup_cursor == pinyins->len && m_pinyin_len == m_text.length())
        lookup_cursor = 0;
    return lookup_cursor;
}

gboolean
LibPinyinPhoneticEditor::selectCandidate (guint i)
{

    if (G_UNLIKELY (i >= m_candidates->len))
        return FALSE;

    guint lookup_cursor = getLookupCursor ();

    lookup_candidate_t * candidate = &g_array_index
        (m_candidates, lookup_candidate_t, i);
    if (BEST_MATCH_CANDIDATE == candidate->m_candidate_type) {
        commit ();
        return TRUE;
    }

    lookup_cursor = pinyin_choose_candidate
        (m_instance, lookup_cursor, candidate);
    if (DIVIDED_CANDIDATE == candidate->m_candidate_type ||
        RESPLIT_CANDIDATE == candidate->m_candidate_type) {
        m_text = m_instance->m_raw_full_pinyin;
        updatePinyin ();
    }
    pinyin_guess_sentence (m_instance);

    PinyinKeyPosVector & pinyin_poses = m_instance->m_pinyin_key_rests;
    if (lookup_cursor == pinyin_poses->len) {
        pinyin_train(m_instance);
        commit();
        return TRUE;
    }
    PinyinKeyPos *pos = &g_array_index
        (pinyin_poses, PinyinKeyPos, lookup_cursor);
    m_cursor = pos->m_raw_begin;

    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::selectCandidateInPage (guint i)
{
    guint page_size = m_lookup_table.pageSize ();
    guint cursor_pos = m_lookup_table.cursorPos ();

    if (G_UNLIKELY (i >= page_size))
        return FALSE;
    i += (cursor_pos / page_size) * page_size;

    return selectCandidate (i);
}

gboolean
LibPinyinPhoneticEditor::removeCharBefore (void)
{
    if (G_UNLIKELY (m_cursor == 0))
        return FALSE;

    m_cursor --;
    m_text.erase (m_cursor, 1);

    updatePinyin ();
    update ();

    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::removeCharAfter (void)
{
    if (G_UNLIKELY (m_cursor == m_text.length ()))
        return FALSE;

    m_text.erase (m_cursor, 1);

    updatePinyin ();
    update ();

    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::moveCursorLeft (void)
{
    if (G_UNLIKELY (m_cursor == 0))
        return FALSE;

    m_cursor --;
    update ();
    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::moveCursorRight (void)
{
    if (G_UNLIKELY (m_cursor == m_text.length ()))
        return FALSE;

    m_cursor ++;
    update ();
    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::moveCursorToBegin (void)
{
    if (G_UNLIKELY (m_cursor == 0))
        return TRUE;

    m_cursor = 0;
    update ();
    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::moveCursorToEnd (void)
{
    if (G_UNLIKELY (m_cursor == m_text.length ()))
        return FALSE;

    m_cursor = m_text.length ();
    update ();
    return TRUE;
}


/* move cursor by word functions */

guint
LibPinyinPhoneticEditor::getCursorLeftByWord (void)
{
    guint cursor;

    if (G_UNLIKELY (m_cursor > m_pinyin_len)) {
        cursor = m_pinyin_len;
    } else {
        PinyinKeyPosVector & pinyin_poses = m_instance->m_pinyin_key_rests;
        guint pinyin_cursor = getPinyinCursor ();

        PinyinKeyPos *pos = NULL;

        if (pinyin_cursor < pinyin_poses->len) {
            pos = &g_array_index
                (pinyin_poses, PinyinKeyPos, pinyin_cursor);
            cursor = pos->m_raw_begin;
        } else {
            /* at the end of pinyin string. */
            cursor  = m_cursor;
        }

        /* cursor at the begin of one pinyin */
        g_return_val_if_fail (pinyin_cursor > 0, 0);
        if ( cursor == m_cursor) {
            pos = &g_array_index
                (pinyin_poses, PinyinKeyPos, pinyin_cursor - 1);
            cursor = pos->m_raw_begin;
        }
    }

    return cursor;
}

guint
LibPinyinPhoneticEditor::getCursorRightByWord (void)
{
    guint cursor;

    if (G_UNLIKELY (m_cursor > m_pinyin_len)) {
        cursor = m_text.length ();
    } else {
        guint pinyin_cursor = getPinyinCursor ();
        PinyinKeyPos *pos = &g_array_index
            (m_instance->m_pinyin_key_rests, PinyinKeyPos, pinyin_cursor);
        cursor = pos->m_raw_end;
    }

    return cursor;
}

gboolean
LibPinyinPhoneticEditor::removeWordBefore (void)
{
    if (G_UNLIKELY (m_cursor == 0))
        return FALSE;

    guint cursor = getCursorLeftByWord ();
    m_text.erase (cursor, m_cursor - cursor);
    m_cursor = cursor;
    updatePinyin ();
    update ();
    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::removeWordAfter (void)
{
    if (G_UNLIKELY (m_cursor == m_text.length ()))
        return FALSE;

    guint cursor = getCursorRightByWord ();
    m_text.erase (m_cursor, cursor - m_cursor);
    updatePinyin ();
    update ();
    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::moveCursorLeftByWord (void)
{
    if (G_UNLIKELY (m_cursor == 0))
        return FALSE;

    guint cursor = getCursorLeftByWord ();

    m_cursor = cursor;
    update ();
    return TRUE;
}

gboolean
LibPinyinPhoneticEditor::moveCursorRightByWord (void)
{
    if (G_UNLIKELY (m_cursor == m_text.length ()))
        return FALSE;

    guint cursor = getCursorRightByWord ();

    m_cursor = cursor;
    update ();
    return TRUE;
}
