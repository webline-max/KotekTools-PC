#include "KoTeKTools.h"
#include <QGraphicsOpacityEffect>
#include <QTimer>

void KoTeKTools::showNotify(const QString &msg) 
{ 
    m_notificationLabel->setText(msg); 
    m_notificationLabel->setVisible(true); 
    QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(m_notificationLabel->graphicsEffect()); 
    if (e) e->setOpacity(1.0); 
    m_notificationTimer->start(3000); 
}

void KoTeKTools::showNotifyConv(const QString &msg)
{
    QLabel *convNotify = m_convertPage->findChild<QLabel*>("convNotify");
    if (convNotify) {
        convNotify->setText(msg);
        convNotify->setVisible(true);
        QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(convNotify->graphicsEffect());
        if (e) e->setOpacity(1.0);
        QTimer::singleShot(3000, this, [convNotify]() {
            QGraphicsOpacityEffect *ef = qobject_cast<QGraphicsOpacityEffect*>(convNotify->graphicsEffect());
            if (ef) ef->setOpacity(0.0);
            convNotify->setVisible(false);
        });
    }
}

void KoTeKTools::showAtmoNotify(const QString &msg)
{
    if (m_atmoNotifyLabel) {
        m_atmoNotifyLabel->setText(msg);
        m_atmoNotifyLabel->setVisible(true);
        QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(m_atmoNotifyLabel->graphicsEffect());
        if (e) e->setOpacity(1.0);
        m_atmoTimer->start(3000);
    }
}

void KoTeKTools::showPaintNotify(const QString &msg)
{
    if (m_paintNotifyLabel) {
        m_paintNotifyLabel->setText(msg);
        m_paintNotifyLabel->setVisible(true);
        QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(m_paintNotifyLabel->graphicsEffect());
        if (e) e->setOpacity(1.0);
        m_paintTimer->start(3000);
    }
}

void KoTeKTools::showExtraNotify(const QString &msg)
{
    if (m_extraNotifyLabel) {
        m_extraNotifyLabel->setText(msg);
        m_extraNotifyLabel->setVisible(true);
        QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(m_extraNotifyLabel->graphicsEffect());
        if (e) e->setOpacity(1.0);
        m_extraTimer->start(3000);
    }
}

void KoTeKTools::hideNotification() 
{ 
    QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(m_notificationLabel->graphicsEffect()); 
    if (e) e->setOpacity(0.0); 
    m_notificationLabel->setVisible(false); 
    QLabel *convNotify = m_convertPage->findChild<QLabel*>("convNotify"); 
    if (convNotify) { 
        QGraphicsOpacityEffect *ef = qobject_cast<QGraphicsOpacityEffect*>(convNotify->graphicsEffect()); 
        if (ef) ef->setOpacity(0.0); 
        convNotify->setVisible(false); 
    } 
}