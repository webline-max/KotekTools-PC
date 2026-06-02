#include "KoTeKTools.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

    // кнопки

QGroupBox* KoTeKTools::createGlassCard(const QString &title, QWidget *content,
                                       QPushButton *&browseBtn, QPushButton *&actionBtn,
                                       const QString &btnText)
{
    QGroupBox *card = new QGroupBox(title);
    card->setFlat(true);
    card->setStyleSheet(
        "QGroupBox {"
        "  background-color: rgba(28, 32, 55, 160);"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 14px;"
        "  padding-top: 32px;"
        "  margin: 0px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 14px;"
        "  top: 6px;"
        "  padding: 0px 8px;"
        "  color: #d0d6f0;"
        "  font-family: 'MiSans';"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  background: transparent;"
        "}"
    );

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 20, 80, 100));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(10);
    lay->addWidget(content);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);
    btnRow->setContentsMargins(0, 2, 0, 0);

    browseBtn = new QPushButton("Выбрать файл");
    browseBtn->setCursor(Qt::PointingHandCursor);
    browseBtn->setFixedHeight(42);
    browseBtn->setStyleSheet(
        "QPushButton { outline: none;"
        "  background-color: rgba(35, 40, 70, 200);"
        "  color: #b0b8e0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 16px;"
        "  font-size: 12px;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(45, 50, 90, 230);"
        "  border-color: rgba(100, 120, 200, 180);"
        "  color: #d0d8ff;"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(55, 60, 110, 250);"
        "}"
    );
    btnRow->addWidget(browseBtn);

    actionBtn = new QPushButton(btnText);
    actionBtn->setCursor(Qt::PointingHandCursor);
    actionBtn->setFixedHeight(42);
    actionBtn->setStyleSheet(
        "QPushButton { outline: none;"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1e6040, stop:1 #143828);"
        "  color: #c0f0d0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #24784e, stop:1 #18482e);"
        "  border-color: #3ea060;"
        "  color: #e0ffe0;"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #2a9058, stop:1 #1c5834);"
        "}"
    );
    btnRow->addWidget(actionBtn);

    lay->addLayout(btnRow);
    return card;
}

QGroupBox* KoTeKTools::createAtmoCard(const QString &title, QLineEdit *&hexEdit, QPushButton *&colorBtn)
{
    QGroupBox *card = new QGroupBox(title);
    card->setFlat(true);
    card->setStyleSheet(
        "QGroupBox {"
        "  background-color: rgba(28, 32, 55, 160);"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 14px;"
        "  padding-top: 32px;"
        "  margin: 0px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 14px;"
        "  top: 6px;"
        "  padding: 0px 8px;"
        "  color: #d0d6f0;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  background: transparent;"
        "}"
    );

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 20, 80, 100));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(8);

    QHBoxLayout *hexRow = new QHBoxLayout();
    hexRow->setSpacing(6);
    
    hexEdit = new QLineEdit();
    hexEdit->setPlaceholderText("#FFFFFF");
    hexEdit->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    hexEdit->setFixedHeight(42);
    hexRow->addWidget(hexEdit);

    colorBtn = new QPushButton();
    colorBtn->setIcon(QIcon("icons/painting.png"));
    colorBtn->setIconSize(QSize(28, 28));
    colorBtn->setCursor(Qt::PointingHandCursor);
    colorBtn->setFixedSize(42, 42);
    colorBtn->setStyleSheet(
        "QPushButton { outline: none;"
        "  background-color: rgba(35, 40, 70, 200);"
        "  border: none;"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(45, 50, 90, 230);"
        "}"
    );
    hexRow->addWidget(colorBtn);
    lay->addLayout(hexRow);

    return card;
}