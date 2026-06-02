#include <QApplication>
#include "KoTeKTools.h"
#include "map.h"
#include <QColorDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>

void KoTeKTools::onPickBloodColor() { QColor c = QColorDialog::getColor(Qt::red, this, "Цвет крови"); if (c.isValid()) m_bloodColor->setText(c.name().toUpper()); }
void KoTeKTools::onBrowseMapImage() { QString f = QFileDialog::getOpenFileName(this, "Карта", QString(), "Images (*.png *.jpg);;All (*)"); if (!f.isEmpty()) m_mapImagePath->setText(f); }

void KoTeKTools::onGenerateBlood() {
    QString hex = m_bloodColor->text().trimmed(), sizeStr = m_bloodSize->text().trimmed();
    if (hex.isEmpty()) { QMessageBox::information(this, "Информация", "Введите HEX цвет."); return; }
    if (!hex.startsWith("#") || hex.length() != 7) { QMessageBox::critical(this, "Ошибка", "Неверный HEX."); return; }
    float size = 1.01f; if (!sizeStr.isEmpty()) { bool ok; size = sizeStr.toFloat(&ok); if (!ok) size = 1.01f; }
    KoTeKTools *self = this;
    runAsync([self, hex, size]() {
        QFile tmplFile("sources/particle.cfg");
        if (!tmplFile.open(QIODevice::ReadOnly)) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Шаблон particle.cfg не найден."); }, Qt::QueuedConnection); return; }
        QString content = tmplFile.readAll(); tmplFile.close();
        int r = hex.mid(1,2).toInt(nullptr,16), g = hex.mid(3,2).toInt(nullptr,16), b = hex.mid(5,2).toInt(nullptr,16);
        QStringList lines = content.split("\n"); bool found = false;
        for (int i = 0; i < lines.size(); i++) {
            if (lines[i].contains("BLOOD")) {
                QStringList parts = lines[i].replace('\t', ' ').split(' ', Qt::SkipEmptyParts);
                if (parts.size() >= 10) { parts[1]=QString::number(r); parts[2]=QString::number(g); parts[3]=QString::number(b); parts[9]=QString::number(size,'f',3); lines[i]=parts.join("\t"); found=true; } break;
            }
        }
        if (!found) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Секция BLOOD не найдена."); }, Qt::QueuedConnection); return; }
        QString out = KoTeKTools::outputPath() + "/particle.cfg";
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(lines.join("\n").toUtf8()); o.close();
        QMetaObject::invokeMethod(self, [self]() { self->showExtraNotify("Готово! Файл крови создана"); }, Qt::QueuedConnection);
    });
}

void KoTeKTools::onGenerateMapSlice() {
    QString path = m_mapImagePath->text();
    if (path.isEmpty()) { QMessageBox::information(this, "Информация", "Выберите изображение."); return; }
    KoTeKTools *self = this;
    runAsync([self, path]() {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не могу открыть файл."); }, Qt::QueuedConnection); return; }
        QByteArray data = f.readAll(); f.close();
        QByteArray r = Map::sliceMapImage(data);
        if (r.isEmpty()) { QMetaObject::invokeMethod(self, [self]() { QMessageBox::critical(self, "Ошибка", "Не удалось нарезать карту."); }, Qt::QueuedConnection); return; }
        QString out = KoTeKTools::outputPath() + "/map_sliced.zip";
        QFile o(out); o.open(QIODevice::WriteOnly); o.write(r); o.close();
        QMetaObject::invokeMethod(self, [self]() { self->showExtraNotify("Готово! Карта нарезана"); }, Qt::QueuedConnection);
    });
}