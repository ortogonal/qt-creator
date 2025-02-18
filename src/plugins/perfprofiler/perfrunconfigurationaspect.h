// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "perfprofiler_global.h"

#include <projectexplorer/runconfiguration.h>

#include <QObject>

namespace PerfProfiler {

class PERFPROFILER_EXPORT PerfRunConfigurationAspect :
        public ProjectExplorer::GlobalOrProjectAspect
{
public:
    PerfRunConfigurationAspect(ProjectExplorer::Target *target);
};

} // namespace PerfProfiler
