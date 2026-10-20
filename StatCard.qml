import QtQuick

Rectangle {
    id: card

    property string title: ""
    property real value: 0
    property string unit: ""
    property int decimals: 1

    implicitWidth: 200
    implicitHeight: 110
    radius: 10
    color: Theme.card
    border.color: Theme.border

    Column {
        anchors.centerIn: parent
        spacing: 6

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: card.title
            color: Theme.textSecondary
            font.pixelSize: 16
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: card.value.toFixed(card.decimals) + " " + card.unit
            color: Theme.textPrimary
            font.pixelSize: 30
            font.bold: true
        }
    }
}