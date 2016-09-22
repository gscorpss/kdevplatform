/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2009 Niko Sams <niko.sams@gmail.com>

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
#include "scriptappconfig.h"

#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/ilaunchconfiguration.h>

#include <project/projectmodel.h>

#include "scriptappjob.h"
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/builderjob.h>
#include <kmessagebox.h>
#include <interfaces/iuicontroller.h>
#include <util/executecompositejob.h>
#include <kparts/mainwindow.h>
#include <KFileDialog>
#include <KShell>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>

#include "executescriptplugin.h"
#include <util/kdevstringhandler.h>
#include <util/environmentgrouplist.h>
#include <project/projectitemlineedit.h>

static const QString interpreterForUrl(const KUrl& url) {
    auto mimetype = KMimeType::findByUrl(url);
    static QHash<QString, QString> knownMimetypes;
    if ( knownMimetypes.isEmpty() ) {
        knownMimetypes["text/x-python"] = "python";
        knownMimetypes["application/x-php"] = "php";
        knownMimetypes["application/x-ruby"] = "ruby";
        knownMimetypes["application/x-shellscript"] = "bash";
        knownMimetypes["application/x-perl"] = "perl -e";
    }
    const QString& interp = knownMimetypes.value(mimetype->name());
    return interp;
}

KIcon ScriptAppConfigPage::icon() const
{
    return KIcon("system-run");
}

void ScriptAppConfigPage::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* project )
{
    bool b = blockSignals( true );
    if( project )
    {
        executablePath->setStartDir( project->folder() );
    }
    
    auto doc = KDevelop::ICore::self()->documentController()->activeDocument();
    interpreter->lineEdit()->setText( cfg.readEntry( ExecuteScriptPlugin::interpreterEntry,
                                                     doc ? interpreterForUrl(doc->url()) : "" ) );
    executablePath->setUrl( cfg.readEntry( ExecuteScriptPlugin::executableEntry, "" ) );
    remoteHostCheckbox->setChecked( cfg.readEntry( ExecuteScriptPlugin::executeOnRemoteHostEntry, false ) );
    remoteHost->setText( cfg.readEntry( ExecuteScriptPlugin::remoteHostEntry, "" ) );
    bool runCurrent = cfg.readEntry( ExecuteScriptPlugin::runCurrentFileEntry, true );
    if ( runCurrent ) {
        runCurrentFile->setChecked( true );
    } else {
        runFixedFile->setChecked( true );
    }
    arguments->setText( cfg.readEntry( ExecuteScriptPlugin::argumentsEntry, "" ) );
    workingDirectory->setUrl( cfg.readEntry( ExecuteScriptPlugin::workingDirEntry, KUrl() ) );
    environment->setCurrentProfile( cfg.readEntry( ExecuteScriptPlugin::environmentGroupEntry, QString() ) );
    outputFilteringMode->setCurrentIndex( cfg.readEntry( ExecuteScriptPlugin::outputFilteringEntry, 2u ));
    //runInTerminal->setChecked( cfg.readEntry( ExecuteScriptPlugin::useTerminalEntry, false ) );
    blockSignals( b );
}

