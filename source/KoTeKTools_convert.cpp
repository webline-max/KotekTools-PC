#include "KoTeKTools.h"
#include <QApplication>
#include "dff.h"
#include "bpc.h"
#include "txd.h"
#include "btx_converter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QDirIterator>
#include <zip.h>
#include <QTemporaryDir>

void KoTeKTools::onBrowseBpc() { QString f = QFileDialog::getOpenFileName(this, "BPC", QString(), "BPC (*.bpc);;All (*)"); if (!f.isEmpty()) m_bpcPath->setText(f); }

void KoTeKTools::onConvertBpcToZip() {
    QString s = m_bpcPath->text(); if (s.isEmpty() || !QFile::exists(s)) { QMessageBox::information(this, "Информация", "Выберите BPC файл."); return; }
    KoTeKTools *self = this;
    runAsync([self, s]() {
        QFile f(s); if (!f.open(QIODevice::ReadOnly)) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не могу открыть файл."); }, Qt::QueuedConnection); return; }
        QByteArray data = f.readAll(); f.close();

        // xor дешифр bpc
        QByteArray key = QByteArray("1cK1a5UF2tU8*G2lW#&%");
        QByteArray dec(data.size(), Qt::Uninitialized);
        for (int i = 0; i < data.size(); i++) dec[i] = data[i] ^ key[i % key.size()];

        QFileInfo fi(s);
        QString out = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + ".zip";
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(dec); o.close();

        if (QFile::exists(out)) QMetaObject::invokeMethod(self, [self, fi]() { self->showNotifyConv("Готово! " + fi.completeBaseName() + ".zip"); self->m_bpcPath->clear(); }, Qt::QueuedConnection);
        else QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось создать файл."); }, Qt::QueuedConnection);
    });
}


void KoTeKTools::onBrowseBpcMeta() { QString f = QFileDialog::getOpenFileName(this, "BPC для BPCMETA", QString(), "BPC (*.bpc);;All (*)"); if (!f.isEmpty()) m_bpcMetaPath->setText(f); }


void KoTeKTools::onConvertBpcToMeta() {
    QString s = m_bpcMetaPath->text(); if (s.isEmpty() || !QFile::exists(s)) { QMessageBox::information(this, "Информация", "Выберите BPC файл."); return; }
    
    KoTeKTools *self = this;
    runAsync([self, s]() {
        QFile f(s); if (!f.open(QIODevice::ReadOnly)) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не могу открыть файл."); }, Qt::QueuedConnection); return; }
        QByteArray zipData = f.readAll(); f.close();
        

        // извлечение метаданных из bpc
        QByteArray meta = Bpc::buildBpcMeta(zipData);
        if (meta.isEmpty()) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "BPC не содержит аудиофайлов."); }, Qt::QueuedConnection); return; }

        QFileInfo fi(s); QString out = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + ".bpcmeta";
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(meta); o.close();

        if (QFile::exists(out)) QMetaObject::invokeMethod(self, [self, fi]() { self->showNotifyConv("Готово! " + fi.completeBaseName() + ".bpcmeta"); self->m_bpcMetaPath->clear(); }, Qt::QueuedConnection);
        else QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось создать файл."); }, Qt::QueuedConnection);
        
    });
}



