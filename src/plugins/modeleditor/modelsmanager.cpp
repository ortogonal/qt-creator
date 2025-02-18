// Copyright (C) 2016 Jochen Becher
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "modelsmanager.h"

#include "diagramsviewmanager.h"
#include "extdocumentcontroller.h"
#include "modeldocument.h"
#include "modeleditor_constants.h"
#include "modeleditor.h"
#include "modelindexer.h"
#include "pxnodecontroller.h"

#include "qmt/config/configcontroller.h"
#include "qmt/diagram_controller/dcontainer.h"
#include "qmt/diagram_controller/dreferences.h"
#include "qmt/diagram_scene/diagramscenemodel.h"
#include "qmt/diagram_ui/diagramsmanager.h"
#include "qmt/model/mdiagram.h"
#include "qmt/model_controller/modelcontroller.h"
#include "qmt/model_controller/mcontainer.h"
#include "qmt/model_controller/mreferences.h"
#include "qmt/project_controller/projectcontroller.h"
#include "qmt/project/project.h"
#include "qmt/stereotype/stereotypecontroller.h"
#include "qmt/tasks/diagramscenecontroller.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/icore.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/projecttree.h>
#include <utils/fileutils.h>

#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QAction>

namespace ModelEditor {
namespace Internal {

class ModelsManager::ManagedModel
{
public:
    ManagedModel() = default;
    ManagedModel(ExtDocumentController *m_documentController,ModelDocument *m_modelDocument);

    ExtDocumentController *m_documentController = nullptr;
    ModelDocument *m_modelDocument = nullptr;
};

ModelsManager::ManagedModel::ManagedModel(ExtDocumentController *documentController,
                                          ModelDocument *modelDocument)
    : m_documentController(documentController),
      m_modelDocument(modelDocument)
{
}

class ModelsManager::ModelsManagerPrivate
{
public:
    ~ModelsManagerPrivate()
    {
    }

    QList<ModelsManager::ManagedModel> managedModels;
    ModelIndexer *modelIndexer = nullptr;
    QList<Core::IDocument *> documentsToBeClosed;

    ExtDocumentController *modelClipboardDocumentController = nullptr;
    qmt::MContainer modelClipboard;
    ExtDocumentController *diagramClipboardDocumentController = nullptr;
    qmt::DContainer diagramClipboard;

