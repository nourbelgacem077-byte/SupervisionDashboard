#include "historyreader.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QPointF>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextStream>

#include "dbpath.h"

HistoryReader::HistoryReader(QObject *parent)
    : QObject(parent)
{
}

// Liste blanche : seuls ces noms peuvent entrer dans une requête SQL
QString HistoryReader::columnFor(const QString &quantity)
{
    if (quantity == "Tension") {
        return "voltage";
    }
    if (quantity == "Courant") {
        return "current";
    }
    if (quantity == "Puissance") {
        return "power";
    }
    if (quantity == "Température") {
        return "temperature";
    }
    return QString();   // grandeur inconnue
}

QVariantList HistoryReader::query(const QString &quantity, qint64 fromMs,
                                  qint64 toMs, int maxPoints)
{
    const QString column = columnFor(quantity);
    if (column.isEmpty()) {
        return QVariantList();
    }

    if (maxPoints < 1) {
        maxPoints = 1;
    }

    QVariantList points;

    {   // db et les requêtes vivent seulement dans ce bloc
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        db.setDatabaseName(databasePath());

        if (!db.open()) {
            qWarning() << "Lecture impossible :" << db.lastError().text();
        } else {
            QSqlQuery countQuery(db);
            countQuery.prepare("SELECT COUNT(*) FROM measurements "
                               "WHERE ts BETWEEN ? AND ?");
            countQuery.addBindValue(fromMs);
            countQuery.addBindValue(toMs);

            int count = 0;
            if (countQuery.exec() && countQuery.next()) {
                count = countQuery.value(0).toInt();
            } else {
                qWarning() << "Comptage impossible :" << countQuery.lastError().text();
            }

            if (count > 0) {
                const int step = (count + maxPoints - 1) / maxPoints;

                QSqlQuery q(db);
                q.prepare(QString("SELECT ts, %1 FROM measurements "
                                  "WHERE ts BETWEEN ? AND ? ORDER BY ts")
                              .arg(column));
                q.addBindValue(fromMs);
                q.addBindValue(toMs);

                if (q.exec()) {
                    int index = 0;
                    qint64 firstTs = 0;

                    while (q.next()) {
                        if (index % step == 0) {
                            const qint64 ts = q.value(0).toLongLong();
                            if (points.isEmpty()) {
                                firstTs = ts;
                            }
                            const double x = (ts - firstTs) / 60000.0;   // minutes
                            const double y = q.value(1).toDouble();
                            points.append(QPointF(x, y));
                        }
                        ++index;
                    }
                } else {
                    qWarning() << "Lecture impossible :" << q.lastError().text();
                }
            }
        }
    }

    QSqlDatabase::removeDatabase(m_connectionName);
    return points;
}

QString HistoryReader::exportCsv(const QString &quantity, qint64 fromMs, qint64 toMs)
{
    // 1. grandeur valide ?
    const QString column = columnFor(quantity);
    if (column.isEmpty()) {
        return QString();
    }

    // 2. dossier Documents/SupervisionDashboard
    const QString dir =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/SupervisionDashboard";
    QDir().mkpath(dir);

    // 3. nom du fichier : colonne (sans accent) + date
    const QString fileName =
        QString("historique_%1_%2.csv")
            .arg(column, QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    const QString path = dir + "/" + fileName;

    // 4. ouverture du fichier
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Impossible de créer le fichier :" << path << file.errorString();
        return QString();
    }

    // 5. en-tête (le BOM permet à Excel de lire correctement les accents)
    QTextStream out(&file);
    out.setGenerateByteOrderMark(true);
    out << "timestamp_ms,datetime," << quantity << "\n";

    // 6. lecture de la base et écriture ligne par ligne
    int rows = 0;

    {   // db et la requête vivent seulement dans ce bloc
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "export_conn");
        db.setDatabaseName(databasePath());

        if (!db.open()) {
            qWarning() << "Export impossible :" << db.lastError().text();
        } else {
            QSqlQuery q(db);
            q.prepare(QString("SELECT ts, %1 FROM measurements "
                              "WHERE ts BETWEEN ? AND ? ORDER BY ts")
                          .arg(column));
            q.addBindValue(fromMs);
            q.addBindValue(toMs);

            if (q.exec()) {
                while (q.next()) {
                    const qint64 ts = q.value(0).toLongLong();
                    out << QString::number(ts) << ","
                        << QDateTime::fromMSecsSinceEpoch(ts)
                               .toString("yyyy-MM-dd HH:mm:ss") << ","
                        << QString::number(q.value(1).toDouble(), 'f', 3) << "\n";
                    ++rows;
                }
            } else {
                qWarning() << "Export impossible :" << q.lastError().text();
            }
        }
    }

    QSqlDatabase::removeDatabase("export_conn");

    // 7. fermeture : flush AVANT close, pour que tout soit écrit
    out.flush();
    file.close();

    // 8. aucune ligne : on ne laisse pas un fichier vide
    if (rows == 0) {
        file.remove();
        return QString();
    }

    return path;
}