import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var simulator

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            StatCard { Layout.fillWidth: true; title: "Voltage"; value: root.simulator.voltage; unit: "V" }
            StatCard { Layout.fillWidth: true; title: "Current"; value: root.simulator.current; unit: "A" }
            StatCard { Layout.fillWidth: true; title: "Power"; value: root.simulator.power; unit: "kW"; decimals: 2 }
            StatCard { Layout.fillWidth: true; title: "Temperature"; value: root.simulator.temperature; unit: "°C" }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20

            LiveChart {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Voltage (V)"
                value: root.simulator.voltage
                yMin: 220
                yMax: 240
                lineColor: Theme.accent
            }

            LiveChart {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Current (A)"
                value: root.simulator.current
                yMin: 6
                yMax: 14
                lineColor: Theme.accent2
            }

            GaugeWidget {
                Layout.preferredWidth: 260
                Layout.fillHeight: true
                title: "Temperature"
                value: root.simulator.temperature
                minValue: 0
                maxValue: 120
                warning: 75
                critical: 90
                unit: "°C"
            }
        }
    }
}