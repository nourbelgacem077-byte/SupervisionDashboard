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

    // Points (x = minutes, y = valeur), réduits à maxPoints environ
    Q_INVOKABLE QVariantList query(const QString &quantity, qint64 fromMs,
                                   qint64 toMs, int maxPoints = 600);

    // Écrit TOUTES les mesures de la plage dans un fichier CSV.
    // Retourne le chemin du fichier, ou "" en cas d'erreur ou de plage vide.
    Q_INVOKABLE QString exportCsv(const QString &quantity, qint64 fromMs,
                                  qint64 toMs);

private:
    static QString columnFor(const QString &quantity);   // "" si grandeur inconnue

    QString m_connectionName = "history_conn";
};