#include "sensorsimulator.h"

#include <QRandomGenerator>
#include <algorithm>

namespace {
// nombre aléatoire entre min et max
double randomRange(double min, double max)
{
    return min + QRandomGenerator::global()->generateDouble() * (max - min);
}
}

SensorSimulator::SensorSimulator(QObject *parent)
    : QObject(parent)
{
    m_timer.setInterval(100);
    connect(&m_timer, &QTimer::timeout, this, &SensorSimulator::tick);
    m_timer.start();
}
void SensorSimulator::injectOvervoltage()
{
    m_overvoltageTicks = 30;      // 30 ticks x 100 ms = 3 s
}

void SensorSimulator::injectOverheat()
{
    m_temperature += 40.0;        // saut brutal, puis refroidissement par inertie
}

void SensorSimulator::tick()
{
    // Tension : 230 V avec un peu de bruit
    m_voltage = 230.0 + randomRange(-2.0, 2.0);

    // Défaut injecté : +40 V pendant 30 ticks
    if (m_overvoltageTicks > 0) {
        m_voltage += 40.0;
        --m_overvoltageTicks;
    }

    // Courant : la charge varie lentement entre 8 et 12 A, plus un léger bruit
    m_load = std::clamp(m_load + randomRange(-0.2, 0.2), 8.0, 12.0);
    m_current = m_load + randomRange(-0.1, 0.1);

    // Puissance : tension x courant, convertie en kW
    m_power = m_voltage * m_current / 1000.0;

    // Température : se rapproche lentement d'une cible qui dépend du courant
    const double target = 40.0 + 1.5 * m_current; // 10 A -> 55 °C
    m_temperature += (target - m_temperature) * 0.01;

    emit measurementsChanged();
}