import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var settings        // l'objet AppSettings

    property string message: ""
    property bool messageIsError: false

    // T2 : recharge les valeurs enregistrées dans toutes les rangées
    function reload() {
        for (let i = 0; i < rows.count; ++i) {
            const item = rows.itemAt(i)
            if (item) {
                item.load()
            }
        }
    }

    // T3 : applique toutes les rangées, sans s'arrêter à la première erreur
    function applyAll() {
        let allOk = true

        for (let i = 0; i < rows.count; ++i) {
            const item = rows.itemAt(i)
            if (item) {
                const ok = item.apply()     // appelé pour CHAQUE rangée
                if (!ok) {
                    allOk = false
                }
            }
        }

        root.messageIsError = !allOk
        root.message = allOk
            ? "Réglages enregistrés"
            : "Certaines valeurs sont invalides (en rouge) : les autres ont été enregistrées"
    }

    // T4 : valeurs par défaut
    function resetAll() {
        root.settings.resetDefaults()
        reload()
        root.messageIsError = false
        root.message = "Valeurs par défaut restaurées"
    }

    // la page se recharge chaque fois qu'on revient dessus
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
            text: "Paramètres"
            color: Theme.textPrimary
            font.pixelSize: 24
            font.bold: true
        }

        Text {
            text: "Seuils d'alarme"
            color: Theme.textSecondary
            font.pixelSize: 16
        }

        // T1 : une rangée par grandeur
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

        // T5 : boutons
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
                    text: "Appliquer"
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
                    text: "Valeurs par défaut"
                    color: Theme.accent
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.resetAll()
                }
            }
        }

        // T6 : message de retour (invisible, donc sans place, quand il est vide)
        Text {
            Layout.fillWidth: true
            visible: root.message !== ""
            text: root.message
            color: root.messageIsError ? Theme.critical : Theme.ok
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }

        Item { Layout.fillHeight: true }   // espace extensible : colle tout en haut
    }
}