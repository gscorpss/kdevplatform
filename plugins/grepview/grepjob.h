/***************************************************************************
 *   Copyright 1999-2001 by Bernd Gehrmann                                 *
 *   bernd@kdevelop.org                                                    *
 *   Copyright 2008 by Hamish Rodda                                        *
 *   rodda@kde.org                                                         *
 *   Copyright 2010 Silvère Lestang <silvere.lestang@gmail.com>            *
 *   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_GREPJOB_H
#define KDEVPLATFORM_PLUGIN_GREPJOB_H

#include <QPointer>
#include <QUrl>

#include <KJob>

#include <interfaces/istatus.h>

#include "grepfindthread.h"
#include "grepoutputmodel.h"

namespace KDevelop
{
    class IProject;
    class ProcessLineMaker;
}

class QRegExp;
class GrepViewPlugin;
class FindReplaceTest; //FIXME: this is useful only for tests

class GrepJob : public KJob, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IStatus )

    friend class GrepViewPlugin;
    friend class FindReplaceTest;

private:
    ///Job can only be instanciated by plugin
    explicit GrepJob( QObject *parent = nullptr );

public:

    void setOutputModel(GrepOutputModel * model);
    void setPatternString(const QString& patternString);
    void setTemplateString(const QString &templateString);
    void setReplacementTemplateString(const QString &replTmplString);
    void setFilesString(const QString &filesString);
    void setExcludeString(const QString &excludeString);
    void setDirectoryChoice(const QList<QUrl> &choice);
    void setDepth(int depth);
    void setRegexpFlag(bool regexpFlag);
    void setCaseSensitive(bool caseSensitive);
    void setProjectFilesFlag(bool projectFilesFlag);

    void start() override;

    QString statusName() const override;
protected:
    bool doKill() override;

//    GrepOutputModel* model() const;

private Q_SLOTS:
    void slotFindFinished();
    void testFinishState(KJob *job);

Q_SIGNALS:
    void clearMessage( KDevelop::IStatus* ) override;
    void showMessage( KDevelop::IStatus*, const QString & message, int timeout = 0) override;
    void showErrorMessage(const QString & message, int timeout = 0) override;
    void hideProgress( KDevelop::IStatus* ) override;
    void showProgress( KDevelop::IStatus*, int minimum, int maximum, int value) override;
    void foundMatches( const QString& filename, const GrepOutputItem::List& matches);

private:
    Q_INVOKABLE void slotWork();

    QString m_patternString;
    QRegExp m_regExp;
    QString m_regExpSimple;
    GrepOutputModel *m_outputModel;

    enum {
        WorkCollectFiles,
        WorkGrep,
        WorkIdle,
        WorkCancelled
    } m_workState;

    QList<QUrl> m_fileList;
    int m_fileIndex;
    QPointer<GrepFindFilesThread> m_findThread;

    QString m_errorMessage;
    QString m_templateString;
    QString m_replacementTemplateString;
    QString m_filesString;
    QString m_excludeString;
    QList<QUrl> m_directoryChoice;

    bool m_useProjectFilesFlag;
    bool m_regexpFlag;
    bool m_caseSensitiveFlag;
    int m_depthValue;

    bool m_findSomething;
};

//FIXME: this function is used externally only for tests, find a way to keep it
//       static for a regular compilation
GrepOutputItem::List grepFile(const QString &filename, const QRegExp &re);

#endif
