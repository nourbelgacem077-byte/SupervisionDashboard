#include "measurementparser.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStringList>


bool parseStatus(const QByteArray &message, bool &online)
{
    const QByteArray text = message.trimmed();

    if (text == "online") {
        online = true;
        return true;
    }
    if (text == "offline") {
        online = false;
        return true;
    }

    qWarning() << "MQTT: status inconnu :" << message;
    return false;
}
bool  parseMeasurement(const QByteArray &message, Measurement &out)
{
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(message, &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "MQTT: JSON invalide :" << err.errorString();
        return false;
    }

    const QJsonObject o = doc.object();
    static const QStringList keys = {"voltage", "current", "power", "temperature"};
    for (const QString &key : keys) {
        if (!o.contains(key) || !o.value(key).isDouble()) {
            qWarning() << "MQTT: clé manquante ou invalide :" << key;
            return false;
        }
    }

    out.voltage = o.value("voltage").toDouble();
    out.current = o.value("current").toDouble();
    out.power = o.value("power").toDouble();
    out.temperature = o.value("temperature").toDouble();
    return true;
}