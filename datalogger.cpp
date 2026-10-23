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

DataSource*DataLogger::source() const
{
    return m_source;
}

void DataLogger::setsource(DataSource*source)
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
                this, &DataLogger::onMeasurements);
    }

    emit sourceChanged();
}

void DataLogger::onMeasurements()
{
    if (!m_source) {
        return;
    }

    ++m_counter;
    if (m_counter % 10 != 0) {
        return;   // 1/10 mesure per 1sec
    }

    emit sampleReady(QDateTime::currentMSecsSinceEpoch(),
                     m_source->voltage(),
                     m_source->current(),
                     m_source->power(),
                     m_source->temperature());
}