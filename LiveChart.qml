import QtQuick
import QtQuick.Layouts
import QtGraphs

Rectangle {
    id: root

    property string title: ""
    property real value: 0
    property real yMin: 0
    property real yMax: 100
    property color lineColor: Theme.accent
    property real windowSeconds: 60

    property double startTime: Date.now()
    property var times: []

    implicitWidth: 400
    implicitHeight: 250
    radius: 10
    color: Theme.card
    border.color: Theme.border

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        Text {
            text: root.title
            color: Theme.textSecondary
            font.pixelSize: 16
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            theme: GraphsTheme {
                theme: GraphsTheme.Theme.UserDefined
                backgroundColor: Theme.card
                plotAreaBackgroundColor: Theme.card
                labelTextColor: Theme.textSecondary
                grid.mainColor: Theme.grid
                grid.subColor: Theme.grid
            }

            axisX: ValueAxis {
                id: timeAxis
                min: 0
                max: root.windowSeconds
                tickInterval: 10
            }
            axisY: ValueAxis {
                min: root.yMin
                max: root.yMax
            }

            LineSeries {
                id: series
                color: root.lineColor
            }
        }
    }

    onValueChanged: {
        const t = (Date.now() - startTime) / 1000.0

        series.append(t, value)
        times.push(t)

        while (times.length > 0 && times[0] < t - windowSeconds) {
            series.remove(0)
            times.shift()
        }

        if (t > windowSeconds) {
            timeAxis.min = t - windowSeconds
            timeAxis.max = t
        }
    }
}