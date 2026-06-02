#include "KoTeKTools.h"
#include <QApplication>
#include "listva_names.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <zip.h>
#include <QDirIterator>

void KoTeKTools::onBrowseBillboard() { QString f = QFileDialog::getOpenFileName(this, "Билборд", QString(), "BTX (*.btx);;All (*)"); if (!f.isEmpty()) m_billboardPath->setText(f); }


void KoTeKTools::onCopyBillboard() {
    // копирование btx(bild)
    QString s = m_billboardPath->text(); if (s.isEmpty() || !QFile::exists(s)) { QMessageBox::information(this, "Информация", "Выберите файл."); return; }
    KoTeKTools *self = this;
    runAsync([self, s]() {
        QString app = KoTeKTools::outputPath(), tmp = app + "/bild_temp"; QDir().mkpath(tmp);
        

        // билды
        QStringList names = {
            "Billd_GostownParadise.btx", "Billd_GTABerlin.btx", "Billd_GTAUnited.btx",
            "Billd_Myriadlslands.btx", "Billd_SanVice.btx", "Billd_YouAreHere.btx",
            "BLBRD_1_a889.btx", "BLBRD_2_a889.btx", "BLBRD_3_a889.btx",
            "BLBRD_4_a889.btx", "BLBRD_5_a889.btx", "BLBRD_6_a889.btx",
            "bilb_sign1.btx", "bilb_sign2.btx", "Billb_AlienCity.btx",
            "Billb_GostownParadise.btx", "Billb_GTABer.btx", "Billb_GTAUnited.btx",
            "Billb_MyriadIslands.btx", "Billb_SanVice.btx", "Billb_YouAreHere.btx",
            "BLBRD_2_889.btx", "BLBRD_3_889.btx", "BLBRD_4_889.btx",
            "BLBRD_5_889.btx", "BLBRD_6_889.btx", "BLBRD_btn1_a889.btx",
            "BLBRD_main1_a889.btx", "reclam62.btx", "reclam63.btx",
            "reclam64.btx", "reclam65.btx", "reclam66.btx", "reclam67.btx",
            "reclam68.btx", "reclam69.btx"
        };

        for (const QString &n : names) { if (QFile::exists(tmp+"/"+n)) QFile::remove(tmp+"/"+n); QFile::copy(s, tmp+"/"+n); }
        

        QString zip = app + "/bildbillboards.zip"; if (QFile::exists(zip)) QFile::remove(zip);
        
        // libzip упаковка
        int error;
        zip_t *newZip = zip_open(zip.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
        if (newZip) {
            QDirIterator it(tmp, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                QString filePath = it.filePath();
                QString fileName = QFileInfo(filePath).fileName();
                zip_source_t *source = zip_source_file(newZip, filePath.toUtf8().constData(), 0, 0);
                if (source) {
                    zip_file_add(newZip, fileName.toUtf8().constData(), source, ZIP_FL_OVERWRITE);
                }
            }
            zip_close(newZip);
        }
        
        QDir(tmp).removeRecursively();
        

        if (QFile::exists(zip)) QMetaObject::invokeMethod(self, [self]() { self->showNotify("Готово! билдборды созданы"); }, Qt::QueuedConnection);
        
        else QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось создать архив."); }, Qt::QueuedConnection);
    });
}

void KoTeKTools::onBrowseLogo() { QString f = QFileDialog::getOpenFileName(this, "Логотип", QString(), "BTX (*.btx);;All (*)"); if (!f.isEmpty()) m_logoPath->setText(f); }

void KoTeKTools::onCopyLogos() {
    // копирование btx(logo)
    QString s = m_logoPath->text(); if (s.isEmpty() || !QFile::exists(s)) { QMessageBox::information(this, "Информация", "Выберите файл."); return; }
    KoTeKTools *self = this;
    runAsync([self, s]() {
        // список серверов
        QString jsonPath = KoTeKTools::outputPath() + "/servers.json";
        
        if (!self->m_serversLoaded && QFile::exists(jsonPath)) self->parseJsonFile(jsonPath);
        
        if (self->m_serverFirstNames.isEmpty()) { QProcess ps; ps.start("powershell", QStringList() << "-Command" << QString("Invoke-WebRequest -Uri 'https://api.blackrussia.online/servers.json' -OutFile '%1'").arg(jsonPath)); ps.waitForFinished(10000); self->parseJsonFile(jsonPath); }
        
        if (self->m_serverFirstNames.isEmpty()) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Список серверов не загружен."); }, Qt::QueuedConnection); return; }

        QString app = KoTeKTools::outputPath(), tmp = app + "/logos_temp"; QDir().mkpath(tmp);
        
        for (const QString &n : self->m_serverFirstNames) { QString d = tmp + "/logo" + n + ".btx"; if (QFile::exists(d)) QFile::remove(d); QFile::copy(s, d); }

        QString zip = app + "/logos.zip"; if (QFile::exists(zip)) QFile::remove(zip);
        
        // libzip упаковка
        int error;
        zip_t *newZip = zip_open(zip.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
        if (newZip) {
            QDirIterator it(tmp, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                QString filePath = it.filePath();
                QString fileName = QFileInfo(filePath).fileName();
                zip_source_t *source = zip_source_file(newZip, filePath.toUtf8().constData(), 0, 0);
                if (source) {
                    zip_file_add(newZip, fileName.toUtf8().constData(), source, ZIP_FL_OVERWRITE);
                }
            }
            zip_close(newZip);
        }
        
        QDir(tmp).removeRecursively();

        if (QFile::exists(zip)) QMetaObject::invokeMethod(self, [self]() { self->showNotify("Готово! логотипы созданы (" + QString::number(self->m_serverFirstNames.size()) + " шт.)"); }, Qt::QueuedConnection);
        
        else QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось создать архив."); }, Qt::QueuedConnection);
    });
}

void KoTeKTools::onBrowseListva() { QString f = QFileDialog::getOpenFileName(this, "Листва", QString(), "BTX (*.btx);;All (*)"); if (!f.isEmpty()) m_listvaPath->setText(f); }

void KoTeKTools::onCopyListva() {
    // копирование btx(listva)
    QString s = m_listvaPath->text(); if (s.isEmpty() || !QFile::exists(s)) { QMessageBox::information(this, "Информация", "Выберите файл."); return; }
    KoTeKTools *self = this;
    runAsync([self, s]() {
        QString app = KoTeKTools::outputPath(), tmp = app + "/listva_temp"; QDir().mkpath(tmp);

        QStringList names = getListvaNames();
        for (const QString &n : names) { if (QFile::exists(tmp+"/"+n)) QFile::remove(tmp+"/"+n); QFile::copy(s, tmp+"/"+n); }

        QString zip = app + "/listva.zip"; if (QFile::exists(zip)) QFile::remove(zip);
        
        // libzip упаковка
        int error;
        zip_t *newZip = zip_open(zip.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
        if (newZip) {
            QDirIterator it(tmp, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                QString filePath = it.filePath();
                QString fileName = QFileInfo(filePath).fileName();
                zip_source_t *source = zip_source_file(newZip, filePath.toUtf8().constData(), 0, 0);
                if (source) {
                    zip_file_add(newZip, fileName.toUtf8().constData(), source, ZIP_FL_OVERWRITE);
                }
            }
            zip_close(newZip);
        }
        
        QDir(tmp).removeRecursively();

        if (QFile::exists(zip)) QMetaObject::invokeMethod(self, [self, names]() { self->showNotify("Готово! листва создана (" + QString::number(names.size()) + " шт.)"); }, Qt::QueuedConnection);
        
        else QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось создать архив."); }, Qt::QueuedConnection);
    });
}