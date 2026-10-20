#include "alarmrule.h"

AlarmRule::AlarmRule(double warning, double critical, double hysteresis)
    : m_warning(warning)
    , m_critical(critical)
    , m_hysteresis(hysteresis)
{
}

AlarmLevel AlarmRule::level() const
{
    return m_level;
}

AlarmLevel AlarmRule::evaluate(double value)
{
    switch (m_level) {
    case AlarmLevel::Normal:
        if (value >= m_critical)
            m_level = AlarmLevel::Critical;
        else if (value >= m_warning)
            m_level = AlarmLevel::Warning;
        break;

    case AlarmLevel::Warning:
        if (value >= m_critical)
            m_level = AlarmLevel::Critical;
        else if (value < m_warning - m_hysteresis)
            m_level = AlarmLevel::Normal;
        break;

    case AlarmLevel::Critical:
        if (value < m_critical - m_hysteresis) {
            if (value >= m_warning - m_hysteresis)
                m_level = AlarmLevel::Warning;
            else
                m_level = AlarmLevel::Normal;
        }
        break;
    }

    return m_level;
}