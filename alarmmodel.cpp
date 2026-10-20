#include "alarmmodel.h"

AlarmModel::AlarmModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // plus aucun seuil écrit ici : ils viennent de AppSettings (setSettings)

    // nouvel événement : on l'insère en ligne 0 (le plus récent en haut)
    connect(&m_manager, &AlarmManager::alarmRaised, this,
            [this](const AlarmEvent &event) {
                beginInsertRows(QModelIndex(), 0, 0);
                m_events.prepend(event);
                endInsertRows();
            });

    // le manager signale un changement : le compteur d'alarmes actives aussi
    connect(&m_manager, &AlarmManager::eventsChanged,
            this, &AlarmModel::activeCountChanged);
}

int AlarmModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;   // liste plate : aucun élément n'a d'enfants
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

SensorSimulator *AlarmModel::simulator() const
{
    return m_simulator;
}

void AlarmModel::setSimulator(SensorSimulator *simulator)
{
    if (m_simulator == simulator) {
        return;
    }

    if (m_simulator) {
        disconnect(m_simulator, nullptr, this, nullptr);
    }

    m_simulator = simulator;

    if (m_simulator) {
        connect(m_simulator, &SensorSimulator::measurementsChanged,
                this, &AlarmModel::onMeasurements);
    }

    emit simulatorChanged();
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
        rebuildRules();   // les règles existent dès le branchement
    }

    emit settingsChanged();
}

void AlarmModel::rebuildRules()
{
    if (!m_settings) {
        return;
    }

    // addRule remplace la règle existante de même nom
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
    if (!m_simulator) {
        return;
    }

    m_manager.processMeasurement("Tension", m_simulator->voltage());
    m_manager.processMeasurement("Courant", m_simulator->current());
    m_manager.processMeasurement("Puissance", m_simulator->power());
    m_manager.processMeasurement("Température", m_simulator->temperature());
}

void AlarmModel::acknowledge(int row)
{
    if (row < 0 || row >= m_events.size()) {
        return;
    }

    // ligne 0 du modèle = dernier élément du manager
    const int managerIndex = static_cast<int>(m_events.size()) - 1 - row;
    m_manager.acknowledge(managerIndex);

    m_events[row].acknowledged = true;
    emit dataChanged(index(row), index(row), {AcknowledgedRole});
}