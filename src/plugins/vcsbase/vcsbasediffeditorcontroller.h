// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "vcsbase_global.h"
#include "vcsenums.h"

#include <diffeditor/diffeditorcontroller.h>

QT_BEGIN_NAMESPACE
class QTextCodec;
QT_END_NAMESPACE

namespace Core { class IDocument; }

namespace Utils {
class Environment;
class FilePath;
} // Utils

namespace VcsBase {

class VcsBaseDiffEditorControllerPrivate;

class VCSBASE_EXPORT VcsBaseDiffEditorController : public DiffEditor::DiffEditorController
{
    Q_OBJECT

public:
    explicit VcsBaseDiffEditorController(Core::IDocument *document);
    ~VcsBaseDiffEditorController() override;

    void setProcessEnvironment(const Utils::Environment &value);
    void setVcsBinary(const Utils::FilePath &path);
    void setVcsTimeoutS(int value);
    void setWorkingDirectory(const Utils::FilePath &workingDir);

protected:
    void runCommand(const QList<QStringList> &args, RunFlags flags, QTextCodec *codec = nullptr);
    virtual void processCommandOutput(const QString &output);

    Utils::FilePath workingDirectory() const;
    void setStartupFile(const QString &startupFile);
    QString startupFile() const;
    void setDisplayName(const QString &displayName);

private:
    friend class VcsBaseDiffEditorControllerPrivate;
    VcsBaseDiffEditorControllerPrivate *d;
};

} //namespace VcsBase
