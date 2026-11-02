import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var source

    ColumnLayout {

        anchors.fill: parent
        anchors.margins: 20
        spacing: 20
 ConnectionBadge { source: root.source }
        RowLayout {
            Layout.fillWidth: true
            spacing: 20
 opacity: root.source.connected ? 1.0 : 0.4
            StatCard { Layout.fillWidth: true; title: "Voltage"; value: root.source.voltage; unit: "V" }
            StatCard { Layout.fillWidth: true; title: "Current"; value: root.source.current; unit: "A" }
            StatCard { Layout.fillWidth: true; title: "Power"; value: root.source.power; unit: "kW"; decimals: 2 }
            StatCard { Layout.fillWidth: true; title: "Temperature"; value: root.source.temperature; unit: "°C" }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20
 opacity: root.source.connected ? 1.0 : 0.4
            LiveChart {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Voltage (V)"
                value: root.source.voltage
                yMin: 220
                yMax: 240
                lineColor: Theme.accent
            }

            LiveChart {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Current (A)"
                value: root.source.current
                yMin: 6
                yMax: 14
                lineColor: Theme.accent2
            }

            GaugeWidget {
                Layout.preferredWidth: 260
                Layout.fillHeight: true
                title: "Temperature"
                value: root.source.temperature
                minValue: 0
                maxValue: 120
                warning: 75
                critical: 90
                unit: "°C"
            }
        }
    }
}