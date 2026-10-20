#ifndef SENSORSIMULATOR_H
#define SENSORSIMULATOR_H

#pragma once

#include <QObject>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

class SensorSimulator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(double voltage READ voltage NOTIFY measurementsChanged)
    Q_PROPERTY(double current READ current NOTIFY measurementsChanged)
    Q_PROPERTY(double power READ power NOTIFY measurementsChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY measurementsChanged)

public:
    explicit SensorSimulator(QObject *parent = nullptr);
    Q_INVOKABLE void injectOvervoltage();
    Q_INVOKABLE void injectOverheat();
    double voltage() const { return m_voltage; }
    double current() const { return m_current; }
    double power() const { return m_power; }
    double temperature() const { return m_temperature; }

signals:
    void measurementsChanged();

private slots:
    void tick();

private:
    QTimer m_timer;
    double m_voltage = 230.0;  // V
    double m_load = 10.0;
    double m_current = 10.0;   // A
    double m_power = 2.3;      // kW
    double m_temperature = 55.0; // °C
    int m_overvoltageTicks = 0;

};

#endif // SENSORSIMULATOR_H