ScriptAppConfigPage::ScriptAppConfigPage( QWidget* parent )
    : LaunchConfigurationPage( parent )
{
    setupUi(this);
    interpreter->lineEdit()->setPlaceholderText(i18n("Type or select an interpreter"));

    //Set workingdirectory widget to ask for directories rather than files
    workingDirectory->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    //connect signals to changed signal
    connect( interpreter->lineEdit(), SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( executablePath->lineEdit(), SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( executablePath, SIGNAL(urlSelected(KUrl)), SIGNAL(changed()) );
    connect( arguments, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( workingDirectory, SIGNAL(urlSelected(KUrl)), SIGNAL(changed()) );
    connect( workingDirectory->lineEdit(), SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( environment, SIGNAL(currentProfileChanged(QString)), SIGNAL(changed()) );
    //connect( runInTerminal, SIGNAL(toggled(bool)), SIGNAL(changed()) );
}

void ScriptAppConfigPage::saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project ) const
{
    Q_UNUSED( project );
    cfg.writeEntry( ExecuteScriptPlugin::interpreterEntry, interpreter->lineEdit()->text() );
    cfg.writeEntry( ExecuteScriptPlugin::executableEntry, executablePath->url() );
    cfg.writeEntry( ExecuteScriptPlugin::executeOnRemoteHostEntry, remoteHostCheckbox->isChecked() );
    cfg.writeEntry( ExecuteScriptPlugin::remoteHostEntry, remoteHost->text() );
    cfg.writeEntry( ExecuteScriptPlugin::runCurrentFileEntry, runCurrentFile->isChecked() );
    cfg.writeEntry( ExecuteScriptPlugin::argumentsEntry, arguments->text() );
    cfg.writeEntry( ExecuteScriptPlugin::workingDirEntry, workingDirectory->url() );
    cfg.writeEntry( ExecuteScriptPlugin::environmentGroupEntry, environment->currentProfile() );
    cfg.writeEntry( ExecuteScriptPlugin::outputFilteringEntry, outputFilteringMode->currentIndex() );
    //cfg.writeEntry( ExecuteScriptPlugin::useTerminalEntry, runInTerminal->isChecked() );
}

QString ScriptAppConfigPage::title() const
{
    return i18n("Configure Script Application");
}

QList< KDevelop::LaunchConfigurationPageFactory* > ScriptAppLauncher::configPages() const
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

const QString& ScriptAppLauncher::description() const
{
    static QString descr("Executes Script Applications");
    return descr;
}

const QString& ScriptAppLauncher::id() const
{
    static QString idstr("scriptAppLauncher");
    return idstr;
}

const QString& ScriptAppLauncher::name() const
{
    static QString namestr(i18n("Script Application"));
    return namestr;
}

ScriptAppLauncher::ScriptAppLauncher(ExecuteScriptPlugin* plugin)
: m_plugin( plugin )
{
}

ScriptAppJob* ScriptAppLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if( !cfg )
    {
        return 0;
    }
    if( launchMode == "execute" )
    {
        return new ScriptAppJob( m_plugin, cfg);
    }
    kWarning() << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return 0;
}

const QStringList& ScriptAppLauncher::supportedModes() const
{
    static QStringList modes([]() { return QStringList() << "execute"; }());
    return modes;
}

KDevelop::LaunchConfigurationPage* ScriptAppPageFactory::createWidget(QWidget* parent)
{
    return new ScriptAppConfigPage( parent );
}

ScriptAppPageFactory::ScriptAppPageFactory()
{
}

ScriptAppConfigType::ScriptAppConfigType()
: LaunchConfigurationType(i18n("Script Application"), ExecuteScriptPlugin::_scriptAppConfigTypeId)
{
    factoryList.append( new ScriptAppPageFactory() );
}

ScriptAppConfigType::~ScriptAppConfigType()
{
    qDeleteAll(factoryList);
    factoryList.clear();
}

const QList< KDevelop::LaunchConfigurationPageFactory* >& ScriptAppConfigType::configPages() const
{
    return factoryList;
}

KIcon ScriptAppConfigType::icon() const
{
    return KIcon("preferences-plugin-script");
}

bool ScriptAppConfigType::canLaunch(const KUrl& file) const
{
    return ! interpreterForUrl(file).isEmpty();
}

bool ScriptAppConfigType::canLaunch(KDevelop::ProjectBaseItem* item) const
{
    return ! interpreterForUrl(item->path().toUrl()).isEmpty();
}

void ScriptAppConfigType::configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const
{
    config.writeEntry(ExecuteScriptPlugin::executableEntry, item->path().toUrl());
    config.writeEntry(ExecuteScriptPlugin::interpreterEntry, interpreterForUrl(item->path().toUrl()));
    config.writeEntry(ExecuteScriptPlugin::outputFilteringEntry, 2u);
    config.writeEntry(ExecuteScriptPlugin::runCurrentFileEntry, false);
    config.sync();
}

void ScriptAppConfigType::configureLaunchFromCmdLineArguments(KConfigGroup cfg, const QStringList &args) const
{
    QStringList a(args);
    cfg.writeEntry( ExecuteScriptPlugin::interpreterEntry, a.takeFirst() );
    cfg.writeEntry( ExecuteScriptPlugin::executableEntry, a.takeFirst() );
    cfg.writeEntry( ExecuteScriptPlugin::argumentsEntry, KShell::joinArgs(a) );
    cfg.writeEntry( ExecuteScriptPlugin::runCurrentFileEntry, false );
    cfg.sync();
}

#include "scriptappconfig.moc"
