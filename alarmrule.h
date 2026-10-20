#ifndef ALARMRULE_H
#define ALARMRULE_H

#pragma once

enum class AlarmLevel { Normal, Warning, Critical };

class AlarmRule
{
public:
    AlarmRule(double warning, double critical, double hysteresis);

    AlarmLevel evaluate(double value);
    AlarmLevel level() const;

private:
    double m_warning;
    double m_critical;
    double m_hysteresis;
    AlarmLevel m_level = AlarmLevel::Normal;
};
#endif // ALARMRULE_H
