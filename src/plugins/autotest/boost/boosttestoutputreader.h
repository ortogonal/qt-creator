// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "../testoutputreader.h"

namespace Autotest {
namespace Internal {

class BoostTestResult;
enum class LogLevel;
enum class ReportLevel;

class BoostTestOutputReader : public TestOutputReader
{
    Q_OBJECT
public:
    BoostTestOutputReader(const QFutureInterface<TestResultPtr> &futureInterface,
                          Utils::QtcProcess *testApplication, const Utils::FilePath &buildDirectory,
                          const Utils::FilePath &projectFile, LogLevel log, ReportLevel report);
protected:
    void processOutputLine(const QByteArray &outputLine) override;
    void processStdError(const QByteArray &outputLine) override;
    TestResultPtr createDefaultResult() const override;

private:
    void onDone();
    void sendCompleteInformation();
    void handleMessageMatch(const QRegularExpressionMatch &match);
    void reportNoOutputFinish(const QString &description, ResultType type);
    Utils::FilePath m_projectFile;
    QString m_currentModule;
    QString m_currentSuite;
    QString m_currentTest;
    QString m_description;
    Utils::FilePath m_fileName;
    ResultType m_result = ResultType::Invalid;
    int m_lineNumber = 0;
    int m_testCaseCount = -1;
    LogLevel m_logLevel;
    ReportLevel m_reportLevel;
};

} // namespace Internal
} // namespace Autotest
