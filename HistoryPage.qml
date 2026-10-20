import QtQuick
import QtQuick.Layouts
import QtGraphs

Item {
    id: root

    // noms IDENTIQUES à la liste blanche du C++ (accent compris)
    readonly property var quantities: ["Tension", "Courant", "Puissance", "Température"]
    readonly property var rangeLabels: ["1 min", "10 min", "1 h", "Tout"]
    readonly property var rangeMs: [60000, 600000, 3600000, 0]   // 0 = tout l'historique

    property int pointCount: 0
    property string statusText: ""

    HistoryReader { id: reader }

    // début de la plage choisie (0 = depuis le début de l'historique)
    function rangeStart(now) {
        const duration = root.rangeMs[rangeBar.currentIndex]
        return duration === 0 ? 0 : now - duration
    }

    // règle un axe sans jamais laisser min dépasser max
    function setRange(axis, lo, hi) {
        if (lo > axis.max) {
            axis.max = hi
            axis.min = lo
        } else {
            axis.min = lo
            axis.max = hi
        }
    }

    function refresh() {
        const now = Date.now()
        const pts = reader.query(root.quantities[quantityBar.currentIndex],
                                 rangeStart(now), now, 600)

        series.clear()

        if (pts.length === 0) {
            root.pointCount = 0
            root.statusText = ""
            setRange(xAxis, 0, 1)
            setRange(yAxis, 0, 1)
            return
        }

        let yMin = pts[0].y
        let yMax = pts[0].y
        for (let i = 0; i < pts.length; ++i) {
            series.append(pts[i].x, pts[i].y)
            yMin = Math.min(yMin, pts[i].y)
            yMax = Math.max(yMax, pts[i].y)
        }

        const xMax = pts[pts.length - 1].x > 0 ? pts[pts.length - 1].x : 1
        const margin = yMax > yMin ? (yMax - yMin) * 0.1 : 1

        setRange(xAxis, 0, xMax)
        setRange(yAxis, yMin - margin, yMax + margin)

        root.pointCount = pts.length
        root.statusText = pts.length + " points, " + xMax.toFixed(1) + " min"
    }

    function exportCurrent() {
        const now = Date.now()
        const path = reader.exportCsv(root.quantities[quantityBar.currentIndex],
                                      rangeStart(now), now)
        root.statusText = path !== "" ? "Exporté : " + path
                                      : "Export impossible : aucune donnée"
    }

    onVisibleChanged: {
        if (visible) {
            refresh()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        // ---------- En-tête ----------
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Text {
                text: "Historique"
                color: Theme.textPrimary
                font.pixelSize: 24
                font.bold: true
            }
            Text {
                Layout.fillWidth: true
                text: root.statusText
                color: Theme.textSecondary
                font.pixelSize: 16
                elide: Text.ElideMiddle
            }

            Rectangle {
                implicitWidth: 120
                implicitHeight: 34
                radius: 6
                color: Theme.accent

                Text {
                    anchors.centerIn: parent
                    text: "Actualiser"
                    color: Theme.window
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.refresh()
                }
            }

            Rectangle {
                implicitWidth: 130
                implicitHeight: 34
                radius: 6
                color: "transparent"
                border.color: Theme.accent

                Text {
                    anchors.centerIn: parent
                    text: "Exporter CSV"
                    color: Theme.accent
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.exportCurrent()
                }
            }
        }

        // ---------- Choix de la grandeur et de la plage ----------
        RowLayout {
            Layout.fillWidth: true
            spacing: 32

            ChoiceBar {
                id: quantityBar
                model: root.quantities
                onCurrentIndexChanged: root.refresh()
            }

            ChoiceBar {
                id: rangeBar
                model: root.rangeLabels
                onCurrentIndexChanged: root.refresh()
            }

            Item { Layout.fillWidth: true }
        }

        // ---------- Graphique ----------
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 10
            color: Theme.card
            border.color: Theme.border

            GraphsView {
                id: chart
                anchors.fill: parent
                anchors.margins: 12

                theme: GraphsTheme {
                    theme: GraphsTheme.Theme.UserDefined
                    backgroundColor: Theme.card
                    plotAreaBackgroundColor: Theme.card
                    labelTextColor: Theme.textSecondary
                    grid.mainColor: Theme.grid
                    grid.subColor: Theme.grid
                }

                axisX: ValueAxis { id: xAxis; min: 0; max: 1 }
                axisY: ValueAxis { id: yAxis; min: 0; max: 1 }

                LineSeries {
                    id: series
                    color: Theme.accent
                }
            }

            Text {
                anchors.centerIn: parent
                visible: root.pointCount === 0
                text: "Aucune donnée pour cette période"
                color: Theme.textSecondary
                font.pixelSize: 20
            }
        }
    }
}