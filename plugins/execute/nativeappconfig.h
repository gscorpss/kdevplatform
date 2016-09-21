/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

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

#ifndef KDEVPLATFORM_PLUGIN_NATIVEAPPCONFIGTYPE_H
#define KDEVPLATFORM_PLUGIN_NATIVEAPPCONFIGTYPE_H

#include <interfaces/launchconfigurationtype.h>
#include <interfaces/launchconfigurationpage.h>
#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchmode.h>
#include <util/executecompositejob.h>
#include "ui_nativeappconfig.h"

//TODO: Split the page into two, one concerning executable/arguments/behaviour the other for dependencies

class NativeAppConfigPage : public KDevelop::LaunchConfigurationPage, Ui::NativeAppPage
{
Q_OBJECT
public:
    NativeAppConfigPage( QWidget* parent );
    void loadFromConfiguration( const KConfigGroup& cfg, KDevelop::IProject* project = 0 );
    void saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project = 0 ) const;
    QString title() const;
    KIcon icon() const;
private slots:
    void addDep();
    void removeDep();
    void moveDependencyUp();
    void moveDependencyDown();
    void activateDeps( int );
    void checkActions( const QItemSelection& , const QItemSelection& );
    void depEdited( const QString& );
    void selectItemDialog();
};

class NativeAppLauncher : public KDevelop::ILauncher
{
public:
    NativeAppLauncher();
    virtual QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const;
    const QString& description() const override;
    const QString& id() const override;
    const QString& name() const override;
    KDevelop::ExecuteCompositeJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg) override;
    const QStringList& supportedModes() const override;
};

class NativeAppPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    NativeAppPageFactory();
    virtual KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent);
};

/**
 * A specific configuration to start a launchable, this could be a native
 * compiled application, or some script file or byte-compiled file or something else
 * Provides access to the various configured informations, as well as its type and a name
 */
class NativeAppConfigType : public KDevelop::LaunchConfigurationType
{
Q_OBJECT
public:
    NativeAppConfigType();
    virtual ~NativeAppConfigType();

    const QString& id() const;
    const QString& name() const;
    const QList<KDevelop::LaunchConfigurationPageFactory*>& configPages() const;
    KIcon icon() const;
    bool canLaunch( KDevelop::ProjectBaseItem* item ) const;
    bool canLaunch( const KUrl& file ) const;
    void configureLaunchFromItem ( KConfigGroup cfg, 
                                   KDevelop::ProjectBaseItem* item ) const;
    void configureLaunchFromCmdLineArguments ( KConfigGroup cfg,
                                               const QStringList& args ) const;
    QMenu* launcherSuggestions();
private:
    QList<KDevelop::LaunchConfigurationPageFactory*> factoryList;

public slots:
    void suggestionTriggered();
};
#endif

