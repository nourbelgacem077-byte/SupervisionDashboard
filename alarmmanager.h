#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#pragma once

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

#include "alarmrule.h"

struct AlarmEvent
{
    QDateTime time;
    QString quantity;
    AlarmLevel level = AlarmLevel::Normal;
    double value = 0.0;
    bool acknowledged = false;
};

class AlarmManager : public QObject
{
    Q_OBJECT

public:
    explicit AlarmManager(QObject *parent = nullptr);

    void addRule(const QString &quantity, double warning,
                 double critical, double hysteresis);
    void processMeasurement(const QString &quantity, double value);

    const QList<AlarmEvent> &events() const;
    int activeCount() const;
    void acknowledge(int index);

signals:
    void alarmRaised(const AlarmEvent &event);
    void eventsChanged();

private:
    QHash<QString, AlarmRule> m_rules;
    QList<AlarmEvent> m_events;
};

#endif // ALARMMANAGER_H
