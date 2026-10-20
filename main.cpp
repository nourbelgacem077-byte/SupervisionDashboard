#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include<QDir>
#include<QFile>
#include <QDateTime>
#include "alarmrule.h"
#include "alarmmanager.h"
#include "dbworker.h"
#include "historyreader.h"
#include "appsettings.h"
namespace {
// Test provisoire : deviendra un vrai test Qt Test en semaine 4
void testAlarmRule()
{
    AlarmRule rule(250.0, 260.0, 3.0);

    for (double v : {245.0, 251.0, 249.0, 246.0, 265.0,
                     258.0, 256.0, 246.0, 252.0, 240.0}) {
        qDebug() << v << "->" << static_cast<int>(rule.evaluate(v));
    }
}
void testAlarmManager()
{
    AlarmManager manager;

    manager.addRule("Tension", 250.0, 260.0, 3.0);
    manager.addRule("Température", 75.0, 90.0, 5.0);

    QObject::connect(&manager, &AlarmManager::alarmRaised,
                     [](const AlarmEvent &e) {
                         qDebug() << e.quantity << static_cast<int>(e.level)
                         << e.value << e.acknowledged;
                     });

    for (double v : {245.0, 251.0, 249.0, 246.0, 265.0, 258.0, 256.0, 246.0}) {
        manager.processMeasurement("Tension", v);
    }

    for (double v : {70.0, 80.0, 95.0, 84.0, 69.0}) {
        manager.processMeasurement("Température", v);
    }

    manager.processMeasurement("Humidité", 50.0); // ignorée

    qDebug() << "actives:" << manager.activeCount();
    manager.acknowledge(0);
    qDebug() << "actives:" << manager.activeCount();
    manager.acknowledge(99); // ne doit pas planter
}
void testDbWorker()
{
    const QString path = QDir::tempPath() + "/test_supervision.db";
    QFile::remove(path);   // repartir de zéro à chaque lancement

    {   // premier bloc : le DbWorker est détruit à la fermeture de l'accolade
        DbWorker worker(path);
        qDebug() << "open:" << worker.open();

        for (int i = 0; i < 50; ++i) {
            worker.insertSample(1700000000000LL + i * 1000LL,
                                230.0 + i * 0.1,
                                10.0, 2.3, 55.0);
        }
        worker.flush();
        qDebug() << "count:" << worker.sampleCount();
    }

    {   // deuxième bloc : on rouvre la même base
        DbWorker worker2(path);
        worker2.open();
        qDebug() << "count après réouverture:" << worker2.sampleCount();
    }
}
void testHistoryReader()
{
    HistoryReader reader;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    const QVariantList pts = reader.query("Tension", 0, now, 600);
    qDebug() << "points :" << pts.size();

    if (!pts.isEmpty()) {
        qDebug() << "premier :" << pts.first().toPointF()
        << "dernier :" << pts.last().toPointF();
    }

    qDebug() << "Humidité :" << reader.query("Humidité", 0, now, 600).size();
    qDebug() << "plage inversée :" << reader.query("Tension", now, 0, 600).size();
}
void testExportCsv()
{
    HistoryReader reader;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    qDebug() << "export :" << reader.exportCsv("Tension", 0, now);
    qDebug() << "Humidité :" << reader.exportCsv("Humidité", 0, now);
    qDebug() << "plage inversée :" << reader.exportCsv("Tension", now, 0);
}
void testAppSettings()
{
    AppSettings s;
    s.resetDefaults();
    qDebug() << "warning Tension :" << s.warning("Tension");

    int signalCount = 0;
    QObject::connect(&s, &AppSettings::thresholdsChanged,
                     [&signalCount]() { ++signalCount; });

    const bool ok = s.setThresholds("Tension", 245, 255, 2);
    qDebug() << "set 245/255 :" << ok << "  signaux :" << signalCount;
    qDebug() << "warning Tension :" << s.warning("Tension");

    const bool invalid = s.setThresholds("Tension", 260, 250, 2);
    qDebug() << "set invalide :" << invalid << "  signaux :" << signalCount;
    qDebug() << "warning Tension :" << s.warning("Tension");

    qDebug() << "grandeur inconnue :" << s.setThresholds("Humidité", 10, 20, 1);

    {   // écrit par un objet, puis détruit
        AppSettings s2;
        s2.setThresholds("Courant", 13, 15, 1);
    }
    AppSettings s3;   // relit le fichier
    qDebug() << "Courant lu par un autre objet :" << s3.warning("Courant");

    s.resetDefaults();   // ne pas laisser de réglages de test
}
}
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);                   // 1. d'abord l'application
    app.setApplicationName("SupervisionDashboard");    // 2. puis son nom

    // 3. ensuite seulement, tout ce qui utilise Qt (SQL, fichiers...)
   // testHistoryReader();
   // testExportCsv();
    testAppSettings();

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("SupervisionDashboard", "Main");

    return app.exec();
}