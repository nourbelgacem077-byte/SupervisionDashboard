#ifndef APPSETTINGS_H
#define APPSETTINGS_H
#pragma once

#include <QHash>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

class AppSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit AppSettings(QObject *parent = nullptr);

    Q_INVOKABLE QStringList quantities() const;
    Q_INVOKABLE double warning(const QString &quantity) const;
    Q_INVOKABLE double critical(const QString &quantity) const;
    Q_INVOKABLE double hysteresis(const QString &quantity) const;

    // Retourne false (sans rien changer) si la grandeur est inconnue,
    // si warningLevel >= criticalLevel, ou si hyst < 0
    Q_INVOKABLE bool setThresholds(const QString &quantity, double warningLevel,
                                   double criticalLevel, double hyst);
    Q_INVOKABLE void resetDefaults();

signals:
    void thresholdsChanged();

private:
    struct Limits
    {
        double warning = 0.0;
        double critical = 0.0;
        double hysteresis = 0.0;
    };

    static QString keyId(const QString &quantity);
    static Limits defaultsFor(const QString &quantity);
    void save(const QString &quantity);

    QSettings m_store;
    QHash<QString, Limits> m_limits;
};

#endif // APPSETTINGS_H
