#ifndef DATALOGGER_H
#define DATALOGGER_H

#pragma once

#include <QObject>
#include <QThread>
#include <QtQml/qqmlregistration.h>

#include "dbworker.h"
#include "sensorsimulator.h"

class DataLogger : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(SensorSimulator *simulator READ simulator WRITE setSimulator
                   NOTIFY simulatorChanged)

public:
    explicit DataLogger(QObject *parent = nullptr);
    ~DataLogger() override;

    SensorSimulator *simulator() const;
    void setSimulator(SensorSimulator *simulator);

signals:
    void simulatorChanged();
    void sampleReady(qint64 timestampMs, double voltage, double current,
                     double power, double temperature);

private slots:
    void onMeasurements();

private:
    QThread m_thread;
    DbWorker *m_worker = nullptr;
    SensorSimulator *m_simulator = nullptr;
    int m_counter = 0;
};

#endif // DATALOGGER_H