void KoTeKTools::onBrowseIfpAni()
{
    QString file = QFileDialog::getOpenFileName(this, "Выберите IFP, ANI или ZIP", QString(), 
        "Поддерживаемые (*.ifp *.ani *.zip);;IFP (*.ifp);;ANI (*.ani);;ZIP (*.zip);;Все (*)");
        
    if (!file.isEmpty()) {
        if (file.toLower().endsWith(".zip")) {
            QTemporaryDir tempDir;
            if (tempDir.isValid()) {
                // libzip проверка содержимого
                zip_t *zip = zip_open(file.toUtf8().constData(), ZIP_RDONLY, nullptr);
                if (zip) {
                    bool found = false;
                    zip_int64_t numEntries = zip_get_num_entries(zip, 0);
                    for (zip_int64_t i = 0; i < numEntries; ++i) {
                        const char *name = zip_get_name(zip, i, 0);
                        if (name) {
                            QString qname = QString::fromUtf8(name);
                            if (qname.endsWith(".ifp", Qt::CaseInsensitive) || qname.endsWith(".ani", Qt::CaseInsensitive)) {
                                found = true;
                                break;
                            }
                        }
                    }
                    zip_close(zip);
                    if (!found) { QMessageBox::warning(this, "Предупреждение", "В архиве нет .ifp или .ani файлов."); return; }
                }
            }
        }
        m_ifpAniPath->setText(file);
        
    }
}

static void convertAniToIfp(const QString &inputPath, const QString &outputPath) {
    QFile input(inputPath);
    if (!input.open(QIODevice::ReadOnly)) return;
    QByteArray aniData = input.readAll();
    input.close();
    if (aniData.size() < 36 || aniData.left(4) != "ANP3") return;
    QByteArray ifpData(aniData.size(), Qt::Uninitialized);
    ifpData[0] = aniData[0];
    ifpData[1] = aniData[1];
    ifpData[2] = aniData[2];
    ifpData[3] = aniData[3];
    ifpData[4] = 0x14;
    ifpData[5] = 0x3D;
    ifpData[6] = 0x23;
    ifpData[7] = 0x00;
    for (int i = 4; i < 32; ++i) ifpData[i + 4] = aniData[i];
    for (int i = 36; i < aniData.size(); ++i) ifpData[i] = aniData[i];
    QFile output(outputPath);
    if (output.open(QIODevice::WriteOnly)) { output.write(ifpData); output.close(); }
}

static void processIfpAniDir(const QString &srcDir, const QString &dstDir)
{
    QDir src(srcDir), dst(dstDir);
    if (!dst.exists()) dst.mkpath(".");

    // ifp в ani
    QStringList ifpFiles = src.entryList(QStringList() << "*.ifp", QDir::Files);
    for (const QString &file : ifpFiles) {
        QFile f(src.absoluteFilePath(file));
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray data = f.readAll();
            if (data.size() >= 36) {
                data[28]=data[32]; data[29]=data[33]; data[32]=data[4]; data[33]=data[5]; data[34]=data[6]; data[35]=data[7]; data[4]=data[8]; data[5]=data[9]; data[6]=data[10]; data[8]=0; data[9]=0; data[10]=0;
                QFile o(dst.absoluteFilePath(QFileInfo(file).completeBaseName() + ".ani"));
                if (o.open(QIODevice::WriteOnly)) { o.write(data); o.close(); }
            }
            f.close();
        }
    }

    // ani в ifp
    QStringList aniFiles = src.entryList(QStringList() << "*.ani", QDir::Files);
    for (const QString &file : aniFiles) {
        convertAniToIfp(src.absoluteFilePath(file), dst.absoluteFilePath(QFileInfo(file).completeBaseName() + ".ifp"));
    }

    // подпапки
    QStringList subDirs = src.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &dir : subDirs) processIfpAniDir(src.absoluteFilePath(dir), dst.absoluteFilePath(dir));
}

