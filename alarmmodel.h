#ifndef ALARMMODEL_H
#define ALARMMODEL_H

#pragma once

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

#include "alarmmanager.h"
#include "appsettings.h"
#include "sensorsimulator.h"

class AlarmModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(SensorSimulator *simulator READ simulator WRITE setSimulator
                   NOTIFY simulatorChanged)
    Q_PROPERTY(AppSettings *settings READ settings WRITE setSettings
                   NOTIFY settingsChanged)
    Q_PROPERTY(int activeCount READ activeCount NOTIFY activeCountChanged)

public:
    enum Roles {
        QuantityRole = Qt::UserRole + 1,
        LevelRole,
        ValueRole,
        TimeRole,
        AcknowledgedRole
    };

    explicit AlarmModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    SensorSimulator *simulator() const;
    void setSimulator(SensorSimulator *simulator);

    AppSettings *settings() const;
    void setSettings(AppSettings *settings);

    int activeCount() const;

    Q_INVOKABLE void acknowledge(int row);

signals:
    void simulatorChanged();
    void settingsChanged();
    void activeCountChanged();

private slots:
    void onMeasurements();
    void rebuildRules();

private:
    AlarmManager m_manager;
    QList<AlarmEvent> m_events;
    SensorSimulator *m_simulator = nullptr;
    AppSettings *m_settings = nullptr;
};
#endif // ALARMMODEL_H
