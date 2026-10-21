import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var settings
    property string message: ""
    property bool messageIsError: false

    function reload() {
        for (let i = 0; i < rows.count; ++i) {
            const item = rows.itemAt(i)
            if (item) {
                item.load()
            }
        }
    }


    function applyAll() {
        let allOk = true

        for (let i = 0; i < rows.count; ++i) {
            const item = rows.itemAt(i)
            if (item) {
                const ok = item.apply()
                if (!ok) {
                    allOk = false
                }
            }
        }

        root.messageIsError = !allOk
        root.message = allOk
            ? "Settings saved successfully"
            : "Some values are invalid (highlighted in red); the others have been saved"
    }


    function resetAll() {
        root.settings.resetDefaults()
        reload()
        root.messageIsError = false
        root.message = "Default values restored"
    }


    onVisibleChanged: {
        if (visible) {
            root.message = ""
            reload()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Text {
            text: "Settings"
            color: Theme.textPrimary
            font.pixelSize: 24
            font.bold: true
        }

        Text {
            text: "Alarm Thresholds"
            color: Theme.textSecondary
            font.pixelSize: 16
        }


        Repeater {
            id: rows
            model: root.settings.quantities()

            ThresholdRow {
                required property string modelData

                Layout.fillWidth: true
                settings: root.settings
                quantity: modelData
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                implicitWidth: 130
                implicitHeight: 34
                radius: 6
                color: Theme.accent

                Text {
                    anchors.centerIn: parent
                    text: "Apply"
                    color: Theme.window
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.applyAll()
                }
            }

            Rectangle {
                implicitWidth: 170
                implicitHeight: 34
                radius: 6
                color: "transparent"
                border.color: Theme.accent

                Text {
                    anchors.centerIn: parent
                    text: "Default Values"
                    color: Theme.accent
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.resetAll()
                }
            }
        }


        Text {
            Layout.fillWidth: true
            visible: root.message !== ""
            text: root.message
            color: root.messageIsError ? Theme.critical : Theme.ok
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }

        Item { Layout.fillHeight: true }
    }
}