void KoTeKTools::onConvertIfpAni()
{
    // конвертация ifp<>ani
    QString inputPath = m_ifpAniPath->text();
    if (inputPath.isEmpty() || !QFile::exists(inputPath)) { QMessageBox::information(this, "Информация", "Выберите файл."); return; }
    QString ext = QFileInfo(inputPath).suffix().toLower();
    if (ext != "ifp" && ext != "ani" && ext != "zip") { QMessageBox::warning(this, "Ошибка", "Поддерживаются: .ifp, .ani, .zip."); return; }
    

    KoTeKTools *self = this;
    runAsync([self, inputPath, ext]() {
        QFileInfo fi(inputPath);
        

        if (ext == "zip") {
            // распаковка, конвертация, упаковка
            QString tempDir = KoTeKTools::outputPath() + "/_ifpani_temp"; QDir().mkpath(tempDir);
            int randomId = QRandomGenerator::global()->bounded(10000, 99999);
            QString extractDir = tempDir + QString("/extract_%1").arg(randomId);
            QString processedDir = tempDir + QString("/processed_%1").arg(randomId);
            
            // libzip распаковка
            zip_t *zip = zip_open(inputPath.toUtf8().constData(), ZIP_RDONLY, nullptr);
            if (!zip) { QDir(tempDir).removeRecursively(); QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось распаковать ZIP."); }, Qt::QueuedConnection); return; }
            
            QDir().mkpath(extractDir);
            zip_int64_t numEntries = zip_get_num_entries(zip, 0);
            for (zip_int64_t i = 0; i < numEntries; ++i) {
                const char *name = zip_get_name(zip, i, 0);
                if (!name) continue;
                QString fullPath = extractDir + "/" + QString::fromUtf8(name);
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

            QDir().mkpath(processedDir);
            processIfpAniDir(extractDir, processedDir);
            
            // libzip упаковка
            QString outputZip = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + "_converted.zip";
            int error;
            zip_t *newZip = zip_open(outputZip.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
            if (newZip) {
                QDirIterator it(processedDir, QDir::Files, QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    it.next();
                    QString filePath = it.filePath();
                    QString relativePath = QDir(processedDir).relativeFilePath(filePath);
                    zip_source_t *source = zip_source_file(newZip, filePath.toUtf8().constData(), 0, 0);
                    if (source) {
                        zip_file_add(newZip, relativePath.toUtf8().constData(), source, ZIP_FL_OVERWRITE);
                    }
                }
                zip_close(newZip);
            }
            
            QDir(tempDir).removeRecursively();
            

            QMetaObject::invokeMethod(self, [self, outputZip]() {
                if (QFile::exists(outputZip)) { QFileInfo fi(outputZip); self->showNotifyConv("Готово! " + fi.fileName()); self->m_ifpAniPath->clear(); }
                else QMessageBox::critical(self, "Ошибка", "Не удалось создать ZIP.");
                
            }, Qt::QueuedConnection);
        } else {
            QFile f(inputPath);
            if (!f.open(QIODevice::ReadOnly)) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не могу открыть файл."); }, Qt::QueuedConnection); return; }
            QByteArray data = f.readAll(); f.close();
            if (data.size() < 36) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Файл поврежден."); }, Qt::QueuedConnection); return; }
            

            QString outExt;
            if (ext == "ifp") { data[28]=data[32]; data[29]=data[33]; data[32]=data[4]; data[33]=data[5]; data[34]=data[6]; data[35]=data[7]; data[4]=data[8]; data[5]=data[9]; data[6]=data[10]; data[8]=0; data[9]=0; data[10]=0; outExt=".ani"; }
            else if (ext == "ani") { 
                // новая конвертация ani в ifp
                QFile f2(inputPath);
                if (!f2.open(QIODevice::ReadOnly)) return;
                QByteArray aniData = f2.readAll();
                f2.close();
                if (aniData.size() >= 36 && aniData.left(4) == "ANP3") {
                    QByteArray ifpData(aniData.size(), Qt::Uninitialized);
                    ifpData[0] = aniData[0];
                    ifpData[1] = aniData[1];
                    ifpData[2] = aniData[2];
                    ifpData[3] = aniData[3];
                    ifpData[4] = 0x14;
                    ifpData[5] = 0x3D;
                    ifpData[6] = 0x23;
                    ifpData[7] = 0x00;
                    for (int i = 4; i < 32; ++i) ifpData[i + 4] = aniData[i];
                    for (int i = 36; i < aniData.size(); ++i) ifpData[i] = aniData[i];
                    data = ifpData;
                }
                outExt=".ifp"; 
            }
            

            QString outputPath = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + outExt;
            QFile o(outputPath); o.open(QIODevice::WriteOnly); o.write(data); o.close();
            

            QMetaObject::invokeMethod(self, [self, outputPath]() {
                if (QFile::exists(outputPath)) { QFileInfo fi(outputPath); self->showNotifyConv("Готово! " + fi.fileName()); self->m_ifpAniPath->clear(); }
                else QMessageBox::critical(self, "Ошибка", "Не удалось создать файл.");
            }, Qt::QueuedConnection);
        }
    });
}

void KoTeKTools::onBrowseMod()
{
    // выбор mod, dff, zip
    QString file = QFileDialog::getOpenFileName(this, "Выберите MOD, DFF или ZIP", QString(), 
        "Поддерживаемые (*.mod *.dff *.zip);;MOD (*.mod);;DFF (*.dff);;ZIP (*.zip);;Все (*)");
        
    if (!file.isEmpty()) {
        if (file.toLower().endsWith(".zip")) {
            QTemporaryDir tempDir;
            
            if (tempDir.isValid()) {
                // libzip проверка содержимого
                zip_t *zip = zip_open(file.toUtf8().constData(), ZIP_RDONLY, nullptr);
                if (zip) {
                    bool found = false;
                    zip_int64_t numEntries = zip_get_num_entries(zip, 0);
                    for (zip_int64_t i = 0; i < numEntries; ++i) {
                        const char *name = zip_get_name(zip, i, 0);
                        if (name) {
                            QString qname = QString::fromUtf8(name);
                            if (qname.endsWith(".mod", Qt::CaseInsensitive) || qname.endsWith(".dff", Qt::CaseInsensitive)) {
                                found = true;
                                break;
                            }
                        }
                    }
                    zip_close(zip);
                    if (!found) { QMessageBox::warning(this, "Предупреждение", "В архиве нет .mod или .dff файлов."); return; }
                }
            }
        }
        m_modPath->setText(file);
    }
}

static void processModDffDir(const QString &srcDir, const QString &dstDir)
{
    QDir src(srcDir), dst(dstDir);
    if (!dst.exists()) dst.mkpath(".");

    // mod в dff
    QStringList modFiles = src.entryList(QStringList() << "*.mod", QDir::Files);
    for (const QString &file : modFiles) {
        QFile f(src.absoluteFilePath(file));
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray result = Dff::processModFile(f.readAll());
            if (!result.isEmpty()) { QFile o(dst.absoluteFilePath(QFileInfo(file).completeBaseName() + ".dff")); if (o.open(QIODevice::WriteOnly)) { o.write(result); o.close(); } }
            f.close();
        }
    }
    

    // dff в dff
    QStringList dffFiles = src.entryList(QStringList() << "*.dff", QDir::Files);
    for (const QString &file : dffFiles) {
        QFile f(src.absoluteFilePath(file));
        if (f.open(QIODevice::ReadOnly)) { QFile o(dst.absoluteFilePath(QFileInfo(file).completeBaseName() + ".mod")); if (o.open(QIODevice::WriteOnly)) { o.write(f.readAll()); o.close(); } f.close(); }
    }
    

    // подпапки
    QStringList subDirs = src.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &dir : subDirs) processModDffDir(src.absoluteFilePath(dir), dst.absoluteFilePath(dir));
}


void KoTeKTools::onConvertMod()
{
    // конвертация mod<>dff
    QString inputPath = m_modPath->text();
    if (inputPath.isEmpty() || !QFile::exists(inputPath)) { QMessageBox::information(this, "Информация", "Выберите файл."); return; }
    QString ext = QFileInfo(inputPath).suffix().toLower();
    if (ext != "mod" && ext != "dff" && ext != "zip") { QMessageBox::warning(this, "Ошибка", "Поддерживаются: .mod, .dff, .zip."); return; }

    KoTeKTools *self = this;
    runAsync([self, inputPath, ext]() {
        QFileInfo fi(inputPath);

        if (ext == "zip") {
            // распаковка, конвертация, упаковка
            QString tempDir = KoTeKTools::outputPath() + "/_moddff_temp"; QDir().mkpath(tempDir);
            int randomId = QRandomGenerator::global()->bounded(10000, 99999);
            
            QString extractDir = tempDir + QString("/extract_%1").arg(randomId);
            QString processedDir = tempDir + QString("/processed_%1").arg(randomId);
            
            // libzip распаковка
            zip_t *zip = zip_open(inputPath.toUtf8().constData(), ZIP_RDONLY, nullptr);
            if (!zip) { QDir(tempDir).removeRecursively(); QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось распаковать ZIP."); }, Qt::QueuedConnection); return; }
            
            QDir().mkpath(extractDir);
            zip_int64_t numEntries = zip_get_num_entries(zip, 0);
            for (zip_int64_t i = 0; i < numEntries; ++i) {
                const char *name = zip_get_name(zip, i, 0);
                if (!name) continue;
                QString fullPath = extractDir + "/" + QString::fromUtf8(name);
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

            QDir().mkpath(processedDir);
            processModDffDir(extractDir, processedDir);

            // libzip упаковка
            QString outputZip = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + "_converted.zip";
            int error;
            zip_t *newZip = zip_open(outputZip.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
            if (newZip) {
                QDirIterator it(processedDir, QDir::Files, QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    it.next();
                    QString filePath = it.filePath();
                    QString relativePath = QDir(processedDir).relativeFilePath(filePath);
                    zip_source_t *source = zip_source_file(newZip, filePath.toUtf8().constData(), 0, 0);
                    if (source) {
                        zip_file_add(newZip, relativePath.toUtf8().constData(), source, ZIP_FL_OVERWRITE);
                    }
                }
                zip_close(newZip);
            }
            
            QDir(tempDir).removeRecursively();

            QMetaObject::invokeMethod(self, [self, outputZip]() {
                if (QFile::exists(outputZip)) { QFileInfo fi(outputZip); self->showNotifyConv("Готово! " + fi.fileName()); self->m_modPath->clear(); }
                else QMessageBox::critical(self, "Ошибка", "Не удалось создать ZIP.");
                
            }, Qt::QueuedConnection);
        } else {
            QFile f(inputPath);
            if (!f.open(QIODevice::ReadOnly)) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не могу открыть файл."); }, Qt::QueuedConnection); return; }
            QByteArray fileBytes = f.readAll(); f.close();
            

            QByteArray outputBytes; QString outExt;
            if (ext == "mod") { outputBytes = Dff::processModFile(fileBytes); outExt = ".dff"; }
            else if (ext == "dff") { outputBytes = fileBytes; outExt = ".mod"; }
            

            if (outputBytes.isEmpty()) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось обработать файл."); }, Qt::QueuedConnection); return; }
            

            QString outputPath = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + outExt;
            QFile o(outputPath); o.open(QIODevice::WriteOnly); o.write(outputBytes); o.close();
            

            QMetaObject::invokeMethod(self, [self, outputPath]() {
                if (QFile::exists(outputPath)) { QFileInfo fi(outputPath); self->showNotifyConv("Готово! " + fi.fileName()); self->m_modPath->clear(); }
                else QMessageBox::critical(self, "Ошибка", "Не удалось создать файл.");
            }, Qt::QueuedConnection);
        }
    });
}

void KoTeKTools::onBrowseTxd() { QString f = QFileDialog::getOpenFileName(this, "TXD", QString(), "TXD (*.txd);;All (*)"); if (!f.isEmpty()) m_txdPath->setText(f); }

void KoTeKTools::onConvertTxd() {
    // txd в png в zip
    QString s = m_txdPath->text(); if (s.isEmpty() || !QFile::exists(s)) { QMessageBox::information(this, "Информация", "Выберите TXD файл."); return; }
    KoTeKTools *self = this;
    
    runAsync([self, s]() {
        QFile f(s); if (!f.open(QIODevice::ReadOnly)) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не могу открыть файл."); }, Qt::QueuedConnection); return; }
        QByteArray data = f.readAll(); f.close();

        QByteArray r = Txd::convertToZip(data);

        QFileInfo fi(s); QString out = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + ".zip";
        
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(r); o.close();
        

        if (QFile::exists(out)) QMetaObject::invokeMethod(self, [self, fi]() { self->showNotifyConv("Готово! " + fi.completeBaseName() + ".zip"); }, Qt::QueuedConnection);
        else QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось создать файл."); }, Qt::QueuedConnection);
    });
}


void KoTeKTools::onBrowseBtx()
{
    // выбор btx, png, zip
    QString file = QFileDialog::getOpenFileName(this, "Выберите BTX, PNG или ZIP", QString(), 
        "Поддерживаемые (*.btx *.png *.zip);;BTX (*.btx);;PNG (*.png);;ZIP (*.zip);;Все (*)");
        
    if (!file.isEmpty()) {
        if (file.toLower().endsWith(".zip")) {
            QTemporaryDir tempDir;
            
            if (tempDir.isValid()) {
                // libzip проверка содержимого
                zip_t *zip = zip_open(file.toUtf8().constData(), ZIP_RDONLY, nullptr);
                if (zip) {
                    bool found = false;
                    zip_int64_t numEntries = zip_get_num_entries(zip, 0);
                    for (zip_int64_t i = 0; i < numEntries; ++i) {
                        const char *name = zip_get_name(zip, i, 0);
                        if (name) {
                            QString qname = QString::fromUtf8(name);
                            if (qname.endsWith(".btx", Qt::CaseInsensitive) || qname.endsWith(".png", Qt::CaseInsensitive)) {
                                found = true;
                                break;
                            }
                        }
                    }
                    zip_close(zip);
                    if (!found) { QMessageBox::warning(this, "Предупреждение", "В архиве нет .btx или .png файлов."); return; }
                }
            }
        }
        m_btxPath->setText(file);
    }
}

void KoTeKTools::onConvertBtx()
{
    // конвертация png<>btx
    QString inputPath = m_btxPath->text();
    if (inputPath.isEmpty() || !QFile::exists(inputPath)) { QMessageBox::information(this, "Информация", "Выберите файл."); return; }
    QString ext = QFileInfo(inputPath).suffix().toLower();
    if (ext != "btx" && ext != "png" && ext != "zip") { QMessageBox::warning(this, "Ошибка", "Поддерживаются: .btx, .png, .zip."); return; }
    

    KoTeKTools *self = this;
    runAsync([self, inputPath, ext]() {
        QFileInfo fi(inputPath);
        QString outputPath; bool success = false;
        

        if (ext == "png") { outputPath = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + ".btx"; success = BtxConverter::convertPngToBtx(inputPath, outputPath); }
        
        else if (ext == "btx") { outputPath = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + ".png"; success = BtxConverter::convertBtxToPng(inputPath, outputPath); }
        
        else if (ext == "zip") { outputPath = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + "_converted.zip"; success = BtxConverter::convertZipPngToBtx(inputPath, outputPath); if (!success) { outputPath = KoTeKTools::outputPath() + "/" + fi.completeBaseName() + "_converted.zip"; success = BtxConverter::convertZipBtxToPng(inputPath, outputPath); } }
        

        QMetaObject::invokeMethod(self, [self, success, outputPath]() {
            if (success) { QFileInfo fi(outputPath); self->showNotifyConv("Готово! " + fi.fileName()); self->m_btxPath->clear(); }
            
            else QMessageBox::critical(self, "Ошибка", "Конвертация не удалась.\nПроверьте что astcenc.exe и PVRTexToolCLI.exe в папке exe/\nДля ZIP: архив должен содержать .png или .btx файлы");
        }, Qt::QueuedConnection);
    });
}