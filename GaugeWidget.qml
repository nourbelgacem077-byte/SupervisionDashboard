import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes

Rectangle {
    id: gauge

    property string title: ""
    property string unit: ""
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property real warning: 75
    property real critical: 90


    readonly property real fraction:
        Math.max(0, Math.min(1, (value - minValue) / (maxValue - minValue)))


    readonly property color stateColor:
        value >= critical ? Theme.critical : (value >= warning ? Theme.warning : Theme.ok)


    property real animatedFraction: fraction
    Behavior on animatedFraction { NumberAnimation { duration: 250 } }

    readonly property real thickness: 14

    implicitWidth: 250
    implicitHeight: 250
    radius: 10
    color: Theme.card
    border.color: Theme.border

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        Text {
            text: gauge.title
            color: Theme.textSecondary
            font.pixelSize: 16
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                id: dial
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height)
                height: width

                Shape {
                    anchors.fill: parent
                    layer.enabled: true
                    layer.samples: 4

                    // arc de fond
                    ShapePath {
                        strokeColor: Theme.gaugeTrack
                        strokeWidth: gauge.thickness
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap

                        PathAngleArc {
                            centerX: dial.width / 2
                            centerY: dial.height / 2
                            radiusX: dial.width / 2 - gauge.thickness
                            radiusY: dial.height / 2 - gauge.thickness
                            startAngle: 150
                            sweepAngle: 240
                        }
                    }


                    ShapePath {
                        strokeColor: gauge.stateColor
                        strokeWidth: gauge.thickness
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap

                        PathAngleArc {
                            centerX: dial.width / 2
                            centerY: dial.height / 2
                            radiusX: dial.width / 2 - gauge.thickness
                            radiusY: dial.height / 2 - gauge.thickness
                            startAngle: 150
                            sweepAngle: 240 * gauge.animatedFraction
                        }
                    }
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 2

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: gauge.value.toFixed(1)
                        color: Theme.textPrimary
                        font.pixelSize: 32
                        font.bold: true
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: gauge.unit
                        color: Theme.textSecondary
                        font.pixelSize: 16
                    }
                }
            }
        }
    }
}