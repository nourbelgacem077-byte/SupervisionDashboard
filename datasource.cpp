#include "datasource.h"
#include <QRandomGenerator>
#include <algorithm>

namespace {

double randomRange(double min, double max)
{
    return min + QRandomGenerator::global()->generateDouble() * (max - min);
}
}

DataSource::DataSource()(QObject *parent)
    : QObject(parent)
{
    m_timer.setInterval(100);
    connect(&m_timer, &QTimer::timeout, this, &DataSource::tick);
    m_timer.start();
}
void DataSource::injectOvervoltage()
{
    m_overvoltageTicks = 30;      // 30 ticks x 100 ms = 3 s
}

void DataSource::injectOverheat()
{
    m_temperature += 40.0;
}

void DataSource::tick()
{

    m_voltage = 230.0 + randomRange(-2.0, 2.0);


    if (m_overvoltageTicks > 0) {
        m_voltage += 40.0;
        --m_overvoltageTicks;
    }


    m_load = std::clamp(m_load + randomRange(-0.2, 0.2), 8.0, 12.0);
    m_current = m_load + randomRange(-0.1, 0.1);

    m_power = m_voltage * m_current / 1000.0;


    const double target = 40.0 + 1.5 * m_current; // 10 A -> 55 °C
    m_temperature += (target - m_temperature) * 0.01;

    emit measurementsChanged();
}