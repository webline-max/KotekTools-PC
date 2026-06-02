#include <QApplication>
#include "KoTeKTools.h"
#include "painting.h"
#include <QColorDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>

void KoTeKTools::onPickHudColor() { QColor c = QColorDialog::getColor(Qt::white, this, "Элементы худа"); if (c.isValid()) m_hudColor->setText(c.name().toUpper()); }

void KoTeKTools::onPickButtonsColor() { QColor c = QColorDialog::getColor(Qt::white, this, "Кнопки"); if (c.isValid()) m_buttonsColor->setText(c.name().toUpper()); }

void KoTeKTools::onPickPaintImageColor() { QColor c = QColorDialog::getColor(Qt::white, this, "Изображение"); if (c.isValid()) m_paintImageColor->setText(c.name().toUpper()); }

void KoTeKTools::onBrowsePaintImage() { QString f = QFileDialog::getOpenFileName(this, "Изображение", QString(), "Images (*.png *.jpg *.jpeg *.zip);;All (*)"); if (!f.isEmpty()) m_paintImagePath->setText(f); }

void KoTeKTools::onGenerateHud() {
    QString hex = m_hudColor->text().trimmed();
    if (hex.isEmpty()) { QMessageBox::information(this, "Внимание", "Введите HEX цвет."); return; }
    
    if (!Painting::isValidHex(hex)) { QMessageBox::critical(this, "Ошибка", "Неверный HEX."); return; }
    
    KoTeKTools *self = this;
    runAsync([self, hex]() {
        QByteArray r = Painting::createHudZip(hex);
        if (r.isEmpty()) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось покрасить элементы-худа."); }, Qt::QueuedConnection); return; }
        QString out = KoTeKTools::outputPath() + "/hud.zip";
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(r); o.close();
        QMetaObject::invokeMethod(self, [self]() { self->showPaintNotify("Готово! Элементы-худа покрашены"); }, Qt::QueuedConnection);
    });
}






void KoTeKTools::onGenerateButtons() {
    QString hex = m_buttonsColor->text().trimmed();
    if (hex.isEmpty()) { QMessageBox::information(this, "Внимание", "Введите HEX цвет."); return; }
    
    if (!Painting::isValidHex(hex)) { QMessageBox::critical(this, "Ошибка", "Неверный HEX."); return; }
    
    KoTeKTools *self = this;
    runAsync([self, hex]() {
        QByteArray r = Painting::createButtonsZip(hex);
        if (r.isEmpty()) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось покрасить кнопки.zip."); }, Qt::QueuedConnection); return; }
        QString out = KoTeKTools::outputPath() + "/buttons.zip";
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(r); o.close();
        QMetaObject::invokeMethod(self, [self]() { self->showPaintNotify("Готово! Кнопки покрашены"); }, Qt::QueuedConnection);
    });
}



void KoTeKTools::onGeneratePaintImage() {
    QString path = m_paintImagePath->text(), hex = m_paintImageColor->text().trimmed();
    if (path.isEmpty() || hex.isEmpty()) { QMessageBox::information(this, "Внимание", "Выберите файл и введите HEX цвет."); return; }
    
    if (!Painting::isValidHex(hex)) { QMessageBox::critical(this, "Ошибка", "Неверный HEX."); return; }
    
    KoTeKTools *self = this;
    runAsync([self, path, hex]() {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не могу открыть файл."); }, Qt::QueuedConnection); return; }
        QByteArray data = f.readAll(); f.close();
        QByteArray r = Painting::colorizeImage(data, hex);
        QFileInfo fi(path);
        QString out = KoTeKTools::outputPath() + "/painted_" + fi.fileName();
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(r); o.close();
        QMetaObject::invokeMethod(self, [self, fi]() { self->showPaintNotify("Готово! painted_" + fi.fileName()); }, Qt::QueuedConnection);
    });
}