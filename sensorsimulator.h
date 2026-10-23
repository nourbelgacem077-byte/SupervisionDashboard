#ifndef SENSORSIMULATOR_H
#define SENSORSIMULATOR_H

#pragma once

#include <QObject>
#include <QTimer>
#include <QtQml/qqmlregistration.h>
#include "datasource.h"

class SensorSimulator : public DataSource
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit SensorSimulator(QObject *parent = nullptr);
    void injectOvervoltage() override;
    void injectOverheat() override;

signals:
    void measurementsChanged();

private slots:
    void tick();

private:
    QTimer m_timer;
    double m_load = 10.0;
    int m_overvoltageTicks = 0;

};

#endif // SENSORSIMULATOR_H
