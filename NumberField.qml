import QtQuick
import QtQuick.Layouts

ColumnLayout {
    id: field

    property string label: ""
    property alias text: input.text
    property bool invalid: false

    spacing: 4

    // convertit la saisie en nombre ; NaN si vide ou illisible
    function numericValue() {
        const t = input.text.trim().replace(",", ".")
        return t === "" ? NaN : Number(t)
    }

    Text {
        text: field.label
        color: Theme.textSecondary
        font.pixelSize: 13
    }

    Rectangle {
        implicitWidth: 110
        implicitHeight: 36
        radius: 6
        color: Theme.card
        border.color: field.invalid ? Theme.critical
                                    : (input.activeFocus ? Theme.accent : Theme.border)
        border.width: (field.invalid || input.activeFocus) ? 2 : 1

        TextInput {
            id: input
            anchors.fill: parent
            leftPadding: 8
            rightPadding: 8
            verticalAlignment: TextInput.AlignVCenter
            color: Theme.textPrimary
            font.pixelSize: 16
            selectByMouse: true
            clip: true

            // l'utilisateur modifie le champ : on retire le signal d'erreur
            onTextEdited: field.invalid = false
        }
    }
}