#ifndef DATASOURCE_H
#define DATASOURCE_H

#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

class DataSource : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("DataSource est une classe de base abstraite")
    Q_PROPERTY(double voltage READ voltage NOTIFY measurementsChanged)
    Q_PROPERTY(double current READ current NOTIFY measurementsChanged)
    Q_PROPERTY(double power READ power NOTIFY measurementsChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY measurementsChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

public:
    double voltage() const { return m_voltage; }
    double current() const { return m_current; }
    double power() const { return m_power; }
    double temperature() const { return m_temperature; }
    bool connected() const { return m_connected; }

signals:
    void measurementsChanged();
    void connectedChanged();

protected:
    explicit DataSource(QObject *parent = nullptr) : QObject(parent) {}

    void setConnected(bool value)
    {
        if (m_connected == value)
            return;
        m_connected = value;
        emit connectedChanged();
    }

    double m_voltage = 230.0;     // V
    double m_current = 10.0;      // A
    double m_power = 2.3;         // kW
    double m_temperature = 55.0;  // °C
    bool m_connected = true;
};

#endif // DATASOURCE_H
