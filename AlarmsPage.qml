import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var alarmModel
    required property var source

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        // ---------- En-tête ----------
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Text {
                text: "Alarm"
                color: Theme.textPrimary
                font.pixelSize: 24
                font.bold: true
            }
            Text {
                text: "active: " + root.alarmModel.activeCount
                color: Theme.textSecondary
                font.pixelSize: 16
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                implicitWidth: 130
                implicitHeight: 34
                radius: 6
                color: Theme.critical

                Text {
                    anchors.centerIn: parent
                    text: "overvoltage"
                    color: "white"
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.source.injectOvervoltage()
                }
            }

            Rectangle {
                implicitWidth: 130
                implicitHeight: 34
                radius: 6
                color: Theme.warning

                Text {
                    anchors.centerIn: parent
                    text: "Overheated"
                    color: "white"
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.source.injectOverheat()
                }
            }
        }

        // ---------- Liste (dans un conteneur pour pouvoir centrer le message vide) ----------
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: list
                anchors.fill: parent
                clip: true
                spacing: 8
                model: root.alarmModel

                delegate: Rectangle {
                    id: row

                    required property int index
                    required property string quantity
                    required property int level       // 1 = Warning, 2 = Critical
                    required property real value
                    required property string time
                    required property bool acknowledged

                    width: ListView.view.width
                    height: 60
                    radius: 8
                    color: Theme.card
                    border.color: Theme.border
                    opacity: acknowledged ? 0.5 : 1.0

                    // T3 : barre de couleur selon le niveau
                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.margins: 6
                        width: 6
                        radius: 3
                        color: row.level === 2 ? Theme.critical : Theme.warning
                    }

                    // Nom de la grandeur + détails (T4)
                    Column {
                        anchors.left: parent.left
                        anchors.leftMargin: 24
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: row.quantity
                            color: Theme.textPrimary
                            font.pixelSize: 18
                            font.bold: true
                        }
                        Text {
                            text: (row.level === 2 ? "Critical" : "Warning")
                                  + "   " + row.value.toFixed(1)
                                  + "   " + row.time
                            color: Theme.textSecondary
                            font.pixelSize: 14
                        }
                    }

                    // T5 : bouton Acquitter, visible seulement si non acquittée
                    Rectangle {
                        anchors.right: parent.right
                        anchors.rightMargin: 16
                        anchors.verticalCenter: parent.verticalCenter
                        implicitWidth: 110
                        implicitHeight: 32
                        radius: 6
                        color: "transparent"
                        border.color: Theme.accent
                        visible: !row.acknowledged

                        Text {
                            anchors.centerIn: parent
                            text: "Acquit"
                            color: Theme.accent
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.alarmModel.acknowledge(row.index)
                        }
                    }
                }
            }

            // T6 : message quand la liste est vide
            Text {
                anchors.centerIn: parent
                visible: list.count === 0
                text: "No alarm"
                color: Theme.textSecondary
                font.pixelSize: 20
            }
        }
    }
}