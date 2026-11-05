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

bool parseMeasurement(const QByteArray &message, Measurement &out);


bool parseStatus(const QByteArray &message, bool &online);

#endif // MEASUREMENTPARSER_H