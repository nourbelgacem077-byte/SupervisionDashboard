#include "datalogger.h"
#include"dbpath.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

DataLogger::DataLogger(QObject *parent)
    : QObject(parent)
{
    // 1. dossier de la base (créé s'il n'existe pas)
    const QString path = databasePath();
    qDebug() << "Base de données :" << path;

    // 2. worker SANS parent (un parent empêcherait moveToThread)
    m_worker = new DbWorker(path);

    // 3. déménagement vers le thread de travail
    m_worker->moveToThread(&m_thread);

    // 4. connexions (avant le démarrage du thread)
    //    a) le thread démarre -> le worker ouvre la base, dans son thread
    connect(&m_thread, &QThread::started, m_worker, &DbWorker::open);
    //    b) le thread se termine -> le worker se supprime, dans son thread
    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    //    c) un échantillon est prêt -> le worker l'insère
    connect(this, &DataLogger::sampleReady, m_worker, &DbWorker::insertSample);

    // 5. démarrage en dernier
    m_thread.start();
}

DataLogger::~DataLogger()
{
    m_thread.quit();   // demande l'arrêt de la boucle d'événements
    m_thread.wait();   // attend que le thread soit réellement terminé
}

SensorSimulator *DataLogger::simulator() const
{
    return m_simulator;
}

void DataLogger::setSimulator(SensorSimulator *simulator)
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
                this, &DataLogger::onMeasurements);
    }

    emit simulatorChanged();
}

void DataLogger::onMeasurements()
{
    if (!m_simulator) {
        return;
    }

    ++m_counter;
    if (m_counter % 10 != 0) {
        return;   // une mesure sur dix : une par seconde
    }

    emit sampleReady(QDateTime::currentMSecsSinceEpoch(),
                     m_simulator->voltage(),
                     m_simulator->current(),
                     m_simulator->power(),
                     m_simulator->temperature());
}