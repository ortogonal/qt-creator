// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuick.Layouts 1.15
import HelperWidgets 2.0
import StudioControls 1.0 as StudioControls
import StudioTheme 1.0 as StudioTheme

Section {
    id: root
    anchors.left: parent.left
    anchors.right: parent.right
    caption: qsTr("Text Extras")

    property bool showElide: false
    property bool showWrapMode: false
    property bool showFormatProperty: false
    property bool showFontSizeMode: false
    property bool showLineHeight: false

    function isBackendValueAvailable(name) {
        if (backendValues[name] !== undefined)
            return backendValues[name].isAvailable

        return false
    }

    SectionLayout {
        id: sectionLayout

        PropertyLabel {
            visible: root.showWrapMode
            text: qsTr("Wrap mode")
            blockedByTemplate: !root.isBackendValueAvailable("wrapMode")
        }

        SecondColumnLayout {
            visible: root.showWrapMode

            ComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                backendValue: backendValues.wrapMode
                scope: "Text"
                model: ["NoWrap", "WordWrap", "WrapAnywhere", "Wrap"]
                enabled: root.isBackendValueAvailable("wrapMode")
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: root.showElide
            text: qsTr("Elide")
            blockedByTemplate: !root.isBackendValueAvailable("elide")
        }

        SecondColumnLayout {
            visible: root.showElide

            ComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                backendValue: backendValues.elide
                scope: "Text"
                model: ["ElideNone", "ElideLeft", "ElideMiddle", "ElideRight"]
                enabled: root.isBackendValueAvailable("elide")
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: root.showFormatProperty
            text: qsTr("Format")
            blockedByTemplate: !root.isBackendValueAvailable("textFormat")
        }

        SecondColumnLayout {
            visible: root.showFormatProperty

            ComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                scope: "Text"
                model: ["PlainText", "RichText", "AutoText", "StyledText", "MarkdownText"]
                backendValue: backendValues.textFormat
                enabled: root.isBackendValueAvailable("textFormat")
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            text: qsTr("Render type")
            tooltip: qsTr("Overrides the default rendering type for this component.")
            blockedByTemplate: !root.isBackendValueAvailable("renderType")
        }

        SecondColumnLayout {
            ComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                scope: "Text"
                model: ["QtRendering", "NativeRendering"]
                backendValue: backendValues.renderType
                enabled: root.isBackendValueAvailable("renderType")
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            text: qsTr("Render type quality")
            tooltip: qsTr("Overrides the default rendering type quality for this component.")
            blockedByTemplate: !root.isBackendValueAvailable("renderTypeQuality")
            enabled: backendValues.renderType !== undefined
                        ? backendValues.renderType.enumeration === "QtRendering"
                        : false
        }

        SecondColumnLayout {
            ComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                scope: "Text"
                model: ["DefaultRenderTypeQuality", "LowRenderTypeQuality", "NormalRenderTypeQuality",
                        "HighRenderTypeQuality", "VeryHighRenderTypeQuality"]
                backendValue: backendValues.renderTypeQuality
                enabled: root.isBackendValueAvailable("renderTypeQuality")
                         && backendValues.renderType.enumeration === "QtRendering"
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: root.showLineHeight
            text: qsTr("Line height mode")
            tooltip: qsTr("Determines how the line height is specified.")
            blockedByTemplate: !root.isBackendValueAvailable("lineHeightMode")
        }

        SecondColumnLayout {
            visible: root.showLineHeight

            ComboBox {
                scope: "Text"
                model: ["ProportionalHeight", "FixedHeight"]
                backendValue: backendValues.lineHeightMode
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                enabled: root.isBackendValueAvailable("lineHeightMode")
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: root.showFontSizeMode
            text: qsTr("Size mode")
            tooltip: qsTr("Specifies how the font size of the displayed text is determined.")
            blockedByTemplate: !root.isBackendValueAvailable("fontSizeMode")
        }

        SecondColumnLayout {
            visible: root.showFontSizeMode

            ComboBox {
                id: fontSizeMode
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                scope: "Text"
                model: ["FixedSize", "HorizontalFit", "VerticalFit", "Fit"]
                backendValue: backendValues.fontSizeMode
                enabled: root.isBackendValueAvailable("fontSizeMode")
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: root.showFontSizeMode
            text: qsTr("Min size")
            blockedByTemplate: !root.isBackendValueAvailable("minimumPixelSize")
                               && !root.isBackendValueAvailable("minimumPointSize")
        }

        SecondColumnLayout {
            visible: root.showFontSizeMode

            SpinBox {
                enabled: (fontSizeMode.currentIndex !== 0)
                         || root.isBackendValueAvailable("minimumPixelSize")
                minimumValue: 0
                maximumValue: 500
                decimals: 0
                backendValue: backendValues.minimumPixelSize
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
            }

            Spacer { implicitWidth: StudioTheme.Values.controlLabelGap }

            ControlLabel {
                text: "px"
                tooltip: qsTr("Minimum font pixel size of scaled text.")
                enabled: root.isBackendValueAvailable("minimumPixelSize")
            }

            Spacer { implicitWidth: StudioTheme.Values.controlGap }

            SpinBox {
                enabled: (fontSizeMode.currentIndex !== 0)
                         || root.isBackendValueAvailable("minimumPointSize")
                minimumValue: 0
                maximumValue: 500
                decimals: 0
                backendValue: backendValues.minimumPointSize
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
            }

            Spacer { implicitWidth: StudioTheme.Values.controlLabelGap }

            ControlLabel {
                text: "pt"
                tooltip: qsTr("Minimum font point size of scaled text.")
                enabled: root.isBackendValueAvailable("minimumPointSize")
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: root.showElide
            text: qsTr("Max line count")
            tooltip: qsTr("Limits the number of lines that the text component will show.")
            blockedByTemplate: !root.isBackendValueAvailable("maximumLineCount")
        }

        SecondColumnLayout {
            visible: root.showElide

            SpinBox {
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.maximumLineCount
                minimumValue: 0
                maximumValue: 10000
                decimals: 0
                enabled: root.isBackendValueAvailable("maximumLineCount")
            }

            ExpandingSpacer {}
        }
    }
}
