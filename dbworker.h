#ifndef DBWORKER_H
#define DBWORKER_H

#pragma once

#include <QObject>
#include <QString>

class DbWorker : public QObject
{
    Q_OBJECT

public:
    explicit DbWorker(const QString &dbPath, QObject *parent = nullptr);
    ~DbWorker() override;

public slots:
    bool open();
    void insertSample(qint64 timestampMs, double voltage, double current,
                      double power, double temperature);
    void flush();
    int sampleCount();

private:
    QString m_path;
    QString m_connectionName = "supervision_conn";
    int m_pending = 0;
};
#endif // DBWORKER_H
