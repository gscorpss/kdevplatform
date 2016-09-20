/* This file is part of KDevelop
Copyright 2008 Andreas Pakulat <apaku@gmx.de>

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

#include "session.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <kurl.h>
#include <kstandarddirs.h>
#include <kparts/mainwindow.h>
#include <kdebug.h>
#include <kstringhandler.h>

#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugin.h>
#include "core.h"
#include "sessioncontroller.h"
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <util/fileutils.h>
#include <language/duchain/repositories/itemrepository.h>

namespace
{

using namespace KDevelop;

QString generatePrettyContents( const SessionInfo& info )
{
    if( info.projects.isEmpty() )
        return QString();

    QStringList projectNames;
    projectNames.reserve( info.projects.size() );

    foreach( const KUrl& url, info.projects ) {
        IProject* project = 0;
        if( ICore::self() && ICore::self()->projectController() ) {
            project = ICore::self()->projectController()->findProjectForUrl( url );
        }

        if( project ) {
            projectNames << project->name();
        } else {
            QString projectName = url.fileName();
            projectName.remove( QRegExp( "\\.kdev4$", Qt::CaseInsensitive ) );
            projectNames << projectName;
        }
    }

    if( projectNames.isEmpty() ) {
        return i18n("(no projects)");
    } else {
        return projectNames.join( ", " );
    }
}

QString generateDescription( const SessionInfo& info )
{
    QString prettyContentsFormatted = generatePrettyContents( info );
    QString description;

    if( info.name.isEmpty() ) {
        description = prettyContentsFormatted;
    } else {
        description = info.name + ":  " + prettyContentsFormatted;
    }

    return description;
}

void buildDescription( SessionInfo& info )
{
    info.description = generateDescription( info );;
    info.config->group( QString() ).writeEntry( Session::cfgSessionDescriptionEntry, info.description );
    info.config->sync();
}
}

namespace KDevelop
{

const QString Session::cfgSessionNameEntry = "SessionName";
const QString Session::cfgSessionDescriptionEntry = "SessionPrettyContents";
const QString Session::cfgSessionProjectsEntry = "Open Projects";
const QString Session::cfgSessionOptionsGroup = "General Options";

Session::Session( const QString& id, QObject* parent )
        : ISession(parent)
        , info( Session::parse( id, true ) )
{
}

Session::~Session()
{
}

void Session::updateDescription()
{
    buildDescription( info );
    emit sessionUpdated( this );
}

KUrl Session::pluginDataArea( const IPlugin* p )
{
    QString name = Core::self()->pluginController()->pluginInfo( p ).pluginName();
    KUrl url( info.path );
    url.addPath( name );
    if( !QFile::exists( url.toLocalFile() ) ) {
        QDir( info.path ).mkdir( name );
    }
    return url;
}

void Session::setName( const QString& newname )
{
    info.name = newname;

    info.config->group( QString() ).writeEntry( cfgSessionNameEntry, newname );
    info.config->sync();

    updateDescription();
}

void Session::setContainedProjects( const KUrl::List& projects )
{
    info.projects = projects;

    info.config->group( cfgSessionOptionsGroup ).writeEntry( cfgSessionProjectsEntry, projects.toStringList() );
    info.config->sync();

    updateDescription();
}

SessionInfo Session::parse( const QString& id, bool mkdir )
{
    SessionInfo ret;
    QString sessionPath = SessionController::sessionDirectory(id);

    QDir sessionDir( sessionPath );
    if( !sessionDir.exists() ) {
        if( mkdir ) {
            sessionDir.mkpath(sessionPath);
            Q_ASSERT( sessionDir.exists() );
        } else {
            return ret;
        }
    }

    ret.uuid = id;
    ret.path = sessionPath;
    ret.config = KSharedConfig::openConfig( sessionPath + "/sessionrc" );

    KConfigGroup cfgRootGroup = ret.config->group( QString() );
    KConfigGroup cfgOptionsGroup = ret.config->group( cfgSessionOptionsGroup );

    ret.name = cfgRootGroup.readEntry( cfgSessionNameEntry, QString() );
    ret.projects = cfgOptionsGroup.readEntry( cfgSessionProjectsEntry, QStringList() );
    buildDescription( ret );

    return ret;
}

}
#include "session.moc"

