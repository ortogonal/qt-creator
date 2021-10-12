#pragma once

#include <QDialog>
#include <memory>

namespace Ui {
class CMakeAddDialog;
}

namespace Core {
class IEditor;
}

namespace CMakeProjectManager {
namespace Internal {

class CMakeDocument;
class CMakeAddLocationAlternativesModel;

class CMakeAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CMakeAddDialog(std::shared_ptr<CMakeDocument> document, QWidget *parent = nullptr);
    ~CMakeAddDialog();

private:
    std::shared_ptr<CMakeDocument> m_document;
    CMakeAddLocationAlternativesModel *m_addAlternativesModel = nullptr;

    Core::IEditor *m_editor = nullptr;
    Ui::CMakeAddDialog *m_ui = nullptr;
};

}
}
