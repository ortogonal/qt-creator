// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "ibconsolebuildstep.h"

#include "commandbuilderaspect.h"
#include "incredibuildconstants.h"

#include <projectexplorer/abstractprocessstep.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/gnumakeparser.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>

#include <utils/aspects.h>

using namespace ProjectExplorer;
using namespace Utils;

namespace IncrediBuild {
namespace Internal {

namespace Constants {
const QLatin1String IBCONSOLE_NICE("IncrediBuild.IBConsole.Nice");
const QLatin1String IBCONSOLE_COMMANDBUILDER("IncrediBuild.IBConsole.CommandBuilder");
const QLatin1String IBCONSOLE_KEEPJOBNUM("IncrediBuild.IBConsole.KeepJobNum");
const QLatin1String IBCONSOLE_FORCEREMOTE("IncrediBuild.IBConsole.ForceRemote");
const QLatin1String IBCONSOLE_ALTERNATE("IncrediBuild.IBConsole.Alternate");
}

class IBConsoleBuildStep final : public AbstractProcessStep
{
    Q_DECLARE_TR_FUNCTIONS(IncrediBuild::Internal::IBConsoleBuildStep)

public:
    IBConsoleBuildStep(BuildStepList *buildStepList, Id id);

    void setupOutputFormatter(OutputFormatter *formatter) final;
};

IBConsoleBuildStep::IBConsoleBuildStep(BuildStepList *buildStepList, Id id)
    : AbstractProcessStep(buildStepList, id)
{
    setDisplayName(tr("IncrediBuild for Linux"));

    addAspect<TextDisplay>("<b>" + tr("Target and Configuration"));

    auto commandBuilder = addAspect<CommandBuilderAspect>(this);
    commandBuilder->setSettingsKey(Constants::IBCONSOLE_COMMANDBUILDER);

    addAspect<TextDisplay>("<i>" + tr("Enter the appropriate arguments to your build command."));
    addAspect<TextDisplay>("<i>" + tr("Make sure the build command's "
                                      "multi-job parameter value is large enough (such as "
                                      "-j200 for the JOM or Make build tools)"));

    auto keepJobNum = addAspect<BoolAspect>();
    keepJobNum->setSettingsKey(Constants::IBCONSOLE_KEEPJOBNUM);
    keepJobNum->setLabel(tr("Keep original jobs number:"));
    keepJobNum->setToolTip(tr("Forces IncrediBuild to not override the -j command line switch, "
                              "that controls the number of parallel spawned tasks. The default "
                              "IncrediBuild behavior is to set it to 200."));

    addAspect<TextDisplay>("<b>" + tr("IncrediBuild Distribution Control"));

    auto nice = addAspect<IntegerAspect>();
    nice->setSettingsKey(Constants::IBCONSOLE_NICE);
    nice->setToolTip(tr("Specify nice value. Nice Value should be numeric and between -20 and 19"));
    nice->setLabel(tr("Nice value:"));
    nice->setRange(-20, 19);

    auto forceRemote = addAspect<BoolAspect>();
    forceRemote->setSettingsKey(Constants::IBCONSOLE_ALTERNATE);
    forceRemote->setLabel(tr("Force remote:"));

    auto alternate = addAspect<BoolAspect>();
    alternate->setSettingsKey(Constants::IBCONSOLE_FORCEREMOTE);
    alternate->setLabel(tr("Alternate tasks preference:"));

    setCommandLineProvider([=] {
        QStringList args;

        if (nice->value() != 0)
            args.append(QString("--nice %1 ").arg(nice->value()));

        if (alternate->value())
            args.append("--alternate");

        if (forceRemote->value())
            args.append("--force-remote");

        args.append(commandBuilder->fullCommandFlag(keepJobNum->value()));

        return CommandLine("ib_console", args);
    });
}

void IBConsoleBuildStep::setupOutputFormatter(OutputFormatter *formatter)
{
    formatter->addLineParser(new GnuMakeParser());
    formatter->addLineParsers(kit()->createOutputParsers());
    formatter->addSearchDir(processParameters()->effectiveWorkingDirectory());
    AbstractProcessStep::setupOutputFormatter(formatter);
}

// IBConsoleStepFactory

IBConsoleStepFactory::IBConsoleStepFactory()
{
    registerStep<IBConsoleBuildStep>(IncrediBuild::Constants::IBCONSOLE_BUILDSTEP_ID);
    setDisplayName(IBConsoleBuildStep::tr("IncrediBuild for Linux"));
    setSupportedStepLists({ProjectExplorer::Constants::BUILDSTEPS_BUILD,
                           ProjectExplorer::Constants::BUILDSTEPS_CLEAN});
}

} // namespace Internal
} // namespace IncrediBuild
