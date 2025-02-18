// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "ctestconfiguration.h"
#include "ctestoutputreader.h"

namespace Autotest {
namespace Internal {

CTestConfiguration::CTestConfiguration(ITestBase *testBase)
    : TestToolConfiguration(testBase)
{
    setDisplayName("CTest");
}

TestOutputReader *CTestConfiguration::outputReader(const QFutureInterface<TestResultPtr> &fi,
                                                   Utils::QtcProcess *app) const
{
    return new CTestOutputReader(fi, app, workingDirectory());
}

} // namespace Internal
} // namespace Autotest
