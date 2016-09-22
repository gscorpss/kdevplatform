/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.org>
Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "editorcontext.h"
#include <ktexteditor/cursor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

namespace KDevelop {

EditorContext::EditorContext( KTextEditor::View* view, KTextEditor::Cursor position )
: DeclarationContext(Context::EditorContext, view, position ), m_view(view)
{
    m_url = view->document()->url();
    m_position = position;
    m_currentLine = view->document()->line(m_position.line());
    int wordStart = m_position.column();
    int wordEnd = m_position.column();
    while(wordStart > 0 && wordStart < m_currentLine.length() && (m_currentLine[wordStart-1].isLetterOrNumber() || m_currentLine[wordStart-1] == '_'))
        --wordStart;
    while(wordEnd >= 0 && wordEnd < m_currentLine.length() && (m_currentLine[wordEnd].isLetterOrNumber() || m_currentLine[wordEnd] == '_'))
        ++wordEnd;
}

}

