#ifndef MEASUREMENTPARSER_H
#define MEASUREMENTPARSER_H

#pragma once

#include <QByteArray>

struct Measurement
{
    double voltage = 0.0;
    double current = 0.0;
    double power = 0.0;
    double temperature = 0.0;
};

// JSON {"voltage":..,"current":..,"power":..,"temperature":..}
// Retourne false (et ne modifie pas out) si invalide.
bool parseMeasurement(const QByteArray &message, Measurement &out);

// Texte brut "online" / "offline"
bool parseStatus(const QByteArray &message, bool &online);

#endif // MEASUREMENTPARSER_H