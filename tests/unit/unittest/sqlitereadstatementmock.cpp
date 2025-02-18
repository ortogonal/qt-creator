// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "sqlitereadstatementmock.h"

#include "sqlitedatabasemock.h"

SqliteReadStatementMockBase::SqliteReadStatementMockBase(Utils::SmallStringView sqlStatement,
                                                         SqliteDatabaseMock &databaseMock)
    : sqlStatement(sqlStatement)
{
    databaseMock.prepare(sqlStatement);
}
