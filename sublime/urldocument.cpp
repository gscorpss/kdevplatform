/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
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
#include "urldocument.h"

#include <QWidget>
#include <KTextEdit>
#include <KIcon>
#include <KMimeType>

#include <kdebug.h>

namespace Sublime {

// struct UrlDocumentPrivate

// class UrlDocument

UrlDocument::UrlDocument(Controller *controller, const KUrl &url)
:Document(url.fileName(), controller, "Url")
{
    setUrl(url);
}

const KUrl& UrlDocument::getUrl() const
{
    return m_url;
}

void UrlDocument::setUrl(const KUrl& newUrl)
{
    //deep copy
    m_url.setEncodedUrl(newUrl.url().toAscii());
    setTitle(newUrl.fileName());
    setToolTip(newUrl.prettyUrl());
}

QWidget *UrlDocument::createViewWidget(QWidget *parent)
{
    ///@todo adymo: load file contents here
    return new KTextEdit(parent);
}

QString UrlDocument::documentSpecifier() const
{
    return m_url.url();
}

QIcon UrlDocument::defaultIcon() const
{
    return KIcon(KMimeType::iconNameForUrl(m_url));
}

}
