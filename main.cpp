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

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);                   // 1. d'abord l'application
    app.setApplicationName("SupervisionDashboard");    // 2. puis son nom

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