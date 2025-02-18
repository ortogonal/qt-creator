// Copyright (C) 2018 Sergey Morozov
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "cppcheckconstants.h"
#include "cppcheckdiagnostic.h"
#include "cppchecktextmark.h"

#include <utils/stringutils.h>
#include <utils/utilsicons.h>

#include <QAction>
#include <QMap>

namespace Cppcheck {
namespace Internal {

struct Visual
{
    Visual(Utils::Theme::Color color, TextEditor::TextMark::Priority priority,
           const QIcon &icon)
        : color(color),
          priority(priority),
          icon(icon)
    {}
    Utils::Theme::Color color;
    TextEditor::TextMark::Priority priority;
    QIcon icon;
};

static Visual getVisual(Diagnostic::Severity type)
{
    using Color = Utils::Theme::Color;
    using Priority = TextEditor::TextMark::Priority;

    static const QMap<Diagnostic::Severity, Visual> visuals{
        {Diagnostic::Severity::Error, {Color::IconsErrorColor, Priority::HighPriority,
                        Utils::Icons::CRITICAL.icon()}},
        {Diagnostic::Severity::Warning, {Color::IconsWarningColor, Priority::NormalPriority,
                        Utils::Icons::WARNING.icon()}},
    };

    return visuals.value(type, {Color::IconsInfoColor, Priority::LowPriority,
                                Utils::Icons::INFO.icon()});
}

CppcheckTextMark::CppcheckTextMark (const Diagnostic &diagnostic)
    : TextEditor::TextMark (diagnostic.fileName, diagnostic.lineNumber,
                            Utils::Id(Constants::TEXTMARK_CATEGORY_ID)),
    m_severity(diagnostic.severity),
    m_checkId(diagnostic.checkId),
    m_message(diagnostic.message)
{
    const Visual visual = getVisual(diagnostic.severity);
    setPriority(visual.priority);
    setColor(visual.color);
    setIcon(visual.icon);
    setToolTip(toolTipText(diagnostic.severityText));
    setLineAnnotation(diagnostic.message);
    setSettingsPage(Constants::OPTIONS_PAGE_ID);
    setActionsProvider([diagnostic] {
        // Copy to clipboard action
        QAction *action = new QAction;
        action->setIcon(QIcon::fromTheme("edit-copy", Utils::Icons::COPY.icon()));
        action->setToolTip(TextMark::tr("Copy to Clipboard"));
        QObject::connect(action, &QAction::triggered, [diagnostic]() {
            const QString text = QString("%1:%2: %3")
                    .arg(diagnostic.fileName.toUserOutput())
                    .arg(diagnostic.lineNumber)
                    .arg(diagnostic.message);
            Utils::setClipboardAndSelection(text);
        });
        return QList<QAction *>{action};
    });
}

QString CppcheckTextMark::toolTipText(const QString &severityText) const
{
    return QString(
                "<table cellspacing='0' cellpadding='0' width='100%'>"
                "  <tr>"
                "    <td align='left'><b>Cppcheck</b></td>"
                "    <td align='right'>&nbsp;<font color='gray'>%1: %2</font></td>"
                "  </tr>"
                "  <tr>"
                "    <td colspan='2' align='left' style='padding-left:10px'>%3</td>"
                "  </tr>"
                "</table>").arg(m_checkId, severityText, m_message);
}

} // namespace Internal
} // namespace Cppcheck
