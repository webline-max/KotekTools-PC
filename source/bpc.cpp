#include "bpc.h"
#include <cstring>
#include <QList>

namespace Bpc {

QByteArray buildBpcMeta(const QByteArray &zipData) {
    struct Entry {
        unsigned int dataOffset;
        unsigned int fileSize;
        unsigned char isMp3;
        QByteArray name;
    };
    QList<Entry> entries;

    int pos = 0;
    while (pos < zipData.size() - 30) {
        unsigned int sig;
        memcpy(&sig, zipData.constData() + pos, 4);
        if (sig == 0x02014b50 || sig == 0x06054b50) break;
        if (sig != 0x04034b50) { pos++; continue; }
        if (pos + 30 > zipData.size()) break;

        unsigned short version, flags, compression, modTime, modDate;
        unsigned int crc32, compressedSize, uncompressedSize;
        unsigned short nameLen, extraLen;
        int off = pos + 4;
        memcpy(&version, zipData.constData() + off, 2); off += 2;
        memcpy(&flags, zipData.constData() + off, 2); off += 2;
        memcpy(&compression, zipData.constData() + off, 2); off += 2;
        memcpy(&modTime, zipData.constData() + off, 2); off += 2;
        memcpy(&modDate, zipData.constData() + off, 2); off += 2;
        memcpy(&crc32, zipData.constData() + off, 4); off += 4;
        memcpy(&compressedSize, zipData.constData() + off, 4); off += 4;
        memcpy(&uncompressedSize, zipData.constData() + off, 4); off += 4;
        memcpy(&nameLen, zipData.constData() + off, 2); off += 2;
        memcpy(&extraLen, zipData.constData() + off, 2); off += 2;

        unsigned int headerOffset = pos;
        unsigned int dataOffset = headerOffset + 30 + nameLen + extraLen;
        if (pos + 30 + nameLen > zipData.size()) break;
        QByteArray filename = zipData.mid(pos + 30, nameLen);
        QString fname = QString::fromUtf8(filename).toLower();

        if (fname.endsWith(".mp3") || fname.endsWith(".wav") || fname.endsWith(".ogg")) {
            Entry entry;
            entry.dataOffset = dataOffset;
            entry.fileSize = uncompressedSize;
            entry.isMp3 = fname.endsWith(".mp3") ? 1 : 0;
            entry.name = filename;
            entries.append(entry);
        }
        pos = dataOffset + compressedSize;
    }

    if (entries.isEmpty()) return QByteArray();

    int totalSize = 4;
    for (const Entry &e : entries) totalSize += 4 + 4 + 1 + 2 + e.name.size();
    QByteArray buffer(totalSize, 0);

    auto w32 = [](QByteArray &buf, int &p, unsigned int val) {
        buf[p]=val&0xFF; buf[p+1]=(val>>8)&0xFF; buf[p+2]=(val>>16)&0xFF; buf[p+3]=(val>>24)&0xFF; p+=4;
    };
    auto w16 = [](QByteArray &buf, int &p, unsigned short val) {
        buf[p]=val&0xFF; buf[p+1]=(val>>8)&0xFF; p+=2;
    };

    int bp = 0;
    w32(buffer, bp, entries.size());
    for (const Entry &e : entries) {
        w32(buffer, bp, e.dataOffset);
        w32(buffer, bp, e.fileSize);
        buffer[bp] = e.isMp3; bp += 1;
        w16(buffer, bp, e.name.size());
        memcpy(buffer.data()+bp, e.name.constData(), e.name.size());
        bp += e.name.size();
    }
    return buffer;
}

}