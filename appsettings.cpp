#include "appsettings.h"

namespace {
// Les quatre grandeurs, dans l'ordre d'affichage
const QStringList &allQuantities()
{
    static const QStringList list = {"Tension", "Courant", "Puissance", "Température"};
    return list;
}
}

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
    , m_store(QSettings::IniFormat, QSettings::UserScope,
              "SupervisionDashboard", "settings")
{
    for (const QString &quantity : allQuantities()) {
        const Limits defaults = defaultsFor(quantity);
        const QString id = keyId(quantity);

        Limits limits;
        limits.warning = m_store.value(QString("alarms/%1/warning").arg(id),
                                       defaults.warning).toDouble();
        limits.critical = m_store.value(QString("alarms/%1/critical").arg(id),
                                        defaults.critical).toDouble();
        limits.hysteresis = m_store.value(QString("alarms/%1/hysteresis").arg(id),
                                          defaults.hysteresis).toDouble();

        // fichier modifié à la main et devenu incohérent : on revient aux défauts
        if (limits.warning >= limits.critical || limits.hysteresis < 0.0) {
            limits = defaults;
        }

        m_limits.insert(quantity, limits);
    }
}

QStringList AppSettings::quantities() const
{
    return allQuantities();
}

double AppSettings::warning(const QString &quantity) const
{
    return m_limits.value(quantity).warning;       // 0.0 si grandeur inconnue
}

double AppSettings::critical(const QString &quantity) const
{
    return m_limits.value(quantity).critical;
}

double AppSettings::hysteresis(const QString &quantity) const
{
    return m_limits.value(quantity).hysteresis;
}

bool AppSettings::setThresholds(const QString &quantity, double warningLevel,
                                double criticalLevel, double hyst)
{
    // 1. toutes les vérifications d'abord
    if (!m_limits.contains(quantity)) {
        return false;
    }
    if (warningLevel >= criticalLevel || hyst < 0.0) {
        return false;
    }

    // 2. ensuite seulement, les modifications
    Limits limits;
    limits.warning = warningLevel;
    limits.critical = criticalLevel;
    limits.hysteresis = hyst;

    m_limits.insert(quantity, limits);
    save(quantity);

    emit thresholdsChanged();
    return true;
}

void AppSettings::resetDefaults()
{
    for (const QString &quantity : allQuantities()) {
        m_limits.insert(quantity, defaultsFor(quantity));
        save(quantity);
    }
    emit thresholdsChanged();   // un seul signal, à la fin
}

QString AppSettings::keyId(const QString &quantity)
{
    if (quantity == "Tension") {
        return "voltage";
    }
    if (quantity == "Courant") {
        return "current";
    }
    if (quantity == "Puissance") {
        return "power";
    }
    if (quantity == "Température") {
        return "temperature";
    }
    return QString();
}

AppSettings::Limits AppSettings::defaultsFor(const QString &quantity)
{
    Limits limits;

    if (quantity == "Tension") {
        limits.warning = 250.0;
        limits.critical = 260.0;
        limits.hysteresis = 3.0;
    } else if (quantity == "Courant") {
        limits.warning = 14.0;
        limits.critical = 16.0;
        limits.hysteresis = 0.5;
    } else if (quantity == "Puissance") {
        limits.warning = 3.2;
        limits.critical = 3.7;
        limits.hysteresis = 0.1;
    } else if (quantity == "Température") {
        limits.warning = 75.0;
        limits.critical = 90.0;
        limits.hysteresis = 5.0;
    }

    return limits;
}

void AppSettings::save(const QString &quantity)
{
    const Limits limits = m_limits.value(quantity);
    const QString id = keyId(quantity);

    m_store.setValue(QString("alarms/%1/warning").arg(id), limits.warning);
    m_store.setValue(QString("alarms/%1/critical").arg(id), limits.critical);
    m_store.setValue(QString("alarms/%1/hysteresis").arg(id), limits.hysteresis);
    m_store.sync();   // force l'écriture sur le disque
}