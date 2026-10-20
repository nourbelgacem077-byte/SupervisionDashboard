import QtQuick
import QtQuick.Layouts

RowLayout {
    id: bar

    property var model: []
    property int currentIndex: 0

    spacing: 8

    Repeater {
        model: bar.model

        Rectangle {
            id: choice

            required property int index
            required property string modelData

            readonly property bool selected: bar.currentIndex === index

            implicitWidth: label.implicitWidth + 28
            implicitHeight: 34
            radius: 17
            color: selected ? Theme.accent : "transparent"
            border.color: selected ? Theme.accent : Theme.border

            Text {
                id: label
                anchors.centerIn: parent
                text: choice.modelData
                color: choice.selected ? Theme.window : Theme.textSecondary
                font.pixelSize: 14
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: bar.currentIndex = choice.index
            }
        }
    }
}