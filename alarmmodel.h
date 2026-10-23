#ifndef ALARMMODEL_H
#define ALARMMODEL_H

#pragma once

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

#include "alarmmanager.h"
#include "appsettings.h"
#include "datasource.h"

class AlarmModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(DataSource*source READ source WRITE setsource
                   NOTIFY sourceChanged)
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

    DataSource*source() const;
    void setsource(DataSource*source);

    AppSettings *settings() const;
    void setSettings(AppSettings *settings);

    int activeCount() const;

    Q_INVOKABLE void acknowledge(int row);

signals:
    void sourceChanged();
    void settingsChanged();
    void activeCountChanged();

private slots:
    void onMeasurements();
    void rebuildRules();

private:
    AlarmManager m_manager;
    QList<AlarmEvent> m_events;
    DataSource*m_source = nullptr;
    AppSettings *m_settings = nullptr;
};
#endif // ALARMMODEL_H
