// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "common.h"
#include "symbolgroupnode.h"

#include <map>

class SymbolGroup {
public:
    typedef std::vector<DEBUG_SYMBOL_PARAMETERS> SymbolParameterVector;

private:
    SymbolGroup(const SymbolGroup &);
    SymbolGroup &operator=(const SymbolGroup &);

protected:
    explicit SymbolGroup(CIDebugSymbolGroup *,
                         const SymbolParameterVector &vec,
                         const std::string &rootModule,
                         const char *rootName);

public:
    typedef AbstractSymbolGroupNode::AbstractSymbolGroupNodePtrVector AbstractSymbolGroupNodePtrVector;

    virtual ~SymbolGroup();

    // Dump all
    std::string dump(const SymbolGroupValueContext &ctx,
                     const DumpParameters &p = DumpParameters()) const;
    // Expand node and dump
    std::string dump(const std::string &iname, const SymbolGroupValueContext &ctx,
                     const DumpParameters &p, std::string *errorMessage);
    std::string debug(const std::string &iname = std::string(),
                      const std::string &filter = std::string(),
                      unsigned verbosity = 0) const;

    SymbolGroupNode *root() { return m_root; }
    const SymbolGroupNode *root() const { return m_root; }
    AbstractSymbolGroupNode *find(const std::string &iname) const;

    // Expand a single node "locals.A.B" requiring that "locals.A.B" is already visible
    // (think mkdir without -p).
    bool expand(const std::string &node, std::string *errorMessage);
    bool collapse(const std::string &node, std::string *errorMessage);
    bool expandRunComplexDumpers(const std::string &node, const SymbolGroupValueContext &ctx, std::string *errorMessage);
    // Expand a node list "locals.i1,locals.i2", expanding all nested child nodes
    // (think mkdir -p).
    unsigned expandList(const std::vector<std::string> &nodes, std::string *errorMessage);
    unsigned expandListRunComplexDumpers(const std::vector<std::string> &nodes,
                                         const SymbolGroupValueContext &ctx,
                                         std::string *errorMessage);

    // Mark uninitialized (top level only)
    void markUninitialized(const std::vector<std::string> &nodes);

    // Cast an (unexpanded) node
    bool typeCast(const std::string &iname, const std::string &desiredType, std::string *errorMessage);
    // Add a symbol by name expression
    SymbolGroupNode *addSymbol(const std::string &module,
                               const std::string &name,        // Expression like 'myarray[1]'
                               const std::string &displayName, // Name to be displayed, defaults to name
                               const std::string &iname, // Desired iname, defaults to name
                               std::string *errorMessage);
    // Convenience overload for name==displayName
    SymbolGroupNode *addSymbol(const std::string &module,
                               const std::string &name, // Expression like 'myarray[1]'
                               const std::string &iname,
                               std::string *errorMessage);

    bool accept(SymbolGroupNodeVisitor &visitor) const;

    // Assign a value by iname
    bool assign(const std::string &node,
                const std::string &value,
                const SymbolGroupValueContext &ctx,
                std::string *errorMessage);

    CIDebugSymbolGroup *debugSymbolGroup() const { return m_symbolGroup; }

    static bool getSymbolParameters(CIDebugSymbolGroup *m_symbolGroup,
                                    unsigned long start,
                                    unsigned long count,
                                    SymbolParameterVector *vec,
                                    std::string *errorMessage);

protected:
    static bool getSymbolParameters(CIDebugSymbolGroup *m_symbolGroup,
                                    SymbolParameterVector *vec,
                                    std::string *errorMessage);
    bool removeSymbol(AbstractSymbolGroupNode *n, std::string *errorMessage);

private:
    inline AbstractSymbolGroupNode *findI(const std::string &iname) const;

    CIDebugSymbolGroup * const m_symbolGroup;
    SymbolGroupNode *m_root;
};

class LocalsSymbolGroup : public SymbolGroup {
protected:
    explicit LocalsSymbolGroup(CIDebugSymbolGroup *,
                               const SymbolParameterVector &vec,
                               ULONG threadId, unsigned frame,
                               const std::string &function);
public:
    unsigned frame() const { return m_frame; }
    std::string function() const { return m_function; }
    std::string module() const;
    ULONG threadId() const { return m_threadId; }

    static LocalsSymbolGroup *create(CIDebugControl *control,
                                     CIDebugSymbols *,
                                     ULONG threadId,
                                     unsigned frame,
                                     std::string *errorMessage);

private:
    const unsigned m_frame;
    const ULONG m_threadId;
    std::string m_function;
};

class WatchesSymbolGroup : public SymbolGroup {
public:
    typedef std::map<std::string, std::string> InameExpressionMap;

    static const char *watchInamePrefix;
    static bool isWatchIname(const std::string &iname);

    // Add a symbol as 'watch.0' or '0' with expression
    bool addWatch(CIDebugSymbols *s, std::string iname, const std::string &expression, std::string *errorMessage);
    // Synchronize watches passing on a map of '0' -> '*(int *)(0xA0)'
    bool synchronize(CIDebugSymbols *s, const InameExpressionMap &m, std::string *errorMessage);

    static WatchesSymbolGroup *create(CIDebugSymbols *, std::string *errorMessage);

    static std::string fixWatchExpression(CIDebugSymbols *s, const std::string &ex);

private:
    explicit WatchesSymbolGroup(CIDebugSymbolGroup *);
    InameExpressionMap currentInameExpressionMap() const;
    inline SymbolGroupNode *rootChildByIname(const std::string &iname) const;
    static inline std::string fixWatchExpressionI(CIDebugSymbols *s, const std::string &ex);
    bool collapsePointerItems(std::string *errorMessage);
};
