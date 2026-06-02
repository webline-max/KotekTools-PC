#ifndef MAP_H
#define MAP_H

#include <QByteArray>
#include <QString>

namespace Map
{
    QByteArray sliceMapImage(const QByteArray &imageData);
    QByteArray assembleMap(const QByteArray &zipData);
}

#endif