#include "cmakedocument.h"

#include "lexer/cmListFileLexer.h"

#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTextStream>

namespace CMakeProjectManager {
namespace Internal {

CMakeLocation::CMakeLocation() {}

CMakeLocation::CMakeLocation(int line, int column)
    : m_line(line)
    , m_column(column)
{}

bool CMakeLocation::valid() const
{
    return m_line > -1 && m_column > -1;
}

int CMakeLocation::line() const
{
    return m_line;
}

int CMakeLocation::column() const
{
    return m_column;
}

bool CMakeLocation::operator==(const CMakeLocation &rhs) const
{
    return (valid() && rhs.valid() && m_line == rhs.m_line && m_column == rhs.m_column);
}

bool CMakeLocation::operator!=(const CMakeLocation &rhs) const
{
    return !(*this == rhs);
}

bool CMakeLocation::operator<(const CMakeLocation &rhs) const
{
    if (!valid() || !rhs.valid())
        return false;

    if (m_line < rhs.m_line)
        return true;
    if (m_line == rhs.m_line && m_column < rhs.m_column)
        return true;

    return false;
}

bool CMakeLocation::operator<=(const CMakeLocation &rhs) const
{
    if (!valid() || !rhs.valid())
        return false;

    if (m_line < rhs.m_line)
        return true;
    if (m_line == rhs.m_line && m_column <= rhs.m_column)
        return true;

    return false;
}

bool CMakeLocation::operator>(const CMakeLocation &rhs) const
{
    if (!valid() || !rhs.valid())
        return false;

    if (m_line > rhs.m_line)
        return true;
    if (m_line == rhs.m_line && m_column > rhs.m_column)
        return true;

    return false;
}

bool CMakeLocation::operator>=(const CMakeLocation &rhs) const
{
    if (!valid() || !rhs.valid())
        return false;

    if (m_line > rhs.m_line)
        return true;
    if (m_line == rhs.m_line && m_column >= rhs.m_column)
        return true;

    return false;
}

QDebug operator<<(QDebug debug, const CMakeLocation &l)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << l.line() << ", " << l.column() << ')';

