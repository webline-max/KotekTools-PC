#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include <QString>
#include <QColor>

namespace Atmosphere {
    QByteArray generateTimecyc(const QString &topHex, const QString &bottomHex,
                               const QString &sunHex, const QString &cloudsHex);
    QByteArray generateColorcycle(const QString &hexColor);
    bool isValidHex(const QString &hex);
    QColor hexToColor(const QString &hex);
}

#endif