import QtQuick
import QtQuick.Layouts

RowLayout {
    id: row

    required property string quantity
    required property var settings

    spacing: 16


    function load() {
        warningField.text = String(settings.warning(quantity))
        criticalField.text = String(settings.critical(quantity))
        hysteresisField.text = String(settings.hysteresis(quantity))

        warningField.invalid = false
        criticalField.invalid = false
        hysteresisField.invalid = false
    }


    function apply() {
        const w = warningField.numericValue()
        const c = criticalField.numericValue()
        const h = hysteresisField.numericValue()


        warningField.invalid = isNaN(w)
        criticalField.invalid = isNaN(c)
        hysteresisField.invalid = isNaN(h) || h < 0
        if (warningField.invalid || criticalField.invalid || hysteresisField.invalid) {
            return false
        }


        if (w === settings.warning(quantity)
                && c === settings.critical(quantity)
                && h === settings.hysteresis(quantity)) {
            return true
        }


        const ok = settings.setThresholds(quantity, w, c, h)
        warningField.invalid = !ok
        criticalField.invalid = !ok
        return ok
    }

    Component.onCompleted: load()

    Text {
        Layout.preferredWidth: 120
        text: row.quantity
        color: Theme.textPrimary
        font.pixelSize: 18
        font.bold: true
    }

    NumberField { id: warningField; label: "Warning" }
    NumberField { id: criticalField; label: "Critical" }
    NumberField { id: hysteresisField; label: "Hysteresis" }

    Item { Layout.fillWidth: true }
}