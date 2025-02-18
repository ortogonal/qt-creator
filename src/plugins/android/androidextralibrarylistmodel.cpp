// Copyright (C) 2016 BogDan Vatra <bog_dan_ro@yahoo.com>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "androidextralibrarylistmodel.h"

#include <android/androidconstants.h>
#include <android/androidmanager.h>

#include <projectexplorer/buildsystem.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/target.h>

#include <utils/qtcassert.h>

using namespace ProjectExplorer;

namespace Android {

AndroidExtraLibraryListModel::AndroidExtraLibraryListModel(BuildSystem *buildSystem,
                                                           QObject *parent)
    : QAbstractItemModel(parent),
      m_buildSystem(buildSystem)
{
    updateModel();

    connect(buildSystem, &BuildSystem::parsingStarted,
            this, &AndroidExtraLibraryListModel::updateModel);
    connect(buildSystem, &BuildSystem::parsingFinished,
            this, &AndroidExtraLibraryListModel::updateModel);
    // Causes target()->activeBuildKey() result and consequently the node data
    // extracted below to change.
    connect(buildSystem->target(), &Target::activeRunConfigurationChanged,
            this, &AndroidExtraLibraryListModel::updateModel);
}

QModelIndex AndroidExtraLibraryListModel::index(int row, int column, const QModelIndex &) const
{
    return createIndex(row, column);
}

QModelIndex AndroidExtraLibraryListModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int AndroidExtraLibraryListModel::rowCount(const QModelIndex &) const
{
    return m_entries.size();
}

int AndroidExtraLibraryListModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant AndroidExtraLibraryListModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.row() >= 0 && index.row() < m_entries.size());
    if (role == Qt::DisplayRole)
        return QDir::cleanPath(m_entries.at(index.row()));
    return {};
}

void AndroidExtraLibraryListModel::updateModel()
{
    const QString buildKey = m_buildSystem->target()->activeBuildKey();
    const ProjectNode *node = m_buildSystem->target()->project()->findNodeForBuildKey(buildKey);
    if (!node)
        return;

    if (node->parseInProgress()) {
        emit enabledChanged(false);
        return;
    }

    bool enabled;
    beginResetModel();
    if (node->validParse()) {
        m_entries = node->data(Constants::AndroidExtraLibs).toStringList();
        enabled = true;
    } else {
        // parsing error
        m_entries.clear();
        enabled = false;
    }
    endResetModel();

    emit enabledChanged(enabled);
}

void AndroidExtraLibraryListModel::addEntries(const QStringList &list)
{
    const QString buildKey = m_buildSystem->target()->activeBuildKey();
    const ProjectNode *node = m_buildSystem->target()->project()->findNodeForBuildKey(buildKey);
    QTC_ASSERT(node, return);

    beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size() + list.size());

    const QDir dir = node->filePath().toFileInfo().absoluteDir();
    for (const QString &path : list)
        m_entries += "$$PWD/" + dir.relativeFilePath(path);

    m_buildSystem->setExtraData(buildKey, Constants::AndroidExtraLibs, m_entries);
    endInsertRows();
}

bool greaterModelIndexByRow(const QModelIndex &a, const QModelIndex &b)
{
    return a.row() > b.row();
}

void AndroidExtraLibraryListModel::removeEntries(QModelIndexList list)
{
    if (list.isEmpty())
        return;

    std::sort(list.begin(), list.end(), greaterModelIndexByRow);

    int i = 0;
    while (i < list.size()) {
        int lastRow = list.at(i++).row();
        int firstRow = lastRow;
        while (i < list.size() && firstRow - list.at(i).row()  <= 1)
            firstRow = list.at(i++).row();

        beginRemoveRows(QModelIndex(), firstRow, lastRow);
        int count = lastRow - firstRow + 1;
        while (count-- > 0)
            m_entries.removeAt(firstRow);
        endRemoveRows();
    }

    const QString buildKey = m_buildSystem->target()->activeBuildKey();
    m_buildSystem->setExtraData(buildKey, Constants::AndroidExtraLibs, m_entries);
}

} // Android
