import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import SupervisionDashboard

Window {
    id: root
    width: 1200
    height: 700
    visible: true
    color: Theme.window
    title: qsTr("Supervision Dashboard")

    AppSettings { id: appSettings }

    SensorSimulator { id: simulator }
    MqttDataSource { id: mqtt }

    // La source réellement utilisée par toute l'application
    readonly property var activeSource:
        appSettings.sourceType === "mqtt" ? mqtt : simulator

    function applySource() {
        if (appSettings.sourceType === "mqtt") {
            mqtt.host = appSettings.mqttHost
            mqtt.port = appSettings.mqttPort
            mqtt.start()
        } else {
            mqtt.stop()
        }
    }

    Connections {
        target: appSettings
        function onSourceChanged() {
            console.log("sourceChanged ->", appSettings.sourceType)
            root.applySource()
        }
    }

    Component.onCompleted: applySource()

    AlarmModel {
        id: alarms
        source: root.activeSource
        settings: appSettings
    }

    DataLogger {
        source: root.activeSource
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Sidebar {
            id: sidebar
            Layout.fillHeight: true
            Layout.preferredWidth: 200
        }

        StackLayout {
            currentIndex: sidebar.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true

            DashboardPage { source: root.activeSource }
            HistoryPage { }
            AlarmsPage { alarmModel: alarms; source: root.activeSource }
            SettingsPage { settings: appSettings }
        }
    }
}