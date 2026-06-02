#include "btx_converter.h"
#include <zip.h>
#include <QDirIterator>


QString BtxConverter::astcencPath()
{

    // путь к astcenc
    return QApplication::applicationDirPath() + "/exe/astcenc.exe";
}

QString BtxConverter::pvrtexPath()
{
    // путь к PVR
    return QApplication::applicationDirPath() + "/exe/PVRTexToolCLI.exe";
}


bool BtxConverter::runProcessAndKill(const QString &program, const QStringList &arguments)
{
    // запуск и килл
    QProcess process;
    process.start(program, arguments);
    
    if (!process.waitForStarted(10000)) {
        killAstcPvrProcesses();
        return false;
    }
    
    bool finished = process.waitForFinished(60000);
    
    if (!finished) {
        process.kill();
        process.waitForFinished(3000);
    }
    
    killAstcPvrProcesses();
    
    return finished && process.exitCode() == 0;
}

void BtxConverter::killAstcPvrProcesses()
{
    // килл
    QProcess::execute("taskkill", QStringList() << "/F" << "/IM" << "astcenc.exe");
    
    QProcess::execute("taskkill", QStringList() << "/F" << "/IM" << "PVRTexToolCLI.exe");
}

bool BtxConverter::convertPngToBtxAstc(const QString &inputPath, const QString &tempKtxPath)
{
    // png в ktx через astcenc
    QStringList args;
    args << "-cl" << inputPath << tempKtxPath << "8x8" << "-fast" << "-silent";
    return runProcessAndKill(astcencPath(), args);
}


bool BtxConverter::convertPngToBtxPvr(const QString &inputPath, const QString &tempKtxPath)
{
    // png в ktx через pvr
    QStringList args;
    args << "-i" << inputPath 
         << "-o" << tempKtxPath 
         << "-f" << "ASTC_8x8,UBN,sRGB" 
         << "-ics" << "srgb" 
         << "-silent";
    return runProcessAndKill(pvrtexPath(), args);
}



bool BtxConverter::convertBtxToPngAstc(const QString &tempKtxPath, const QString &outputPath)
{
    // ktx в png через astcenc
    QStringList args;
    args << "-dl" << tempKtxPath << outputPath << "-silent";
    return runProcessAndKill(astcencPath(), args);
}

bool BtxConverter::convertBtxToPngPvr(const QString &tempKtxPath, const QString &outputPath)
{
    // ktx в png через pvr
    QStringList args;
    args << "-i" << tempKtxPath 
         << "-d" << outputPath 
         << "-f" << "r8g8b8a8" 
         << "-ics" << "srgb" 
         << "-silent";
    return runProcessAndKill(pvrtexPath(), args);
}

bool BtxConverter::convertPngToBtx(const QString &inputPath, const QString &outputPath)
{
    // png в btx
    QString tempDir = QFileInfo(outputPath).absolutePath();
    int randomId = QRandomGenerator::global()->bounded(1000, 9999);
    QString tempKtxPath = tempDir + QString("/_temp_btx_%1.ktx").arg(randomId);
    
    
    
    // astcenc потом pvr
    bool success = convertPngToBtxAstc(inputPath, tempKtxPath);
    if (!success) {
        success = convertPngToBtxPvr(inputPath, tempKtxPath);
    }
    
    
    if (!success) {
        QFile::remove(tempKtxPath);
        return false;
    }
    
    
    // читаем ktx
    QFile tempFile(tempKtxPath);
    if (!tempFile.open(QIODevice::ReadOnly)) {
        QFile::remove(tempKtxPath);
        return false;
    }
    
    QByteArray ktxData = tempFile.readAll();
    tempFile.close();
    
    // btx байты
    QByteArray btxData;
    btxData.append('\x02');
    btxData.append('\x00');
    btxData.append('\x00');
    btxData.append('\x00');
    btxData.append(ktxData);
    
    
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        QFile::remove(tempKtxPath);
        return false;
    }
    
    
    outputFile.write(btxData);
    outputFile.close();
    
    QFile::remove(tempKtxPath);
    return true;
    
}

