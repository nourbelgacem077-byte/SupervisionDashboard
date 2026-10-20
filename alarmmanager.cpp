#include "alarmmanager.h"

AlarmManager::AlarmManager(QObject *parent)
    : QObject(parent)
{
}

void AlarmManager::addRule(const QString &quantity, double warning,
                           double critical, double hysteresis)
{
    m_rules.insert(quantity, AlarmRule(warning, critical, hysteresis));
}

void AlarmManager::processMeasurement(const QString &quantity, double value)
{
    auto it = m_rules.find(quantity);
    if (it == m_rules.end()) {
        return;
    }

    const AlarmLevel before = it.value().level();
    const AlarmLevel after = it.value().evaluate(value);

    if (after != before && after != AlarmLevel::Normal) {
        AlarmEvent event;
        event.time = QDateTime::currentDateTime();
        event.quantity = quantity;
        event.level = after;
        event.value = value;

        m_events.append(event);
        emit alarmRaised(event);
        emit eventsChanged();
    }
}

const QList<AlarmEvent> &AlarmManager::events() const
{
    return m_events;
}

int AlarmManager::activeCount() const
{
    int count = 0;
    for (const AlarmEvent &e : m_events) {
        if (!e.acknowledged) {
            ++count;
        }
    }
    return count;
}

void AlarmManager::acknowledge(int index)
{
    if (index < 0 || index >= m_events.size()) {
        return;
    }

    m_events[index].acknowledged = true;
    emit eventsChanged();
}