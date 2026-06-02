#include "dff.h"
#include <cstring>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QProcess>
#include <zip.h>
#include <QDirIterator>

namespace Dff {

unsigned int ror32(unsigned int x, unsigned int r) {
    return ((x >> r) | (x << (32 - r))) & 0xFFFFFFFF;
}

void teaDecryptBlock(unsigned char* data, int dataLen, unsigned int* key, int rounds) {
    // tea дешифр
    unsigned int delta = 0x61C88647;
    for (int offset = 0; offset < dataLen; offset += 8) {
        if (offset + 8 > dataLen) break;
        unsigned int v0, v1;
        memcpy(&v0, data + offset, 4);
        memcpy(&v1, data + offset + 4, 4);
        unsigned int sum_val = (-delta * rounds) & 0xFFFFFFFF;
        for (int i = 0; i < rounds; i++) {
            v1 = (v1 - (((v0 << 4) + key[2]) ^ (v0 + sum_val) ^ ((v0 >> 5) + key[3]))) & 0xFFFFFFFF;
            unsigned int new_sum = (sum_val + v1) & 0xFFFFFFFF;
            sum_val = (sum_val + delta) & 0xFFFFFFFF;
            v0 = (v0 - (((v1 << 4) + key[0]) ^ (new_sum) ^ ((v1 >> 5) + key[1]))) & 0xFFFFFFFF;
        }
        memcpy(data + offset, &v0, 4);
        memcpy(data + offset + 4, &v1, 4);
    }
}

QByteArray patchDffHeader(const QByteArray &dffData) {
    if (dffData.size() < 12) return dffData;
    unsigned int realSize = dffData.size() - 12;
    QByteArray patched = dffData;
    memcpy(patched.data() + 4, &realSize, 4);
    return patched;
}

QByteArray cleanDffData(const QByteArray &dffData) {


    int end = dffData.size();
    while (end > 0 && dffData[end - 1] == 0) end--;
    return dffData.left(end);
}

QByteArray processModFile(const QByteArray &modBytes) {
    // tea дешифр
    if (modBytes.size() < 28) return QByteArray();
    unsigned int magic, length, numBlocks;
    memcpy(&magic, modBytes.constData(), 4);
    memcpy(&length, modBytes.constData() + 4, 4);
    memcpy(&numBlocks, modBytes.constData() + 8, 4);
    if (magic == 0x00000010) return modBytes;
    if (magic != 0xAB921033) return QByteArray();
    unsigned int baseKey[4] = {0x6ED9EE7A, 0x930C666B, 0x930E166B, 0x4709EE79};
    unsigned int key[4];
    for (int i = 0; i < 4; i++) key[i] = ror32(baseKey[i] ^ 0x12913AFB, 19);  // вычисление ключа
    QByteArray data = modBytes;
    unsigned char* dataPtr = reinterpret_cast<unsigned char*>(data.data());
    int offset = 28;
    for (unsigned int i = 0; i < numBlocks; i++) {
        if (offset + 0x800 > data.size()) return QByteArray();
        teaDecryptBlock(dataPtr + offset, 0x800, key);  // дешифр блока
        offset += 0x800;
    }
    unsigned int actualLength = qMin(length, (unsigned int)(data.size() - 28));
    QByteArray dff = data.mid(28, actualLength);
    dff = patchDffHeader(dff);
    dff = cleanDffData(dff);
    return dff;
}

QByteArray processDffFile(const QByteArray &dffBytes) {
    return dffBytes;
}

bool extractZip(const QString &zipPath, const QString &destDir) {
    // распаковка zip
    zip_t *zip = zip_open(zipPath.toUtf8().constData(), ZIP_RDONLY, nullptr);
    if (!zip) return false;
    
    zip_int64_t numEntries = zip_get_num_entries(zip, 0);
    
    for (zip_int64_t i = 0; i < numEntries; ++i) {
        const char *name = zip_get_name(zip, i, 0);
        if (!name) continue;
        
        QString fullPath = destDir + "/" + QString::fromUtf8(name);
        QFileInfo fi(fullPath);
        
        QDir().mkpath(fi.absolutePath());
        
        zip_file_t *zf = zip_fopen_index(zip, i, 0);
        if (zf) {
            QFile out(fullPath);
            if (out.open(QIODevice::WriteOnly)) {
                char buffer[8192];
                zip_int64_t bytes;
                while ((bytes = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
                    out.write(buffer, bytes);
                }
            }
            out.close();
            zip_fclose(zf);
        }
    }
    
    zip_close(zip);
    return true;
}
    
    
bool createZip(const QString &srcDir, const QString &zipPath) {
    // dff в zip
    int error;
    zip_t *zip = zip_open(zipPath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip) return false;
    
    QDirIterator it(srcDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QString filePath = it.filePath();
        QString relativePath = QDir(srcDir).relativeFilePath(filePath);
        
        zip_source_t *source = zip_source_file(zip, filePath.toUtf8().constData(), 0, 0);
        if (!source) {
            zip_close(zip);
            return false;
        }
        
        zip_int64_t index = zip_file_add(zip, relativePath.toUtf8().constData(), source, ZIP_FL_OVERWRITE);
        if (index < 0) {
            zip_source_free(source);
            zip_close(zip);
            return false;
        }
    }
    
    zip_close(zip);
    return true;
}

}