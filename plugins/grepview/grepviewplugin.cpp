/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann                                 *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*   Copyright 2010 Benjamin Port <port.benjamin@gmail.com>                *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepviewplugin.h"
#include "grepdialog.h"
#include "grepoutputmodel.h"
#include "grepoutputdelegate.h"
#include "grepjob.h"
#include "grepoutputview.h"
#include "debug.h"

#include <QAction>
#include <QDBusConnection>
#include <QKeySequence>
#include <QMimeDatabase>

#include <KActionCollection>
#include <KLocalizedString>
#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <KTextEditor/Cursor>
#include <KTextEditor/View>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <language/interfaces/editorcontext.h>
#include <outputview/ioutputview.h>

Q_LOGGING_CATEGORY(PLUGIN_GREPVIEW, "kdevplatform.plugins.grepview")

static QString patternFromSelection(const KDevelop::IDocument* doc)
{
    if (!doc)
        return QString();

    QString pattern;
    KTextEditor::Range range = doc->textSelection();
    if( range.isValid() )
    {
        pattern = doc->textDocument()->text( range );
    }
    if( pattern.isEmpty() )
    {
        pattern = doc->textWord();
    }

    // Before anything, this removes line feeds from the
    // beginning and the end.
    int len = pattern.length();
    if (len > 0 && pattern[0] == '\n')
    {
        pattern.remove(0, 1);
        len--;
    }
    if (len > 0 && pattern[len-1] == '\n')
        pattern.truncate(len-1);
    return pattern;
}

GrepViewPlugin::GrepViewPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin( QStringLiteral("kdevgrepview"), parent ), m_currentJob(0)
{
    setXMLFile(QStringLiteral("kdevgrepview.rc"));

    QDBusConnection::sessionBus().registerObject( QStringLiteral("/org/kdevelop/GrepViewPlugin"),
        this, QDBusConnection::ExportScriptableSlots );

    QAction*action = actionCollection()->addAction(QStringLiteral("edit_grep"));
    action->setText(i18n("Find/Replace in Fi&les..."));
    actionCollection()->setDefaultShortcut( action, QKeySequence(QStringLiteral("Ctrl+Alt+F")) );
    connect(action, &QAction::triggered, this, &GrepViewPlugin::showDialogFromMenu);
    action->setToolTip( i18n("Search for expressions over several files") );
    action->setWhatsThis( i18n("Opens the 'Find/Replace in files' dialog. There you "
                               "can enter a regular expression which is then "
                               "searched for within all files in the directories "
                               "you specify. Matches will be displayed, you "
                               "can switch to a match directly. You can also do replacement.") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));

    // instantiate delegate, it's supposed to be deleted via QObject inheritance
    new GrepOutputDelegate(this);
    m_factory = new GrepOutputViewFactory(this);
    core()->uiController()->addToolView(i18n("Find/Replace in Files"), m_factory);
}

GrepOutputViewFactory* GrepViewPlugin::toolViewFactory() const
{
    return m_factory;
}

GrepViewPlugin::~GrepViewPlugin()
{
}

void GrepViewPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
}
void GrepViewPlugin::startSearch(QString pattern, QString directory, bool show)
{
    m_directory = directory;
    showDialog(false, pattern, show);
}

KDevelop::ContextMenuExtension GrepViewPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context);
    if( context->type() == KDevelop::Context::ProjectItemContext ) {
        KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
        QList<KDevelop::ProjectBaseItem*> items = ctx->items();
        // verify if there is only one folder selected
        if ((items.count() == 1) && (items.first()->folder())) {
            QAction* action = new QAction( i18n( "Find/Replace in This Folder..." ), this );
            action->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
            m_contextMenuDirectory = items.at(0)->folder()->path().toLocalFile();
            connect( action, &QAction::triggered, this, &GrepViewPlugin::showDialogFromProject);
            extension.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, action );
        }
    }

    if ( context->type() == KDevelop::Context::EditorContext ) {
        KDevelop::EditorContext *econtext = dynamic_cast<KDevelop::EditorContext*>(context);
        if ( econtext->view()->selection() ) {
            QAction* action = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("&Find/Replace in Files..."), this);
            connect(action, &QAction::triggered, this, &GrepViewPlugin::showDialogFromMenu);
            extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, action);
        }
    }

    if(context->type() == KDevelop::Context::FileContext) {
        KDevelop::FileContext *fcontext = dynamic_cast<KDevelop::FileContext*>(context);
        // TODO: just stat() or QFileInfo().isDir() for local files? should be faster than mime type checking
        QMimeType mimetype = QMimeDatabase().mimeTypeForUrl(fcontext->urls().at(0));
        static const QMimeType directoryMime = QMimeDatabase().mimeTypeForName(QStringLiteral("inode/directory"));
        if (mimetype == directoryMime) {
            QAction* action = new QAction( i18n( "Find/Replace in This Folder..." ), this );
            action->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
            m_contextMenuDirectory = fcontext->urls().at(0).toLocalFile();
            connect( action, &QAction::triggered, this, &GrepViewPlugin::showDialogFromProject);
            extension.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, action );
        }
    }
    return extension;
}

void GrepViewPlugin::showDialog(bool setLastUsed, QString pattern, bool show)
{
    GrepDialog* dlg = new GrepDialog( this, core()->uiController()->activeMainWindow() );
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();

    if(!pattern.isEmpty())
    {
        dlg->setPattern(pattern);
    }
    else if(!setLastUsed)
    {
        QString pattern = patternFromSelection(doc);
        if (!pattern.isEmpty()) {
            dlg->setPattern( pattern );
        }
    }

    //if directory is empty then use a default value from the config file.
    if (!m_directory.isEmpty()) {
        dlg->setSearchLocations(m_directory);
    }

    if(show)
        dlg->show();
    else{
        dlg->startSearch();
        dlg->deleteLater();
    }
}

void GrepViewPlugin::showDialogFromMenu()
{
    showDialog();
}

void GrepViewPlugin::showDialogFromProject()
{
    rememberSearchDirectory(m_contextMenuDirectory);
    showDialog();
}

void GrepViewPlugin::rememberSearchDirectory(QString const & directory)
{
    m_directory = directory;
}

GrepJob* GrepViewPlugin::newGrepJob()
{
    if(m_currentJob != 0)
    {
        m_currentJob->kill();
    }
    m_currentJob = new GrepJob();
    connect(m_currentJob, &GrepJob::finished, this, &GrepViewPlugin::jobFinished);
    return m_currentJob;
}

GrepJob* GrepViewPlugin::grepJob()
{
    return m_currentJob;
}

void GrepViewPlugin::jobFinished(KJob* job)
{
    if(job == m_currentJob)
    {
        emit grepJobFinished();
        m_currentJob = 0;
    }
}
