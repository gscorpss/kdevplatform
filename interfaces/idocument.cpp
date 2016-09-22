/***************************************************************************
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "idocument.h"

#include "icore.h"
#include "idocumentcontroller.h"

namespace KDevelop {

IDocument::IDocument( KDevelop::ICore* core )
: m_core(core)
{
}

IDocument::~IDocument()
{
}

KDevelop::ICore* IDocument::core()
{
    return m_core;
}

void IDocument::notifySaved()
{
    emit m_core->documentController()->documentSaved(this);
}

void IDocument::notifyStateChanged()
{
    emit m_core->documentController()->documentStateChanged(this);
}

void IDocument::notifyActivated()
{
    emit m_core->documentController()->documentActivated(this);
}

void IDocument::notifyContentChanged()
{
    emit m_core->documentController()->documentContentChanged(this);
}

bool IDocument::isTextDocument() const
{
    return false;
}

void IDocument::notifyTextDocumentCreated()
{
    emit m_core->documentController()->textDocumentCreated(this);
}

KTextEditor::Range IDocument::textSelection() const
{
    return KTextEditor::Range::invalid();
}

QString IDocument::textLine() const
{
    return QString();
}

QString IDocument::textWord() const
{
    return QString();
}

const QString& IDocument::prettyName() const
{
    return m_prettyName;
}

void IDocument::setPrettyName(QString name)
{
    m_prettyName = name;
}

void IDocument::notifyUrlChanged()
{
    emit m_core->documentController()->documentUrlChanged(this);
}

void IDocument::notifyLoaded()
{
    emit m_core->documentController()->documentLoadedPrepare(this);
    emit m_core->documentController()->documentLoaded(this);
}

}

