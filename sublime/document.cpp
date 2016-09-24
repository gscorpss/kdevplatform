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
#include "document.h"

#include <kdebug.h>

#include "view.h"
#include "area.h"
#include "controller.h"

namespace Sublime {

//class Document

Document::Document(const QString &title, Controller *controller, const QString& documentType)
    :QObject(controller)
    , m_controller(controller)
    , m_documentType(documentType)
{
    setObjectName(title);
    m_controller->addDocument(this);
    connect(this, SIGNAL(destroyed(QObject*)), m_controller, SLOT(removeDocument(QObject*)));
}

void Document::removeView(QObject *obj)
{
    m_views.removeAll(reinterpret_cast<Sublime::View*>(obj));
    emit viewNumberChanged(this);
    //no need to keep empty document - we need to remove it
    if (m_views.count() == 0)
    {
        emit aboutToDelete(this);
        deleteLater();
    }
}

Controller *Document::controller() const
{
    return m_controller;
}

View *Document::createView()
{
    View *view = newView(this);
    connect(view, SIGNAL(destroyed(QObject*)), this, SLOT(removeView(QObject*)));
    m_views.append(view);
    return view;
}

const QList<View*> &Document::views() const
{
    return m_views;
}

QString Document::title() const
{
    return objectName();
}

const QString& Document::toolTip() const
{
    return documentToolTip;
}

void Document::setTitle(const QString& newTitle)
{
    setObjectName(newTitle);
    emit titleChanged(this);
}

void Document::setToolTip(const QString& newToolTip)
{
    documentToolTip=newToolTip;
}

View *Document::newView(Document *doc)
{
    //first create View, second emit the signal
    View *newView = new View(doc);
    emit viewNumberChanged(this);
    return newView;
}


void Document::setStatusIcon(QIcon icon)
{
    m_statusIcon = icon;
    emit statusIconChanged(this);
}

QIcon Document::statusIcon() const
{
    return m_statusIcon;
}

void Document::closeViews()
{
    kDebug() << "closing all views for the document";
    foreach (Sublime::Area *area, controller()->allAreas())
    {
        QList<Sublime::View*> areaViews = area->views();
        foreach (Sublime::View *view, areaViews) {
            if (views().contains(view)) {
                area->removeView(view);
                delete view;
            }
        }
    }
    Q_ASSERT(views().isEmpty());
}

bool Document::askForCloseFeedback()
{
   return true;
}

bool Document::closeDocument(bool silent)
{
    if( !silent && !askForCloseFeedback() )
        return false;
    closeViews();
    deleteLater();
    return true;
}

QIcon Document::icon() const
{
    QIcon ret = statusIcon();
    if (!ret.isNull()) {
        return ret;
    }
    return defaultIcon();
}

QIcon Document::defaultIcon() const
{
    return QIcon();
}

}

#include "document.moc"
