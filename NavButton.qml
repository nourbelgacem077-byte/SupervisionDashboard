import QtQuick

Rectangle {
    id: button

    property string label: ""
    property bool selected: false
    signal clicked()

    implicitHeight: 44
    radius: 8
    color: selected ? Theme.navSelected : (area.containsMouse ? Theme.navHover : "transparent")

    Rectangle {
        visible: button.selected
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: 4
        height: parent.height * 0.6
        radius: 2
        color: Theme.accent
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        text: button.label
        color: button.selected ? Theme.textPrimary : Theme.textSecondary
        font.pixelSize: 16
    }

    MouseArea {
        id: area
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: button.clicked()
    }
}