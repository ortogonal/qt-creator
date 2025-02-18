// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <synchronousimagecache.h>

#include <QQuickImageProvider>

namespace QmlDesigner {

class AssetsLibraryIconProvider : public QQuickImageProvider
{
public:
    AssetsLibraryIconProvider(SynchronousImageCache &fontImageCache);

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QPixmap generateFontIcons(const QString &filePath, const QSize &requestedSize) const;

    SynchronousImageCache &m_fontImageCache;

    // Generated icon sizes should contain all ItemLibraryResourceView needed icon sizes, and their
    // x2 versions for HDPI sceens
    std::vector<QSize> iconSizes = {{128, 128}, // Drag
                                    {96, 96},   // list @2x
                                    {48, 48}};  // list
};

} // namespace QmlDesigner
