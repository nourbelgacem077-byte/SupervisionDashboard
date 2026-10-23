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
    Q_PROPERTY(QString sourceType READ sourceType NOTIFY sourceChanged)
    Q_PROPERTY(QString mqttHost READ mqttHost NOTIFY sourceChanged)
    Q_PROPERTY(int mqttPort READ mqttPort NOTIFY sourceChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);

    Q_INVOKABLE QStringList quantities() const;
    Q_INVOKABLE double warning(const QString &quantity) const;
    Q_INVOKABLE double critical(const QString &quantity) const;
    Q_INVOKABLE double hysteresis(const QString &quantity) const;

    QString sourceType() const { return m_sourceType; }
    QString mqttHost() const { return m_mqttHost; }
    int mqttPort() const { return m_mqttPort; }

    // Retourne false (sans rien changer) si la grandeur est inconnue,
    // si warningLevel >= criticalLevel, ou si hyst < 0
    Q_INVOKABLE bool setThresholds(const QString &quantity, double warningLevel,
                                   double criticalLevel, double hyst);

    // Retourne false (sans rien changer) si type n'est pas "simulator"/"mqtt",
    // si host est vide, ou si port n'est pas dans 1..65535
    Q_INVOKABLE bool setSource(const QString &type, const QString &host, int port);
    Q_INVOKABLE void resetDefaults();

signals:
    void thresholdsChanged();
    void sourceChanged();

private:
    struct Limits
    {
        double warning = 0.0;
        double critical = 0.0;
        double hysteresis = 0.0;
    };

    static QString keyId(const QString &quantity);
    static Limits defaultsFor(const QString &quantity);
    static bool isValidSource(const QString &type, const QString &host, int port);
    void save(const QString &quantity);

    QSettings m_store;
    QHash<QString, Limits> m_limits;

    QString m_sourceType = "simulator";
    QString m_mqttHost = "localhost";
    int m_mqttPort = 1883;
};

#endif // APPSETTINGS_H