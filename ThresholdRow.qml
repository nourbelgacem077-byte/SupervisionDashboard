import QtQuick
import QtQuick.Layouts

RowLayout {
    id: row

    required property string quantity
    required property var settings        // l'objet AppSettings

    spacing: 16

    // affiche les valeurs enregistrées
    function load() {
        warningField.text = String(settings.warning(quantity))
        criticalField.text = String(settings.critical(quantity))
        hysteresisField.text = String(settings.hysteresis(quantity))

        warningField.invalid = false
        criticalField.invalid = false
        hysteresisField.invalid = false
    }

    // envoie les valeurs saisies à AppSettings ; retourne true si elles sont acceptées
    function apply() {
        const w = warningField.numericValue()
        const c = criticalField.numericValue()
        const h = hysteresisField.numericValue()

        // 1ère couche : la saisie est-elle lisible ?
        warningField.invalid = isNaN(w)
        criticalField.invalid = isNaN(c)
        hysteresisField.invalid = isNaN(h) || h < 0
        if (warningField.invalid || criticalField.invalid || hysteresisField.invalid) {
            return false
        }

        // rien n'a changé : on ne touche à rien (les alarmes ne sont pas remises à zéro)
        if (w === settings.warning(quantity)
                && c === settings.critical(quantity)
                && h === settings.hysteresis(quantity)) {
            return true
        }

        // 2ème couche : la règle métier (Warning < Critical)
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
    NumberField { id: hysteresisField; label: "Hystérésis" }

    Item { Layout.fillWidth: true }
}