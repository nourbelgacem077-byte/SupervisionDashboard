import QtQuick

Item {
    id: root
    property string title: ""

    Text {
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        text: root.title + "\n(Comming soon)"
        color: Theme.textSecondary
        font.pixelSize: 24
    }
}