    return debug;
}

CMakeArgument::CMakeArgument(const QString &name)
    : m_name(name)
    , m_quotes(false)
{}

CMakeArgument::CMakeArgument(const QString &name, bool quotes, const CMakeLocation &startLocation)
    : m_name(name)
    , m_quotes(quotes)
    , m_startLocation(startLocation)
{}

QString CMakeArgument::name() const
{
    return m_name;
}

bool CMakeArgument::quotes() const
{
    return m_quotes;
}

CMakeLocation CMakeArgument::startLocation() const
{
    return m_startLocation;
}

CMakeFunction::CMakeFunction(const QString &name, const CMakeLocation &startLocation)
    : m_name(name)
    , m_startLocation(startLocation)
{}

QString CMakeFunction::name() const
{
    return m_name;
}

CMakeLocation CMakeFunction::startLocation() const
{
    return m_startLocation;
}

CMakeLocation CMakeFunction::endLocation() const
{
    return m_endLocation;
}

void CMakeFunction::setEndLocation(const CMakeLocation &endLocation)
{
    m_endLocation = endLocation;
}

void CMakeFunction::appendArgument(const CMakeArgument &argument)
{
    m_arguments.append(argument);
}

QString CMakeFunction::firstArgumemt() const
{
    return m_arguments.first().name();
}

QString CMakeFunction::data() const
{
    QString data = m_name + "(";

    bool firstArg = true;
    int indentation = 0;
    for (const auto &arg : m_arguments) {

        if (firstArg) {
            if (arg.startLocation().column() > 0) {
                indentation = arg.startLocation().column() - 1;
            }
        } else {
            QString indentationStr;
            data += "\n" + indentationStr.leftJustified(indentation, ' ');
        }

        if (arg.quotes())
            data += "\"";

        data += arg.name();

        if (arg.quotes())
            data += "\"";

        firstArg = false;
    }

    data += ")";

    return data;
}

void CMakeFunction::dumpArguments()
{
    for (const auto &arg : m_arguments) {
        qDebug() << "  - " << arg.name();
    }
}

CMakeDocument::CMakeDocument(const QString &cmakeFile)
    : m_cmakeFile(cmakeFile)
{
    m_fileParsed = parse(cmakeFile);
}

bool CMakeDocument::valid() const
{
    return m_fileParsed;
}

QString CMakeDocument::cmakeFile() const
{
    return m_cmakeFile;
}

/**
   * @brief addSource
   * @param sources
   * @return
   *
   * Currently only adding to PROJECT_SOURCES
   */
bool CMakeDocument::addSource(const QStringList sources)
{
    for (const auto &func : m_functions) {
        if (func->name().toLower() == QStringLiteral("set")
            && func->firstArgumemt() == "PROJECT_SOURCES") {
            for (const auto &str : sources) {
                func->appendArgument(str);
            }

            qDebug() << Q_FUNC_INFO << func->startLocation() << func->endLocation();
            qDebug() << func->data();

            return modify(m_cmakeFile, func->startLocation(), func->endLocation(), func->data());
        }
    }
    return false;
}

bool CMakeDocument::uniqueSourceLocation() const
{
    return noSourceLocations() == 1;
}

int CMakeDocument::noSourceLocations() const
{
    int locations = 0;
    for (const auto &func : m_functions) {
        if (func->name() == QStringLiteral("add_executable")
            || func->name() == QStringLiteral("add_library")
            || func->name() == QStringLiteral("qt_add_executable")) {
            locations++;
        } else if (func->name().toLower() == QStringLiteral("set")
                   && func->firstArgumemt() == QStringLiteral("PROJECT_SOURCES")) {
            locations++;
        }
    }

    return locations;
}

CMakeDocument::SourceLocationLocation CMakeDocument::sourceLocation(int position)
{
    int location = 0;
    for (const auto &func : m_functions) {
        SourceLocationType lastType = SourceLocationType::None;

        if (func->name() == QStringLiteral("add_executable")) {
            lastType = SourceLocationType::AddExecutable;
        } else if (func->name() == QStringLiteral("add_library")) {
            lastType = SourceLocationType::AddLibrary;
        } else if (func->name() == QStringLiteral("qt_add_executable")) {
            lastType = SourceLocationType::QtAddEexecutable;
        } else if (func->name() == QStringLiteral("set")
                   && func->firstArgumemt() == QStringLiteral("PROJECT_SOURCES")) {
            lastType = SourceLocationType::Set;
        }

        if (lastType != SourceLocationType::None) {
            if (location == position) {
                return {lastType, func->startLocation()};
            }
            location++;
        }
    }

    return { SourceLocationType::None, CMakeLocation()};
}

QString CMakeDocument::sourceLocationTypeString(CMakeDocument::SourceLocationType type)
{
    switch (type) {
    case CMakeDocument::SourceLocationType::Set:
        return QStringLiteral("set(PROJECT_SOURCES");
    case CMakeDocument::SourceLocationType::AddExecutable:
        return QStringLiteral("add_executable");
    case CMakeDocument::SourceLocationType::AddLibrary:
        return QStringLiteral("add_library");
    case CMakeDocument::SourceLocationType::QtAddEexecutable:
        return QStringLiteral("qt_add_executable");
    case CMakeDocument::SourceLocationType::None:
        break;
    }
    return QString();
}



bool CMakeDocument::modify(const QString &cmakeFile,
                           const CMakeLocation &startLocation,
                           const CMakeLocation &endLocation,
                           const QString &data)
{
    if (startLocation > endLocation)
        return false;

    QTemporaryFile tempFile;
    QFileInfo fi(cmakeFile);
    if (!fi.isWritable())
        return false;

    QFile f(cmakeFile);
    if (f.open(QFile::ReadOnly | QFile::Text) && tempFile.open()) {
        QTextStream input(&f);
        QTextStream output(&tempFile);

        int lineCount = 1;
        bool skipLine = false;

        while (!input.atEnd()) {
            bool skipNextLine = false;

            QString line = input.readLine();
            if (lineCount == startLocation.line()) {
                line = line.left(startLocation.column() - 1);
                skipLine = line.isEmpty();
                skipNextLine = true;
            } else if (lineCount == endLocation.line()) {
                skipLine = false;
                line = data + line.mid(endLocation.column());
            }

            if (!skipLine) {
                output << line << "\n";
            }

            if (skipNextLine) {
                skipLine = true;
            }

            lineCount++;
        }

        f.close();
        tempFile.close();
        f.remove();
        auto res = QFile::rename(tempFile.fileName(), cmakeFile);
        qDebug() << tempFile.fileName() << cmakeFile << res;
        return true;
    }

    return false;
}

bool CMakeDocument::parse(const QString &cmakeFile)
{
    bool readError = false;
    int parentheses = 0;
    cmListFileLexer_Token *token = nullptr;

    cmListFileLexer *lexer = cmListFileLexer_New();

    if (!cmListFileLexer_SetFileName(lexer, cmakeFile.toLocal8Bit(), nullptr)) {
        qWarning() << "cmake read error";
        cmListFileLexer_Delete(lexer);
        return false;
    }

    while (!readError && (token = cmListFileLexer_Scan(lexer))) {
        switch (token->type) {
        case cmListFileLexer_Token_Identifier: {
            auto identifier = QString::fromLocal8Bit(token->text, token->length);

            if (parentheses == 0) {
                m_currentFunction = std::make_unique<CMakeFunction>(identifier,
                                                                    CMakeLocation(token->line,
                                                                                  token->column));
            } else {
                if (m_currentFunction != nullptr) {
                    m_currentFunction->appendArgument(
                        CMakeArgument(identifier, false, CMakeLocation(token->line, token->column)));
                }
            }

            break;
        }
        case cmListFileLexer_Token_ParenLeft:
            parentheses++;
            break;
        case cmListFileLexer_Token_ParenRight:
            parentheses--;
            m_currentFunction->setEndLocation(CMakeLocation(token->line, token->column));
            m_functions.push_back(std::move(m_currentFunction));
            m_currentFunction = nullptr;
            break;
        case cmListFileLexer_Token_ArgumentQuoted:
            if (m_currentFunction != nullptr) {
                auto argument = QString::fromLocal8Bit(token->text, token->length);
                m_currentFunction->appendArgument(
                    CMakeArgument(argument, true, CMakeLocation(token->line, token->column)));
            }
            break;
        case cmListFileLexer_Token_ArgumentUnquoted:
        case cmListFileLexer_Token_ArgumentBracket:
            if (m_currentFunction != nullptr) {
                auto argument = QString::fromLocal8Bit(token->text, token->length);
                m_currentFunction->appendArgument(
                    CMakeArgument(argument, false, CMakeLocation(token->line, token->column)));
            }
            break;
        case cmListFileLexer_Token_CommentBracket:
        case cmListFileLexer_Token_BadCharacter:
        case cmListFileLexer_Token_BadBracket:
        case cmListFileLexer_Token_BadString:
        case cmListFileLexer_Token_None:
        case cmListFileLexer_Token_Space:
        case cmListFileLexer_Token_Newline:
            break;
        };
    }

    cmListFileLexer_Delete(lexer);
    return true;
}

}
}