    QAction *openDiagramContextMenuItem = nullptr;
    ProjectExplorer::Node *contextMenuOwnerNode = nullptr;
};

ModelsManager::ModelsManager(QObject *parent)
    : QObject(parent),
      d(new ModelsManagerPrivate())
{
    d->modelIndexer = new ModelIndexer(this);
#ifdef OPEN_DEFAULT_MODEL // disable feature - needs setting; does not work with qbs
    connect(d->modelIndexer, &ModelIndexer::openDefaultModel,
            this, &ModelsManager::onOpenDefaultModel, Qt::QueuedConnection);
#endif

    Core::Context projectTreeContext(ProjectExplorer::Constants::C_PROJECT_TREE);
    Core::ActionContainer *folderContainer = Core::ActionManager::actionContainer(
                ProjectExplorer::Constants::M_FOLDERCONTEXT);
    folderContainer->insertGroup(ProjectExplorer::Constants::G_FOLDER_FILES,
                                 Constants::EXPLORER_GROUP_MODELING);
    d->openDiagramContextMenuItem = new QAction(tr("Open Diagram"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(
                d->openDiagramContextMenuItem, Constants::ACTION_EXPLORER_OPEN_DIAGRAM,
                projectTreeContext);
    folderContainer->addAction(cmd, Constants::EXPLORER_GROUP_MODELING);
    connect(d->openDiagramContextMenuItem, &QAction::triggered,
            this, &ModelsManager::onOpenDiagramFromProjectExplorer);
    connect(ProjectExplorer::ProjectTree::instance(), &ProjectExplorer::ProjectTree::aboutToShowContextMenu,
            this, &ModelsManager::onAboutToShowContextMenu);
}

ModelsManager::~ModelsManager()
{
    QMT_CHECK(d->managedModels.isEmpty());
    delete d->modelIndexer;
    delete d;
}

ExtDocumentController *ModelsManager::createModel(ModelDocument *modelDocument)
{
    auto documentController = new ExtDocumentController(this);
    // TODO error output on reading definition files
    documentController->configController()->readStereotypeDefinitions(
        Core::ICore::resourcePath("modeleditor").toString());

    d->managedModels.append(ManagedModel(documentController, modelDocument));
    return documentController;
}

void ModelsManager::releaseModel(ExtDocumentController *documentController)
{
    if (documentController == d->modelClipboardDocumentController)
        d->modelClipboardDocumentController = nullptr;
    if (documentController == d->diagramClipboardDocumentController)
        d->diagramClipboardDocumentController = nullptr;
    for (int i = 0; i < d->managedModels.size(); ++i) {
        ManagedModel *managedModel = &d->managedModels[i];
        if (managedModel->m_documentController == documentController) {
            delete managedModel->m_documentController;
            d->managedModels.removeAt(i);
            return;
        }
    }
    QMT_CHECK(false);
}

void ModelsManager::openDiagram(const qmt::Uid &modelUid, const qmt::Uid &diagramUid)
{
    for (const ManagedModel &managedModel : std::as_const(d->managedModels)) {
        if (managedModel.m_documentController->projectController()->project()->uid() == modelUid) {
            qmt::MDiagram *diagram = managedModel.m_documentController->modelController()->findObject<qmt::MDiagram>(diagramUid);
            QMT_ASSERT(diagram, continue);
            openDiagram(managedModel.m_documentController, diagram);
            return;
        }
    }
}

bool ModelsManager::isModelClipboardEmpty() const
{
    return d->modelClipboard.isEmpty();
}

ExtDocumentController *ModelsManager::modelClipboardDocumentController() const
{
    return d->modelClipboardDocumentController;
}

qmt::MReferences ModelsManager::modelClipboard() const
{
    qmt::MReferences clipboard;
    clipboard.setElements(d->modelClipboard.elements());
    return clipboard;
}

void ModelsManager::setModelClipboard(ExtDocumentController *documentController, const qmt::MContainer &container)
{
    d->modelClipboardDocumentController = documentController;
    d->modelClipboard = container;
    emit modelClipboardChanged(isModelClipboardEmpty());
}

bool ModelsManager::isDiagramClipboardEmpty() const
{
    return d->diagramClipboard.isEmpty();
}

ExtDocumentController *ModelsManager::diagramClipboardDocumentController() const
{
    return d->diagramClipboardDocumentController;
}

qmt::DReferences ModelsManager::diagramClipboard() const
{
    qmt::DReferences clipboard;
    clipboard.setElements(d->diagramClipboard.elements());
    return clipboard;
}

void ModelsManager::setDiagramClipboard(ExtDocumentController *documentController, const qmt::DContainer &dcontainer, const qmt::MContainer &mcontainer)
{
    setModelClipboard(documentController, mcontainer);
    d->diagramClipboardDocumentController = documentController;
    d->diagramClipboard = dcontainer;
    emit diagramClipboardChanged(isDiagramClipboardEmpty());
}

void ModelsManager::onAboutToShowContextMenu(ProjectExplorer::Node *node)
{
    bool canOpenDiagram = false;

    for (const ManagedModel &managedModel : std::as_const(d->managedModels)) {
        if (managedModel.m_documentController->pxNodeController()->hasDiagramForExplorerNode(node)) {
            canOpenDiagram = true;
            break;
        }
    }

    if (canOpenDiagram)
        d->contextMenuOwnerNode = node;
    else
        d->contextMenuOwnerNode = nullptr;
    d->openDiagramContextMenuItem->setVisible(canOpenDiagram);
}

void ModelsManager::onOpenDiagramFromProjectExplorer()
{
    if (ProjectExplorer::ProjectTree::currentNode() == d->contextMenuOwnerNode) {
        qmt::MDiagram *diagram = nullptr;
        for (const ManagedModel &managedModel : std::as_const(d->managedModels)) {
            if ((diagram = managedModel.m_documentController->pxNodeController()->findDiagramForExplorerNode(d->contextMenuOwnerNode))) {
                openDiagram(managedModel.m_documentController, diagram);
                break;
            }
        }
    }
}

void ModelsManager::onOpenDefaultModel(const qmt::Uid &modelUid)
{
    const auto modelFile = Utils::FilePath::fromString(d->modelIndexer->findModel(modelUid));
    if (!modelFile.isEmpty())
        Core::EditorManager::openEditor(modelFile);
}

void ModelsManager::openDiagram(ExtDocumentController *documentController,
                                 qmt::MDiagram *diagram)
{
    for (const ManagedModel &managedModel : std::as_const(d->managedModels)) {
        if (managedModel.m_documentController == documentController) {
            Core::IEditor *editor = Core::EditorManager::activateEditorForDocument(managedModel.m_modelDocument);
            if (auto modelEditor = qobject_cast<ModelEditor *>(editor)) {
                modelEditor->showDiagram(diagram);
            }
            return;
        }
    }
}

} // namespace Internal
} // namespace ModelEditor
