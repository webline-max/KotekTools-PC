#include <QApplication>
#include "KoTeKTools.h"
#include "atmosphere.h"
#include <QColorDialog>
#include <QMessageBox>
#include <QFile>

void KoTeKTools::onPickSkyTopColor() { QColor c = QColorDialog::getColor(Qt::white, this, "Верх неба"); if (c.isValid()) m_skyTopColor->setText(c.name().toUpper()); }
void KoTeKTools::onPickSkyBottomColor() { QColor c = QColorDialog::getColor(Qt::white, this, "Низ неба"); if (c.isValid()) m_skyBottomColor->setText(c.name().toUpper()); }
void KoTeKTools::onPickSunColor() { QColor c = QColorDialog::getColor(Qt::white, this, "Солнце"); if (c.isValid()) m_sunColor->setText(c.name().toUpper()); }
void KoTeKTools::onPickCloudsColor() { QColor c = QColorDialog::getColor(Qt::white, this, "Облака"); if (c.isValid()) m_cloudsColor->setText(c.name().toUpper()); }
void KoTeKTools::onPickEnvironmentColor() { QColor c = QColorDialog::getColor(Qt::white, this, "Окружение"); if (c.isValid()) m_environmentColor->setText(c.name().toUpper()); }

void KoTeKTools::onGenerateTimecyc() {
    QString top = m_skyTopColor->text().trimmed(), bottom = m_skyBottomColor->text().trimmed(), sun = m_sunColor->text().trimmed(), clouds = m_cloudsColor->text().trimmed();
    if (top.isEmpty() || bottom.isEmpty() || sun.isEmpty() || clouds.isEmpty()) { QMessageBox::information(this, "Внимание", "Заполните все 4 цвета."); return; }
    if (!Atmosphere::isValidHex(top) || !Atmosphere::isValidHex(bottom) || !Atmosphere::isValidHex(sun) || !Atmosphere::isValidHex(clouds)) { QMessageBox::critical(this, "Ошибка", "Неверный HEX."); return; }
    KoTeKTools *self = this;
    runAsync([self, top, bottom, sun, clouds]() {
        QByteArray r = Atmosphere::generateTimecyc(top, bottom, sun, clouds);
        if (r.isEmpty()) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Шаблон не найден."); }, Qt::QueuedConnection); return; }
        QString out = KoTeKTools::outputPath() + "/timecyc.json";
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(r); o.close();
        QMetaObject::invokeMethod(self, [self]() { self->showAtmoNotify("Готово! файл неба создан"); }, Qt::QueuedConnection);
    });
}

void KoTeKTools::onGenerateEnvironment() {
    QString val = m_environmentColor->text().trimmed();
    if (val.isEmpty()) { QMessageBox::information(this, "Внимание", "Введите HEX цвет."); return; }
    if (!Atmosphere::isValidHex(val)) { QMessageBox::critical(this, "Ошибка", "Неверный HEX."); return; }
    KoTeKTools *self = this;
    runAsync([self, val]() {
        QByteArray r = Atmosphere::generateColorcycle(val);
        if (r.isEmpty()) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Шаблон не найден."); }, Qt::QueuedConnection); return; }
        QString out = KoTeKTools::outputPath() + "/colorcycle.dat";
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(r); o.close();
        QMetaObject::invokeMethod(self, [self]() { self->showAtmoNotify("Готово! файл окружения создан"); }, Qt::QueuedConnection);
    });
}