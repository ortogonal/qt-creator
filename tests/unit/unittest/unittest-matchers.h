// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <gmock/gmock-matchers.h>

#include <utils/smallstringio.h>

namespace UnitTests {

template <typename StringType>
class EndsWithMatcher
{
 public:
    explicit EndsWithMatcher(const StringType& suffix) : m_suffix(suffix) {}

    template <typename CharType>
    bool MatchAndExplain(CharType *s, testing::MatchResultListener *listener) const
    {
        return s != NULL && MatchAndExplain(StringType(s), listener);
    }


    template <typename MatcheeStringType>
    bool MatchAndExplain(const MatcheeStringType& s,
                         testing::MatchResultListener* /* listener */) const
    {
        return s.endsWith(m_suffix);
    }

    void DescribeTo(::std::ostream* os) const
    {
        *os << "ends with " << m_suffix;
    }

    void DescribeNegationTo(::std::ostream* os) const
    {
        *os << "doesn't end with " << m_suffix;
    }

    EndsWithMatcher(EndsWithMatcher const &) = default;
    EndsWithMatcher &operator=(EndsWithMatcher const &) = delete;

private:
    const StringType m_suffix;
};

inline
testing::PolymorphicMatcher<EndsWithMatcher<Utils::SmallString> >
EndsWith(const Utils::SmallString &suffix)
{
  return testing::MakePolymorphicMatcher(EndsWithMatcher<Utils::SmallString>(suffix));
}
}
