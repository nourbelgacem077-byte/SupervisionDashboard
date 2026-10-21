#include "datalogger.h"
#include"dbpath.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

DataLogger::DataLogger(QObject *parent)
    : QObject(parent)
{

    const QString path = databasePath();
    qDebug() << "Base de données :" << path;

    // Worker
    m_worker = new DbWorker(path);


    m_worker->moveToThread(&m_thread);

    //  Thread connexions
    //     Thread started
    connect(&m_thread, &QThread::started, m_worker, &DbWorker::open);
    //    Thread Finished
    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    //    Insert worker
    connect(this, &DataLogger::sampleReady, m_worker, &DbWorker::insertSample);

    m_thread.start();
}

DataLogger::~DataLogger()
{
    m_thread.quit();
    m_thread.wait();
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
        return;   // 1/10 mesure per 1sec
    }

    emit sampleReady(QDateTime::currentMSecsSinceEpoch(),
                     m_simulator->voltage(),
                     m_simulator->current(),
                     m_simulator->power(),
                     m_simulator->temperature());
}