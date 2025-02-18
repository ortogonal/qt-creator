// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "vcsbase_global.h"

namespace VcsBase {

enum class RunFlags {
    None                   = 0,        // Empty.
    // QtcProcess related
    MergeOutputChannels    = (1 <<  0), // See QProcess::ProcessChannelMode::MergedChannels.
    ForceCLocale           = (1 <<  1), // Force C-locale, sets LANG and LANGUAGE env vars to "C".
    UseEventLoop           = (1 <<  2), // Use event loop when executed in UI thread with
                                        // runBlocking().
    // Decorator related
    SuppressStdErr         = (1 <<  3), // Suppress standard error output.
    SuppressFailMessage    = (1 <<  4), // No message about command failure.
    SuppressCommandLogging = (1 <<  5), // No starting command log entry.
    ShowSuccessMessage     = (1 <<  6), // Show message about successful completion of command.
    ShowStdOut             = (1 <<  7), // Show standard output.
    SilentOutput           = (1 <<  8), // Only when ShowStdOut is set to true, outputs silently.
    ProgressiveOutput      = (1 <<  9), // Emit stdOutText() and stdErrText() signals.
    ExpectRepoChanges      = (1 << 10), // Expect changes in repository by the command.
    NoOutput               = SuppressStdErr | SuppressFailMessage | SuppressCommandLogging
};

inline void VCSBASE_EXPORT operator|=(RunFlags &p, RunFlags r)
{
    p = RunFlags(int(p) | int(r));
}

inline RunFlags VCSBASE_EXPORT operator|(RunFlags p, RunFlags r)
{
    return RunFlags(int(p) | int(r));
}

inline void VCSBASE_EXPORT operator&=(RunFlags &p, RunFlags r)
{
    p = RunFlags(int(p) & int(r));
}

// Note, that it returns bool, not RunFlags.
// It's only meant for testing whether a specific bit is set.
inline bool VCSBASE_EXPORT operator&(RunFlags p, RunFlags r)
{
    return bool(int(p) & int(r));
}

} // namespace VcsBase
