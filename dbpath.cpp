#include "dbpath.h"

#include <QDir>
#include <QStandardPaths>

QString databasePath()
{
    const QString dir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/supervision.db";
}