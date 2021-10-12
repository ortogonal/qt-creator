#pragma once

#include <QString>
#include <QDebug>

namespace CMakeProjectManager {
namespace Internal {

class CMakeLocation
{
public:
    CMakeLocation();
    CMakeLocation(int line, int column);

    bool valid() const;

    int line() const;
    int column() const;

    bool operator==(const CMakeLocation &rhs) const;
    bool operator!=(const CMakeLocation &rhs) const;

    bool operator<(const CMakeLocation &rhs) const;
    bool operator<=(const CMakeLocation &rhs) const;
    bool operator>(const CMakeLocation &rhs) const;
    bool operator>=(const CMakeLocation &rhs) const;

private:
    int m_line = -1;
    int m_column = -1;
};

QDebug operator<<(QDebug debug, const CMakeLocation &l);

class CMakeArgument
{
public:
    CMakeArgument(const QString &name);
    CMakeArgument(const QString &name, bool quotes, const CMakeLocation &startLocation);

    QString name() const;
    bool quotes() const;
    CMakeLocation startLocation() const;

private:
    QString m_name;
    bool m_quotes = false;
    CMakeLocation m_startLocation;
};

class CMakeFunction
{
public:
    CMakeFunction() = default;
    CMakeFunction(const QString &name, const CMakeLocation &startLocation);
    CMakeFunction(const CMakeFunction &other) = delete;

    QString name() const;

    CMakeLocation startLocation() const;
    CMakeLocation endLocation() const;

    void setEndLocation(const CMakeLocation &endLocation);

    void appendArgument(const CMakeArgument &argument);

    QString data() const;

    QString firstArgumemt() const;
    void dumpArguments();

private:
    QString m_name;
    QList<CMakeArgument> m_arguments;
    CMakeLocation m_startLocation;
    CMakeLocation m_endLocation;
};

class CMakeDocument
{
public:
    enum class SourceLocationType {
        Set,
        AddExecutable,
        AddLibrary,
        QtAddEexecutable,

        None
    };

    struct SourceLocationLocation {
        SourceLocationType type;
        CMakeLocation location;
    };

    CMakeDocument() = default;
    CMakeDocument(const QString &cmakeFile);

    bool valid() const;
    QString cmakeFile() const;

    bool addSource(const QStringList sources);

    bool uniqueSourceLocation() const;
    int noSourceLocations() const;

    SourceLocationLocation sourceLocation(int position);

    static QString sourceLocationTypeString(SourceLocationType type);

private:
    bool modify(const QString &cmakeFile, const CMakeLocation &startLocation, const CMakeLocation &endLocation, const QString &data);
    bool parse(const QString &cmakeFile);

    QString m_cmakeFile;
    std::vector<std::unique_ptr<CMakeFunction>> m_functions;
    std::unique_ptr<CMakeFunction> m_currentFunction;

    bool m_fileParsed = false;
};

}
}