bool BtxConverter::convertBtxToPng(const QString &inputPath, const QString &outputPath)
{
    // btx в png
    QString tempDir = QFileInfo(outputPath).absolutePath();
    int randomId = QRandomGenerator::global()->bounded(1000, 9999);
    QString tempKtxPath = tempDir + QString("/_temp_btx_%1.ktx").arg(randomId);
    
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly))
        return false;
        
    
    QByteArray btxData = inputFile.readAll();
    inputFile.close();
    
    if (btxData.size() < 4)
        return false;
        
    
     // первые 4 байта
    QByteArray ktxData = btxData.mid(4);
    
    QFile tempFile(tempKtxPath);
    if (!tempFile.open(QIODevice::WriteOnly))
        return false;
    
    tempFile.write(ktxData);
    tempFile.close();
    
    // astcenc вместо pvr
    bool success = convertBtxToPngAstc(tempKtxPath, outputPath);
    if (!success) {
        success = convertBtxToPngPvr(tempKtxPath, outputPath);
    }
    
    QFile::remove(tempKtxPath);
    killAstcPvrProcesses();
    
    return success;
}

bool BtxConverter::extractZip(const QString &zipPath, const QString &destDir)
{
    // zip упаковка
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

bool BtxConverter::createZip(const QString &srcDir, const QString &zipPath)
{
    // zip упаковка
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

void BtxConverter::processDirectoryPngToBtx(const QString &srcDir, const QString &dstDir)
{
    // конвертация всех png в папке в btx
    QDir src(srcDir);
    QDir dst(dstDir);
    
    
    if (!dst.exists())
        dst.mkpath(".");
        
    
    QStringList pngFiles = src.entryList(QStringList() << "*.png", QDir::Files);
    for (const QString &file : pngFiles) {
        QString inputPath = src.absoluteFilePath(file);
        QString outputName = QFileInfo(file).completeBaseName() + ".btx";
        QString outputPath = dst.absoluteFilePath(outputName);
        convertPngToBtx(inputPath, outputPath);
    }
    


    QStringList subDirs = src.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &dir : subDirs) {
        processDirectoryPngToBtx(src.absoluteFilePath(dir), dst.absoluteFilePath(dir));
    }
}

void BtxConverter::processDirectoryBtxToPng(const QString &srcDir, const QString &dstDir)
{
    // конвертация всех btx в папке в png
    QDir src(srcDir);
    QDir dst(dstDir);
    
    if (!dst.exists())
        dst.mkpath(".");
    
    QStringList btxFiles = src.entryList(QStringList() << "*.btx", QDir::Files);
    for (const QString &file : btxFiles) {
        QString inputPath = src.absoluteFilePath(file);
        QString outputName = QFileInfo(file).completeBaseName() + ".png";
        QString outputPath = dst.absoluteFilePath(outputName);
        convertBtxToPng(inputPath, outputPath);
    }
    
    
    
    QStringList subDirs = src.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &dir : subDirs) {
        processDirectoryBtxToPng(src.absoluteFilePath(dir), dst.absoluteFilePath(dir));
    }
}


bool BtxConverter::convertZipPngToBtx(const QString &inputZipPath, const QString &outputZipPath)
{
    // zip png в zip btx, распаковка, конвертация, упаковка
    QString tempBase = QApplication::applicationDirPath() + "/_btx_temp";
    QDir().mkpath(tempBase);
    
    
    int randomId = QRandomGenerator::global()->bounded(10000, 99999);
    QString extractDir = tempBase + QString("/extract_%1").arg(randomId);
    QString processedDir = tempBase + QString("/processed_%1").arg(randomId);
    
    if (!extractZip(inputZipPath, extractDir)) {
        QDir(tempBase).removeRecursively();
        return false;
    }
    
    
    processDirectoryPngToBtx(extractDir, processedDir);
    
    bool success = createZip(processedDir, outputZipPath);
    
    
    QDir(tempBase).removeRecursively();
    killAstcPvrProcesses();
    
    return success;
}

bool BtxConverter::convertZipBtxToPng(const QString &inputZipPath, const QString &outputZipPath)
{
    // zip btx в zip png, распаковка, конвертация, упаковка
    QString tempBase = QApplication::applicationDirPath() + "/_btx_temp";
    QDir().mkpath(tempBase);
    
    
    int randomId = QRandomGenerator::global()->bounded(10000, 99999);
    QString extractDir = tempBase + QString("/extract_%1").arg(randomId);
    QString processedDir = tempBase + QString("/processed_%1").arg(randomId);
    
    if (!extractZip(inputZipPath, extractDir)) {
        QDir(tempBase).removeRecursively();
        return false;
    }
    
    processDirectoryBtxToPng(extractDir, processedDir);
    
    bool success = createZip(processedDir, outputZipPath);
    
    QDir(tempBase).removeRecursively();
    killAstcPvrProcesses();
    
    return success;
}