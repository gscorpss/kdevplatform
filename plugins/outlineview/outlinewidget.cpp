/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "outlinewidget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QLineEdit>
#include <QPushButton>
#include <QIcon>

#include <KLocalizedString>
#include <KRecursiveFilterProxyModel>

#include <interfaces/idocumentcontroller.h>

#include "outlineviewplugin.h"
#include "outlinemodel.h"

using namespace KDevelop;

OutlineWidget::OutlineWidget(QWidget* parent, OutlineViewPlugin* plugin)
    : QWidget(parent)
    , m_plugin(plugin)
    , m_model(new OutlineModel(this))
    , m_tree(new QTreeView(this))
    , m_proxy(new KRecursiveFilterProxyModel(this))
    , m_filter(new QLineEdit(this))
{
    setObjectName(QStringLiteral("Outline View"));
    setWindowTitle(i18n("Outline"));
    setWhatsThis(i18n("Outline View"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("code-class"), windowIcon())); //TODO: better icon?

    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setDynamicSortFilter(false);

    m_tree->setModel(m_proxy);
    m_tree->setHeaderHidden(true);

    //filter
    connect(m_filter, &QLineEdit::textChanged, m_proxy, &KRecursiveFilterProxyModel::setFilterFixedString);
    connect(m_tree, &QTreeView::activated, this, &OutlineWidget::activated);

    QHBoxLayout* filterLayout = new QHBoxLayout();

    m_filter->setPlaceholderText(i18n("Filter..."));
    m_sortAlphabetically = new QPushButton(QIcon::fromTheme(QStringLiteral("view-sort-ascending")), QString(), this);
    m_sortAlphabetically->setToolTip(i18n("Sort alphabetically"));
    m_sortAlphabetically->setCheckable(true);
    connect(m_sortAlphabetically, &QPushButton::toggled, this, [this](bool sort) {
        // calling sort with -1 will restore the original order
        m_proxy->sort(sort ? 0 : -1, Qt::AscendingOrder);
        m_sortAlphabetically->setChecked(sort);
    });

    filterLayout->addWidget(m_sortAlphabetically);
    filterLayout->addWidget(m_filter);
    setFocusProxy(m_filter);

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    vbox->addLayout(filterLayout);
    vbox->addWidget(m_tree);
    setLayout(vbox);
    expandFirstLevel();
    connect(m_model, &QAbstractItemModel::modelReset, this, &OutlineWidget::expandFirstLevel);
}

void OutlineWidget::activated(QModelIndex index)
{
    QModelIndex realIndex = m_proxy->mapToSource(index);
    m_model->activate(realIndex);
}

OutlineWidget::~OutlineWidget()
{
}

void OutlineWidget::expandFirstLevel()
{
    for (int i = 0; i < m_proxy->rowCount(); i++) {
        m_tree->expand(m_proxy->index(i, 0));
    }
}
