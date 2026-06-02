#ifndef DFF_H
#define DFF_H

#include <QByteArray>

namespace Dff {
    unsigned int ror32(unsigned int x, unsigned int r);
    void teaDecryptBlock(unsigned char* data, int dataLen, unsigned int* key, int rounds = 8);
    QByteArray patchDffHeader(const QByteArray &dffData);
    QByteArray cleanDffData(const QByteArray &dffData);
    QByteArray processModFile(const QByteArray &modBytes);
    QByteArray processDffFile(const QByteArray &dffBytes);
    bool extractZip(const QString &zipPath, const QString &destDir);
    bool createZip(const QString &srcDir, const QString &zipPath);
}
#endif