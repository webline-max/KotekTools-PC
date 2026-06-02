#include "painting.h"
#include <QImage>
#include <QColor>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <cmath>
#include <zip.h>
#include <QDirIterator>

namespace Painting {

   // hex валидность
bool isValidHex(const QString &hex) {
    if (!hex.startsWith("#") || hex.length() != 7) return false;
    for (int i = 1; i < 7; i++) {
        if (!hex[i].isLetterOrNumber()) return false;
        if (hex[i].isLetter() && hex[i].toUpper() > 'F') return false;
    }
    return true;
}

QByteArray colorizeImage(const QByteArray &imageData, const QString &hexColor) {
    // загрузка фото
    QImage img = QImage::fromData(imageData);
    if (img.isNull()) return imageData;
    
  
  
    if (img.format() != QImage::Format_ARGB32)
        img = img.convertToFormat(QImage::Format_ARGB32);
    
    // получение цвета
    QColor target(hexColor);
    float r_target = target.red();
    float g_target = target.green();
    float b_target = target.blue();
    
    // перекрашивание
    for (int y = 0; y < img.height(); y++) {
        QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); x++) {
            QRgb px = line[x];
            float r = qRed(px);
            float g = qGreen(px);
            float b = qBlue(px);
            float a = qAlpha(px) / 255.0f;
            
           
            float luminance = (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;
            
            
            // наложение цвета
            int newR = qBound(0, (int)(luminance * r_target), 255);
            int newG = qBound(0, (int)(luminance * g_target), 255);
            int newB = qBound(0, (int)(luminance * b_target), 255);
            
            line[x] = qRgba(newR, newG, newB, qAlpha(px));
        }
    }
    
    
    // сохранение в png
    QByteArray result;
    QBuffer buf(&result);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return result;
}


static QByteArray createZipFromFolder(const QStringList &files, const QString &folder, const QString &hexColor) {
    QByteArray zipData;
    QBuffer zipBuf(&zipData);
    
    // временная папка
    QString appPath = QApplication::applicationDirPath();
    QString tmpDir = appPath + "/painting_temp";
    QDir().mkpath(tmpDir);
    
    
    // покраска файла в папке врем
    for (const QString &file : files) {
        QString srcPath = "sources/" + file;
        if (QFile::exists(srcPath)) {
            QFile f(srcPath);
            if (f.open(QIODevice::ReadOnly)) {
                QByteArray data = f.readAll();
                f.close();
                QByteArray colored = colorizeImage(data, hexColor);
                QFile out(tmpDir + "/" + file);
                if (out.open(QIODevice::WriteOnly)) {
                    out.write(colored);
                    out.close();
                }
            }
        }
    }
    
    // png в zip 
    QString zipPath = appPath + "/painting_result.zip";
    QFile::remove(zipPath);
    
    int error;
    zip_t *zip = zip_open(zipPath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (zip) {
        QDirIterator it(tmpDir, QDir::Files, QDirIterator::Subdirectories);
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
    if (QFile::exists(zipPath)) {
        QFile zf(zipPath);
        if (zf.open(QIODevice::ReadOnly)) {
            result = zf.readAll();
            zf.close();
        }
        QFile::remove(zipPath);
    }
    


    QDir(tmpDir).removeRecursively();
    return result;
}


QByteArray createHudZip(const QString &hexColor) {
    // создание zip худ
    return createZipFromFolder(hudFiles(), "color", hexColor);
}


QByteArray createButtonsZip(const QString &hexColor) {
    // создание zip кнопок
    return createZipFromFolder(buttonFiles(), "color", hexColor);
}



QByteArray processImageZip(const QByteArray &zipData, const QString &hexColor) {
    QByteArray result;
    return colorizeImage(zipData, hexColor);
}

}