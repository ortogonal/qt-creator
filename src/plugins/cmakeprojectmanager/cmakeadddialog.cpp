#include "cmakeadddialog.h"
#include "cmakedocument.h"
#include "cmakeeditor.h"
#include "ui_cmakeadddialog.h"

#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/editormanager/ieditorfactory.h>

#include <texteditor/textdocument.h>

#include <QAbstractListModel>
#include <QDebug>
#include <QItemSelectionModel>
#include <QTextDocument>

namespace CMakeProjectManager {
namespace Internal {

class CMakeAddLocationAlternativesModel : public QAbstractListModel
{
public:
    CMakeAddLocationAlternativesModel(std::shared_ptr<CMakeDocument> document,
                                          QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
private:
    QVariant checkBoxColumn(const QModelIndex &index, int role) const;
    QVariant titleColumn(const QModelIndex &index, int role) const;
    QVariant locationColumn(const QModelIndex &index, int role) const;

    std::shared_ptr<CMakeDocument> m_document;
    QSet<int> m_locationsToUse;
};

CMakeAddLocationAlternativesModel::CMakeAddLocationAlternativesModel(std::shared_ptr<CMakeDocument> document, QObject *parent)
    : QAbstractListModel(parent)
    , m_document(document)
{
}

int CMakeAddLocationAlternativesModel::rowCount(const QModelIndex &) const
{
    return m_document->noSourceLocations();
}

int CMakeAddLocationAlternativesModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant CMakeAddLocationAlternativesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < m_document->noSourceLocations() && index.column() < columnCount(index)) {
        if (index.column() == 0)
            return checkBoxColumn(index, role);
        else if (index.column() == 1)
            return titleColumn(index, role);
        else if (index.column() == 2)
            return locationColumn(index, role);
    }

    return QVariant();
}

Qt::ItemFlags CMakeAddLocationAlternativesModel::flags(const QModelIndex &index) const
{
    if (index.column() == 0) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    }
    return QAbstractItemModel::flags(index);
}

QVariant CMakeAddLocationAlternativesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Add to");
        else if (section == 1)
            return tr("CMake target type");
        else if (section == 2)
            return tr("Location");
    }
    return QVariant();
}

bool CMakeAddLocationAlternativesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() == 0 && role == Qt::CheckStateRole) {
        if (value.toInt() == 0) {
            m_locationsToUse.remove(index.row());
        } else {
            m_locationsToUse.insert(index.row());
        }
    }
    return true;
}

QVariant CMakeAddLocationAlternativesModel::checkBoxColumn(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole) {
        if (m_locationsToUse.contains(index.row()))
            return Qt::Checked;
        else
            return Qt::Unchecked;
    }

    return QVariant();
}

QVariant CMakeAddLocationAlternativesModel::titleColumn(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        auto l = m_document->sourceLocation(index.row());
        return CMakeDocument::sourceLocationTypeString(l.type);
    }
    return QVariant();
}

QVariant CMakeAddLocationAlternativesModel::locationColumn(const QModelIndex &index, int role) const
{
    auto l = m_document->sourceLocation(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return QString("Line: %1:%2").arg(l.location.line()).arg(l.location.column());
    }

    return QVariant();
}

CMakeAddDialog::CMakeAddDialog(std::shared_ptr<CMakeDocument> document, QWidget *parent)
    : QDialog(parent)
    , m_document(document)
    , m_addAlternativesModel(new CMakeAddLocationAlternativesModel(document, this))
    , m_ui(new Ui::CMakeAddDialog)
{
    m_ui->setupUi(this);
    m_ui->tableView->setModel(m_addAlternativesModel);
    m_ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    connect(m_ui->tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, [&](const QItemSelection &index, const QItemSelection &) {
                if (!index.indexes().isEmpty())
                   m_editor->gotoLine(m_document->sourceLocation(index.indexes().first().row()).location.line());
            });

    auto factories = Core::IEditorFactory::preferredEditorFactories("CMakeLists.txt");

    if (!factories.isEmpty()) {
        const auto factory = factories.first();
        m_editor = factory->createEditor();

        QString errorString;
        m_editor->document()->open(&errorString,
                                   Utils::FilePath::fromString(document->cmakeFile()),
                                   Utils::FilePath::fromString(document->cmakeFile()));
        auto editorWidget = qobject_cast<TextEditor::TextEditorWidget *>(m_editor->widget());

        if (editorWidget) {
            editorWidget->setReadOnly(true);
        }

        m_ui->verticalLayout->insertWidget(3, m_editor->widget());
    } else {
        // TODO: Can we asume there will always be an editor, I guess so...
    }
}

CMakeAddDialog::~CMakeAddDialog()
{
    delete m_ui;
}

}
}
