#ifndef BTX_CONVERTER_H
#define BTX_CONVERTER_H

#include <QString>
#include <QByteArray>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QRandomGenerator>
#include <QTemporaryDir>

class BtxConverter
{
public:
    static bool convertPngToBtx(const QString &inputPath, const QString &outputPath);
    static bool convertBtxToPng(const QString &inputPath, const QString &outputPath);
    static bool convertZipPngToBtx(const QString &inputZipPath, const QString &outputZipPath);
    static bool convertZipBtxToPng(const QString &inputZipPath, const QString &outputZipPath);

private:
    static QString astcencPath();
    static QString pvrtexPath();
    
    static bool convertPngToBtxAstc(const QString &inputPath, const QString &tempKtxPath);
    static bool convertPngToBtxPvr(const QString &inputPath, const QString &tempKtxPath);
    static bool convertBtxToPngAstc(const QString &tempKtxPath, const QString &outputPath);
    static bool convertBtxToPngPvr(const QString &tempKtxPath, const QString &outputPath);
    
    static bool runProcessAndKill(const QString &program, const QStringList &arguments);
    static void killAstcPvrProcesses();
    
    static void processDirectoryPngToBtx(const QString &srcDir, const QString &dstDir);
    static void processDirectoryBtxToPng(const QString &srcDir, const QString &dstDir);
    
    static bool extractZip(const QString &zipPath, const QString &destDir);
    static bool createZip(const QString &srcDir, const QString &zipPath);
};

#endif