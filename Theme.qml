pragma Singleton
import QtQuick

QtObject {
    property bool dark: true

    // surfaces
    readonly property color window: dark ? "#121820" : "#f2f5f9"
    readonly property color sidebar: dark ? "#161c26" : "#e4e9f0"
    readonly property color card: dark ? "#1e2530" : "#ffffff"
    readonly property color border: dark ? "#3a4556" : "#cdd5e0"
    readonly property color grid: dark ? "#2c3646" : "#dde3ea"

    // textes
    readonly property color textPrimary: dark ? "#ffffff" : "#1b2430"
    readonly property color textSecondary: dark ? "#9aa7b8" : "#5a6a7e"

    // navigation
    readonly property color navSelected: dark ? "#2a3a52" : "#d3e3f5"
    readonly property color navHover: dark ? "#202a38" : "#dfe6ee"

    // courbes et jauge
    readonly property color accent: dark ? "#4fc3f7" : "#0277bd"
    readonly property color accent2: dark ? "#ffb74d" : "#ef6c00"
    readonly property color gaugeTrack: dark ? "#3a4556" : "#d5dce6"

    // états
    readonly property color ok: "#66bb6a"
    readonly property color warning: "#ffb74d"
    readonly property color critical: "#ef5350"
}