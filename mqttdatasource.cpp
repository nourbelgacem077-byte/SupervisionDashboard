#include "mqttdatasource.h"

#include "mqttdatasource.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMqttTopicFilter>
#include <QMqttTopicName>

namespace {
//const char *kTopicFilter = "supervision/device1/#";
const QString kBaseTopic = QStringLiteral("supervision/device1");
const QString kTopicFilter = kBaseTopic + QStringLiteral("/#");
const QString kCmdTopic = kBaseTopic + QStringLiteral("/cmd");
const int kStaleMs = 3000;
}
MqttDataSource::MqttDataSource(QObject *parent)
    : DataSource(parent)
{
    connect(&m_client, &QMqttClient::connected,
            this, &MqttDataSource::onConnected);
    connect(&m_client, &QMqttClient::stateChanged,
            this, &MqttDataSource::onStateChanged);
    connect(&m_client, &QMqttClient::messageReceived,
            this, &MqttDataSource::onMessage);

    m_watchdog.setInterval(1000);
    connect(&m_watchdog, &QTimer::timeout, this, &MqttDataSource::updateState);
    m_watchdog.start();

    updateState();
}

QString MqttDataSource::host() const
{
    return m_host;
}

void MqttDataSource::setHost(const QString &host)
{
    if (m_host == host) {
        return;
    }
    m_host = host;
    emit hostChanged();
}

int MqttDataSource::port() const
{
    return m_port;
}

void MqttDataSource::setPort(int port)
{
    if (m_port == port) {
        return;
    }
    m_port = port;
    emit portChanged();
}

void MqttDataSource::start()
{
    qDebug() << "MQTT start" << m_host << m_port;

    if (m_client.state() != QMqttClient::Disconnected) {
        m_client.disconnectFromHost();
    }

    m_client.setHostname(m_host);
    m_client.setPort(static_cast<quint16>(m_port));
    m_client.connectToHost();
}
void MqttDataSource::stop()
{
    m_client.disconnectFromHost();
}

void MqttDataSource::onConnected()
{
    qDebug() << "MQTT connected, subscribing to" << kTopicFilter;
    auto *sub = m_client.subscribe(QMqttTopicFilter(kTopicFilter));
    if (!sub) {
        qWarning() << "MQTT: abonnement impossible";
    }
}

void MqttDataSource::onStateChanged()
{
    qDebug() << "MQTT state" << m_client.state();

    if (m_client.state() != QMqttClient::Connected) {
        m_deviceOnline = false;
        m_sinceLast.invalidate();
    }
    updateState();
}

void MqttDataSource::onMessage(const QByteArray &message,
                               const QMqttTopicName &topic)
{
    qDebug() << "MQTT message" << topic.name();
    const QString name = topic.name();

    if (name.endsWith("/measurements")) {
        Measurement m;
        if (!parseMeasurement(message, m)) {
            return;
        }
        m_voltage = m.voltage;
        m_current = m.current;
        m_power = m.power;
        m_temperature = m.temperature;
        m_sinceLast.restart();
        emit measurementsChanged();
        updateState();
    }     else if (name.endsWith("/status")) {
    bool online = false;
    if (parseStatus(message, online)) {
        m_deviceOnline = online;
        updateState();
    }
    }
}
bool MqttDataSource::parseStatus(const QByteArray &message, bool &online)
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
bool MqttDataSource::parseMeasurement(const QByteArray &message, Measurement &out)
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

void MqttDataSource::sendCommand(const QByteArray &command)
{
    if (m_client.state() != QMqttClient::Connected) {
        qWarning() << "MQTT: commande ignorée, non connecté";
        return;
    }
    m_client.publish(QMqttTopicName(kCmdTopic), command);
}

void MqttDataSource::injectOvervoltage()
{
    sendCommand("overvoltage");
}

void MqttDataSource::injectOverheat()
{
    sendCommand("overheat");
}
void MqttDataSource::updateState()
{
    bool live = false;
    QString text;

    if (m_client.state() == QMqttClient::Connecting) {
        text = "Connecting...";
    } else if (m_client.state() != QMqttClient::Connected) {
        text = "Broker unreachable";
    } else if (!m_deviceOnline) {
        text = "Device offline";
    } else if (!m_sinceLast.isValid() || m_sinceLast.elapsed() > kStaleMs) {
        text = "No data";
    } else {
        live = true;
        text = "Live";
    }

    setConnected(live);
    setStatusText(text);
}