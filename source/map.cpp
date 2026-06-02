#include "map.h"
#include <QImage>
#include <QBuffer>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QApplication>
#include <QTemporaryDir>
#include <zip.h>
#include <QDirIterator>

namespace Map {



QByteArray sliceMapImage(const QByteArray &imageData) {
    QImage img = QImage::fromData(imageData);
    if (img.isNull()) return QByteArray();
    
    
    // 14x14 карта
    const int GRID_SIZE = 14;
    
    int segW = img.width() / GRID_SIZE;
    int segH = img.height() / GRID_SIZE;
    
    if (segW <= 0 || segH <= 0) return QByteArray();
    
    
    QTemporaryDir tmpDir(QApplication::applicationDirPath() + "/map_temp_XXXXXX");
    if (!tmpDir.isValid()) return QByteArray();
    
    QString tmpPath = tmpDir.path();
    
    
    // оптимизация
    
    for (int row = 0; row < GRID_SIZE; row++) {
        int yOffset = row * segH;
        for (int col = 0; col < GRID_SIZE; col++) {
            QImage seg = img.copy(col * segW, yOffset, segW, segH);
            int num = row * GRID_SIZE + col;
            QString fileName = QString("%1/radar%2.png")
                .arg(tmpPath)
                .arg(num, 2, 10, QChar('0'));
            seg.save(fileName, "PNG", 85);
        }
    }
    
    
    
    QString zipPath = tmpPath + "/result.zip";
    
    // zip
    int error;
    zip_t *zip = zip_open(zipPath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (zip) {
        QDirIterator it(tmpPath, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QString filePath = it.filePath();
            QString fileName = QFileInfo(filePath).fileName();
            
            zip_source_t *source = zip_source_file(zip, filePath.toUtf8().constData(), 0, 0);
            if (source) {
                zip_file_add(zip, fileName.toUtf8().constData(), source, ZIP_FL_OVERWRITE);
            }
        }
        zip_close(zip);
    }
    
    QByteArray result;
    
    QFile zf(zipPath);
    if (zf.open(QIODevice::ReadOnly)) {
        result = zf.readAll();
        zf.close();
    }
    
    return result;
    
}


QByteArray assembleMap(const QByteArray &zipData) {
    QTemporaryDir tmpDir(QApplication::applicationDirPath() + "/map_assemble_XXXXXX");
    if (!tmpDir.isValid()) return QByteArray();
    
    QString tmpPath = tmpDir.path();
    
    // zip
    QString zipPath = tmpPath + "/radar.zip";
    {
        QFile zf(zipPath);
        if (!zf.open(QIODevice::WriteOnly)) return QByteArray();
        zf.write(zipData);
        zf.close();
    }
    
    
    
    zip_t *zip = zip_open(zipPath.toUtf8().constData(), ZIP_RDONLY, nullptr);
    if (zip) {
        zip_int64_t numEntries = zip_get_num_entries(zip, 0);
        for (zip_int64_t i = 0; i < numEntries; ++i) {
            const char *name = zip_get_name(zip, i, 0);
            if (!name) continue;
            
            QString fullPath = tmpPath + "/" + QString::fromUtf8(name);
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
    }
    
    // название радаров
    QImage first(tmpPath + "/radar00.png");
    if (first.isNull()) return QByteArray();
    
    
    const int GRID_SIZE = 14;
    int segW = first.width();
    int segH = first.height();
    
    
    if (segW <= 0 || segH <= 0) return QByteArray();
    
    QImage result(GRID_SIZE * segW, GRID_SIZE * segH, QImage::Format_ARGB32);
    
    
    
    for (int row = 0; row < GRID_SIZE; row++) {
        int yOffset = row * segH;
        for (int col = 0; col < GRID_SIZE; col++) {
            int num = row * GRID_SIZE + col;
            QString fileName = QString("%1/radar%2.png")
                .arg(tmpPath)
                .arg(num, 2, 10, QChar('0'));
            
            QImage seg(fileName);
            if (!seg.isNull()) {
                int xOffset = col * segW;
                for (int y = 0; y < segH; y++) {
                    memcpy(result.scanLine(yOffset + y) + xOffset * 4,
                           seg.scanLine(y), segW * 4);
                }
            }
        }
    }
    
    
    
    QByteArray outData;
    QBuffer buf(&outData);
    if (!buf.open(QIODevice::WriteOnly)) return QByteArray();
    
    result.save(&buf, "PNG", 85);
    buf.close();
    
    return outData;
    
}

}