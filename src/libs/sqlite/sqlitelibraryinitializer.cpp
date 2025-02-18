// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "sqlitelibraryinitializer.h"

#include "sqlitedatabasebackend.h"

namespace Sqlite {

void LibraryInitializer::initialize()
{
    static LibraryInitializer initializer;
}

LibraryInitializer::LibraryInitializer()
{
    DatabaseBackend::initializeSqliteLibrary();
}

LibraryInitializer::~LibraryInitializer()
{
    DatabaseBackend::shutdownSqliteLibrary();
}

} // namespace Sqlite
