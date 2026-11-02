import QtQuick
import QtQuick.Layouts

RowLayout {
    id: badge

    required property var source

    spacing: 8

    Rectangle {
        implicitWidth: 12
        implicitHeight: 12
        radius: 6
        color: badge.source.connected ? Theme.ok : Theme.critical
    }

    Text {
        text: badge.source.statusText
        color: Theme.textSecondary
        font.pixelSize: 14
    }
}