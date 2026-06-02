#ifndef BPC_H
#define BPC_H

#include <QByteArray>

namespace Bpc {
    QByteArray buildBpcMeta(const QByteArray &zipData);
}

#endif