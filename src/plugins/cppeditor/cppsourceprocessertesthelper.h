// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <QtGlobal>
#include <QString>

namespace CppEditor::Tests::Internal {

class TestIncludePaths
{
    Q_DISABLE_COPY(TestIncludePaths)

public:
    static QString includeBaseDirectory();
    static QString globalQtCoreIncludePath();
    static QString globalIncludePath();
    static QString directoryOfTestFile();
    static QString testFilePath(const QString &fileName = QLatin1String("file.cpp"));
};

} // namespace CppEditor::Tests::Internal
