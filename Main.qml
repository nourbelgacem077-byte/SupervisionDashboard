import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import SupervisionDashboard

Window {
    width: 1200
    height: 700
    visible: true
    color: Theme.window
    title: qsTr("Supervision Dashboard")

    SensorSimulator {
        id: sim
    }
    AlarmModel {
          id: alarms
          simulator: sim
          settings: appSettings
      }

    DataLogger {
        simulator: sim
    }
   AppSettings { id: appSettings }
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

            DashboardPage { simulator: sim }
            HistoryPage { }
            AlarmsPage { alarmModel: alarms; simulator: sim }
            SettingsPage { settings: appSettings }
        }
    }
}