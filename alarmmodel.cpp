#include "alarmmodel.h"

AlarmModel::AlarmModel(QObject *parent)
    : QAbstractListModel(parent)
{


    connect(&m_manager, &AlarmManager::alarmRaised, this,
            [this](const AlarmEvent &event) {
                beginInsertRows(QModelIndex(), 0, 0);
                m_events.prepend(event);
                endInsertRows();
            });


    connect(&m_manager, &AlarmManager::eventsChanged,
            this, &AlarmModel::activeCountChanged);
}

int AlarmModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_events.size());
}

QVariant AlarmModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_events.size()) {
        return QVariant();
    }

    const AlarmEvent &e = m_events.at(index.row());

    switch (role) {
    case QuantityRole:
        return e.quantity;
    case LevelRole:
        return static_cast<int>(e.level);
    case ValueRole:
        return e.value;
    case TimeRole:
        return e.time.toString("HH:mm:ss");
    case AcknowledgedRole:
        return e.acknowledged;
    }
    return QVariant();
}

QHash<int, QByteArray> AlarmModel::roleNames() const
{
    return {
        {QuantityRole, "quantity"},
        {LevelRole, "level"},
        {ValueRole, "value"},
        {TimeRole, "time"},
        {AcknowledgedRole, "acknowledged"}
    };
}

DataSource*AlarmModel::source() const
{
    return m_source;
}

void AlarmModel::setsource(DataSource*source)
{
    if (m_source == source) {
        return;
    }

    if (m_source) {
        disconnect(m_source, nullptr, this, nullptr);
    }

    m_source = source;

    if (m_source) {
        connect(m_source, &DataSource::measurementsChanged,
                this, &AlarmModel::onMeasurements);
    }

    emit sourceChanged();
}

AppSettings *AlarmModel::settings() const
{
    return m_settings;
}

void AlarmModel::setSettings(AppSettings *settings)
{
    if (m_settings == settings) {
        return;
    }

    if (m_settings) {
        disconnect(m_settings, nullptr, this, nullptr);
    }

    m_settings = settings;

    if (m_settings) {
        connect(m_settings, &AppSettings::thresholdsChanged,
                this, &AlarmModel::rebuildRules);
        rebuildRules();
    }

    emit settingsChanged();
}

void AlarmModel::rebuildRules()
{
    if (!m_settings) {
        return;
    }


    const QStringList names = m_settings->quantities();
    for (const QString &name : names) {
        m_manager.addRule(name,
                          m_settings->warning(name),
                          m_settings->critical(name),
                          m_settings->hysteresis(name));
    }
}

int AlarmModel::activeCount() const
{
    return m_manager.activeCount();
}

void AlarmModel::onMeasurements()
{
    if (!m_source) {
        return;
    }

    m_manager.processMeasurement("Tension", m_source->voltage());
    m_manager.processMeasurement("Courant", m_source->current());
    m_manager.processMeasurement("Puissance", m_source->power());
    m_manager.processMeasurement("Température", m_source->temperature());
}

void AlarmModel::acknowledge(int row)
{
    if (row < 0 || row >= m_events.size()) {
        return;
    }


    const int managerIndex = static_cast<int>(m_events.size()) - 1 - row;
    m_manager.acknowledge(managerIndex);

    m_events[row].acknowledged = true;
    emit dataChanged(index(row), index(row), {AcknowledgedRole});
}