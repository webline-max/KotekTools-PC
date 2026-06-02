#include "atmosphere.h"
#include <QFile>
#include <QRegularExpression>
#include <QApplication>

namespace Atmosphere {

bool isValidHex(const QString &hex) {
    QRegularExpression re("^#[0-9A-Fa-f]{6}$");
    return re.match(hex).hasMatch();
}

QColor hexToColor(const QString &hex) {
    return QColor(hex.mid(1,2).toInt(nullptr,16),
                  hex.mid(3,2).toInt(nullptr,16),
                  hex.mid(5,2).toInt(nullptr,16));
}

QByteArray generateTimecyc(const QString &topHex, const QString &bottomHex,
                           const QString &sunHex, const QString &cloudsHex) {
    QFile tmplFile("sources/timecyc.json");
    if (!tmplFile.open(QIODevice::ReadOnly)) return QByteArray();
    QString tmpl = tmplFile.readAll();
    tmplFile.close();

    auto rgb = [](const QString &hex) {
        return QList<int>{hex.mid(1,2).toInt(nullptr,16), hex.mid(3,2).toInt(nullptr,16), hex.mid(5,2).toInt(nullptr,16)};
    };

    QList<int> t = rgb(topHex), b = rgb(bottomHex), s = rgb(sunHex), c = rgb(cloudsHex);

    tmpl.replace("skbr", QString::number(t[0])); tmpl.replace("skbg", QString::number(t[1])); tmpl.replace("skbb", QString::number(t[2]));
    tmpl.replace("sktr", QString::number(b[0])); tmpl.replace("sktg", QString::number(b[1])); tmpl.replace("sktb", QString::number(b[2]));
    tmpl.replace("scr", QString::number(s[0]));  tmpl.replace("scg", QString::number(s[1]));  tmpl.replace("scb", QString::number(s[2]));
    tmpl.replace("clr", QString::number(c[0]));  tmpl.replace("clg", QString::number(c[1]));  tmpl.replace("clb", QString::number(c[2]));

    return tmpl.toUtf8();
}

QByteArray generateColorcycle(const QString &hexColor) {
    QFile tmplFile("sources/colorcyc.dat");
    if (!tmplFile.open(QIODevice::ReadOnly)) return QByteArray();
    QString data = tmplFile.readAll();
    tmplFile.close();

    float r = hexColor.mid(1,2).toInt(nullptr,16) / 255.0f;
    float g = hexColor.mid(3,2).toInt(nullptr,16) / 255.0f;
    float b = hexColor.mid(5,2).toInt(nullptr,16) / 255.0f;

    data.replace("r", QString::number(r, 'f', 3));
    data.replace("g", QString::number(g, 'f', 3));
    data.replace("b", QString::number(b, 'f', 3));

    return data.toUtf8();
}

}