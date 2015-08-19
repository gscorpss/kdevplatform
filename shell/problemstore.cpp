/*
 * Copyright 2015 Laszlo Kis-Adam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "problemstore.h"
#include <shell/problem.h>
#include <shell/watcheddocumentset.h>
#include "problemstorenode.h"

struct ProblemStorePrivate
{
    ProblemStorePrivate()
        : m_documents(nullptr)
        , m_severity(KDevelop::IProblem::Hint)
        , m_rootNode(new KDevelop::ProblemStoreNode())
    {
    }

    // Watched document set. Only problems that are in files in this set are stored.
    KDevelop::WatchedDocumentSet *m_documents;

    // The severity filter setting
    int m_severity;

    // The problems list
    KDevelop::ProblemStoreNode *m_rootNode;

    // Path of the currently open document
    KDevelop::IndexedString m_currentDocument;
};

namespace KDevelop
{

ProblemStore::ProblemStore(QObject *parent)
    : QObject(parent),
      d(new ProblemStorePrivate)
{
}

ProblemStore::~ProblemStore()
{
    clear();
}

void ProblemStore::addProblem(const IProblem::Ptr &problem)
{
    ProblemNode *node = new ProblemNode(d->m_rootNode);
    node->setProblem(problem);
    d->m_rootNode->addChild(node);
}

void ProblemStore::setProblems(const QVector<IProblem::Ptr> &problems)
{
    clear();

    foreach (IProblem::Ptr problem, problems) {
        d->m_rootNode->addChild(new ProblemNode(d->m_rootNode, problem));
    }

    rebuild();
}

const ProblemStoreNode* ProblemStore::findNode(int row, ProblemStoreNode *parent) const
{
    Q_UNUSED(parent);
    return d->m_rootNode->child(row);
}

int ProblemStore::count(ProblemStoreNode *parent) const
{
    if(parent)
        return parent->count();
    else
        return d->m_rootNode->count();
}

void ProblemStore::clear()
{
    d->m_rootNode->clear();
}

void ProblemStore::rebuild()
{
}

void ProblemStore::setSeverity(int severity)
{
    if(severity != d->m_severity)
    {
        d->m_severity = severity;
        rebuild();
        emit changed();
    }
}

int ProblemStore::severity() const
{
    return d->m_severity;
}

WatchedDocumentSet* ProblemStore::documents() const
{
    return d->m_documents;
}

void ProblemStore::setScope(int scope)
{
    ProblemScope cast_scope = static_cast<ProblemScope>(scope);

    if (cast_scope == BypassScopeFilter) {
        setBypassScopeFilter(true);
        return;
    }

    setBypassScopeFilter(false);

    if (d->m_documents) {
        if(cast_scope == d->m_documents->getScope())
            return;

        delete d->m_documents;
    }

    switch (cast_scope) {
    case CurrentDocument:
        d->m_documents = new CurrentDocumentSet(d->m_currentDocument, this);
        break;
    case OpenDocuments:
        d->m_documents = new OpenDocumentSet(this);
        break;
    case CurrentProject:
        d->m_documents = new CurrentProjectSet(d->m_currentDocument, this);
        break;
    case AllProjects:
        d->m_documents = new AllProjectSet(this);
        break;
    }

    rebuild();

    connect(d->m_documents, &WatchedDocumentSet::changed, this, &ProblemStore::onDocumentSetChanged);

    emit changed();
}

void ProblemStore::setGrouping(int grouping)
{
    Q_UNUSED(grouping);
}

void ProblemStore::setBypassScopeFilter(bool bypass)
{
    Q_UNUSED(bypass);
}

void ProblemStore::setCurrentDocument(const IndexedString &doc)
{
    d->m_currentDocument = doc;
    d->m_documents->setCurrentDocument(doc);
}


const KDevelop::IndexedString& ProblemStore::currentDocument() const
{
    return d->m_currentDocument;
}


void ProblemStore::onDocumentSetChanged()
{
    rebuild();

    emit changed();
}

ProblemStoreNode* ProblemStore::rootNode()
{
    return d->m_rootNode;
}

}
