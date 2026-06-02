#ifndef PAINTING_H
#define PAINTING_H

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace Painting {
    QByteArray colorizeImage(const QByteArray &imageData, const QString &hexColor);
    QByteArray createHudZip(const QString &hexColor);
    QByteArray createButtonsZip(const QString &hexColor);
    QByteArray processImageZip(const QByteArray &zipData, const QString &hexColor);
    bool isValidHex(const QString &hex);
    
    inline QStringList hudFiles() {
        return {
            "hud_ruble.png", "hud_heart.png", "hud_health_scale.png",
            "hud_armor_scale.png", "hud_armor.png"
        };
    }
    
    inline QStringList buttonFiles() {
        return {
            "accelerate.png", "brake.png", "Brown.png", "cam-toggle.png",
            "crane_down.png", "crane_top.png", "handbrake.png", "horn.png",
            "hud_arrow_left.png", "hud_arrow_right.png", "hud_bike.png",
            "hud_boat.png", "hud_car.png", "hud_chopper.png", "hud_circle.png",
            "hud_daily_case_active.png", "hud_daily_case.png", "hud_dildo2.png",
            "hud_lockon.png", "hud_monstertruck.png", "hud_nitro.png",
            "hud_swim.png", "leftshoot.png", "punch.png", "radio_widget.png",
            "shoot.png", "sprint.png", "WidgetGetIn.png"
        };
    }
}

#endif