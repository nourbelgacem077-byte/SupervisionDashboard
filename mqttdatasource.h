#ifndef MQTTDATASOURCE_H
#define MQTTDATASOURCE_H


#pragma once

#include <QMqttClient>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include "datasource.h"

class MqttDataSource : public DataSource
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

public:
    explicit MqttDataSource(QObject *parent = nullptr);

    QString host() const;
    void setHost(const QString &host);

    int port() const;
    void setPort(int port);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    struct Measurement
    {
        double voltage = 0.0;
        double current = 0.0;
        double power = 0.0;
        double temperature = 0.0;
    };

    // Statique : testable sans broker (étape 5.6)
    static bool parseMeasurement(const QByteArray &message, Measurement &out);

signals:
    void hostChanged();
    void portChanged();

private slots:
    void onConnected();
    void onStateChanged();
    void onMessage(const QByteArray &message, const QMqttTopicName &topic);

private:
    QMqttClient m_client;
    QString m_host = "localhost";
    int m_port = 1883;
};


#endif // MQTTDATASOURCE_H
