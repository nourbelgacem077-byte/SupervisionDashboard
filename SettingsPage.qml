import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var settings
    property string message: ""
    property bool messageIsError: false
    property string sourceMessage: ""
    property bool sourceMessageIsError: false

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

    // Remplit les champs de la source depuis les réglages enregistrés
    function loadSource() {
        sourceChoice.currentIndex = root.settings.sourceType === "mqtt" ? 1 : 0
        hostField.text = root.settings.mqttHost
        portField.text = String(root.settings.mqttPort)
        hostField.invalid = false
        portField.invalid = false
    }

    function applySourceChoice() {
        const type = sourceChoice.currentIndex === 1 ? "mqtt" : "simulator"
        const host = hostField.text.trim()
        const port = portField.numericValue()

        hostField.invalid = false
        portField.invalid = false

        if (!Number.isInteger(port)) {
            portField.invalid = true
            root.sourceMessageIsError = true
            root.sourceMessage = "Port must be a whole number"
            return
        }

        const ok = root.settings.setSource(type, host, port)

        if (!ok) {
            hostField.invalid = (host === "")
            portField.invalid = (port < 1 || port > 65535)
            root.sourceMessageIsError = true
            root.sourceMessage = "Invalid source settings (check address and port 1-65535)"
            return
        }

        root.sourceMessageIsError = false
        root.sourceMessage = "Data source applied"
    }

    onVisibleChanged: {
        if (visible) {
            root.message = ""
            root.sourceMessage = ""
            reload()
            loadSource()
        }
    }

    Component.onCompleted: loadSource()

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

        // ---------- Source de données ----------
        Text {
            text: "Data Source"
            color: Theme.textSecondary
            font.pixelSize: 16
            Layout.topMargin: 12
        }

        ChoiceBar {
            id: sourceChoice
            model: ["Simulator", "MQTT"]
        }

        RowLayout {
            spacing: 16
            enabled: sourceChoice.currentIndex === 1
            opacity: enabled ? 1.0 : 0.4

            NumberField {
                id: hostField
                label: "Broker address"
                fieldWidth: 240
            }

            NumberField {
                id: portField
                label: "Port"
            }
        }

        Rectangle {
            implicitWidth: 130
            implicitHeight: 34
            radius: 6
            color: Theme.accent

            Text {
                anchors.centerIn: parent
                text: "Apply source"
                color: Theme.window
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.applySourceChoice()
            }
        }

        Text {
            Layout.fillWidth: true
            visible: root.sourceMessage !== ""
            text: root.sourceMessage
            color: root.sourceMessageIsError ? Theme.critical : Theme.ok
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }

        Item { Layout.fillHeight: true }
    }
}