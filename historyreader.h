#pragma once

#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class HistoryReader : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit HistoryReader(QObject *parent = nullptr);


    Q_INVOKABLE QVariantList query(const QString &quantity, qint64 fromMs,
                                   qint64 toMs, int maxPoints = 600);


    Q_INVOKABLE QString exportCsv(const QString &quantity, qint64 fromMs,
                                  qint64 toMs);

private:
    static QString columnFor(const QString &quantity);

    QString m_connectionName = "history_conn";
};