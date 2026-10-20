import QtQuick
import QtQuick.Layouts

Rectangle {
    id: sidebar

    property int currentIndex: 0
    readonly property var items: ["Dashboard", "Historical", "Alarm", "Settings"]

    implicitWidth: 200
    color: Theme.sidebar

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        Text {
            text: "Supervision"
            color: Theme.textPrimary
            font.pixelSize: 20
            font.bold: true
            Layout.topMargin: 8
            Layout.bottomMargin: 20
        }

        Repeater {
            model: sidebar.items

            NavButton {
                required property int index
                required property string modelData

                Layout.fillWidth: true
                label: modelData
                selected: sidebar.currentIndex === index
                onClicked: sidebar.currentIndex = index
            }
        }

        Item { Layout.fillHeight: true }

        NavButton {
            Layout.fillWidth: true
            label: Theme.dark ? "Thème clair" : "Thème sombre"
            onClicked: Theme.dark = !Theme.dark
        }
    }
}