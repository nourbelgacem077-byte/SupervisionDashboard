#include "dbworker.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>


DbWorker::DbWorker(const QString &dbPath, QObject *parent)
    : QObject(parent)
    , m_path(dbPath)
{
}

DbWorker::~DbWorker()
{
    flush();

    if (QSqlDatabase::contains(m_connectionName)) {
        {

            QSqlDatabase db = QSqlDatabase::database(m_connectionName);
            db.close();
        }
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool DbWorker::open()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(m_path);

    if (!db.open()) {
        qWarning() << "Ouverture impossible :" << db.lastError().text();
        return false;
    }

    QSqlQuery q(db);
    if (!q.exec("PRAGMA journal_mode=WAL")) {
        qWarning() << "WAL non activé :" << q.lastError().text();
    }

    if (!q.exec("CREATE TABLE IF NOT EXISTS measurements ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "ts INTEGER NOT NULL, "
                "voltage REAL, "
                "current REAL, "
                "power REAL, "
                "temperature REAL)")) {
        qWarning() << "Création de la table impossible :" << q.lastError().text();
        return false;
    }

    if (!q.exec("CREATE INDEX IF NOT EXISTS idx_measurements_ts "
                "ON measurements(ts)")) {
        qWarning() << "Création de l'index impossible :" << q.lastError().text();
        return false;
    }

    return true;
}

void DbWorker::insertSample(qint64 timestampMs, double voltage, double current,
                            double power, double temperature)
{

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);


    if (m_pending == 0) {
        if (!db.transaction()) {
            qWarning() << "Transaction impossible :" << db.lastError().text();
        }
    }

    QSqlQuery q(db);
    q.prepare("INSERT INTO measurements (ts, voltage, current, power, temperature) "
              "VALUES (?, ?, ?, ?, ?)");
    q.addBindValue(timestampMs);
    q.addBindValue(voltage);
    q.addBindValue(current);
    q.addBindValue(power);
    q.addBindValue(temperature);

    if (!q.exec()) {
        qWarning() << "Insertion impossible :" << q.lastError().text();
    }

    ++m_pending;

    if (m_pending >= 20) {
        db.commit();
        m_pending = 0;
    }
}

void DbWorker::flush()
{
    if (m_pending == 0) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.commit();
    m_pending = 0;
}

int DbWorker::sampleCount()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);

    if (q.exec("SELECT COUNT(*) FROM measurements") && q.next()) {
        return q.value(0).toInt();
    }
    return 0;
}