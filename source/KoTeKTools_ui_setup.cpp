#include "KoTeKTools.h"
#include <QSettings>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QFontDatabase>
#include <QStackedWidget>

void KoTeKTools::setupUi()
{

    // окно
    setWindowTitle("KoTeK Tools");
    setFixedSize(990, 660);

    // кнопка свернуть
    m_btnMinimize = new QPushButton(this);
    m_btnMinimize->setIcon(m_iconWrap);
    m_btnMinimize->setIconSize(QSize(30, 30));
    m_btnMinimize->setFixedSize(40, 40);
    m_btnMinimize->setCursor(Qt::PointingHandCursor);
    m_btnMinimize->setStyleSheet("QPushButton { outline: none; background: transparent; border: none; } QPushButton:hover { background: rgba(255,255,255,20); border-radius: 8px; }");
    connect(m_btnMinimize, &QPushButton::clicked, this, [this]() { showMinimized(); });


    // кнопка закрыть
    m_btnClose = new QPushButton(this);
    m_btnClose->setIcon(m_iconClose);
    m_btnClose->setIconSize(QSize(30, 30));
    m_btnClose->setFixedSize(40, 40);
    m_btnClose->setCursor(Qt::PointingHandCursor);
    m_btnClose->setStyleSheet("QPushButton { outline: none; background: transparent; border: none; } QPushButton:hover { background: rgba(255,80,80,70); border-radius: 8px; }");
    connect(m_btnClose, &QPushButton::clicked, this, [this]() { qApp->quit(); });


    // шрифт
    QString fontPath = QApplication::applicationDirPath() + "/MiSans-Bold.ttf";
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont customFont(family, 12);
        customFont.setWeight(QFont::Black);
        QApplication::setFont(customFont);
    }
    

    // фон окна
    setStyleSheet(
        "KoTeKTools {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #0e1020, stop:0.3 #121832, stop:0.7 #0f1428, stop:1 #0c0f1a);"
        "}"
    );
    

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // боковая панель
    m_sidePanel = new QFrame(this);
    m_sidePanel->setFixedWidth(0);
    m_sidePanel->setStyleSheet("QFrame { background-color: rgba(16, 20, 38, 245); border-right: 2px solid rgba(80, 100, 180, 50); }");
    QGraphicsDropShadowEffect *panelShadow = new QGraphicsDropShadowEffect(m_sidePanel);
    panelShadow->setBlurRadius(35); panelShadow->setOffset(4, 0);
    panelShadow->setColor(QColor(0, 10, 40, 160));
    m_sidePanel->setGraphicsEffect(panelShadow);

    QVBoxLayout *panelLayout = new QVBoxLayout(m_sidePanel);
    panelLayout->setContentsMargins(14, 14, 14, 14);
    panelLayout->setSpacing(8);

    // заголовок панели
    QHBoxLayout *panelHeader = new QHBoxLayout();
    QLabel *panelTitle = new QLabel("Меню");
    panelTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 15px; font-weight: bold; background: transparent; border: none; }");
    panelTitle->setGraphicsEffect(nullptr);
    panelHeader->addWidget(panelTitle); panelHeader->addStretch();

    QPushButton *closePanelBtn = new QPushButton();
    closePanelBtn->setIcon(m_iconClose);
    closePanelBtn->setIconSize(QSize(26, 26));
    closePanelBtn->setCursor(Qt::PointingHandCursor); closePanelBtn->setFixedSize(36, 36);
    
    closePanelBtn->setStyleSheet("QPushButton { outline: none; background: transparent; border: none; border-radius: 10px; } QPushButton:hover { background: rgba(255,80,80,70); }");
    
    connect(closePanelBtn, &QPushButton::clicked, this, &KoTeKTools::closePanel);
    
    panelHeader->addWidget(closePanelBtn);
    panelLayout->addLayout(panelHeader);

    // пункты меню
    auto menuBtn = [](const QString &text) {
    QPushButton *btn = new QPushButton(text);
    btn->setCursor(Qt::PointingHandCursor); btn->setFixedHeight(46);
    btn->setStyleSheet("QPushButton { outline: none; background-color: rgba(28, 32, 55, 180); color: #b0b8e0; border: none; border-radius: 12px; padding: 0px 16px; font-size: 13px; font-weight: 500; text-align: left; } QPushButton:hover { background-color: rgba(38, 42, 70, 220); color: #d0d8ff; }");
    return btn;
    };

    QPushButton *menuCopy = menuBtn("Копирование");
    connect(menuCopy, &QPushButton::clicked, this, &KoTeKTools::showCopyPage);
    panelLayout->addWidget(menuCopy);
    

    QPushButton *menuConvert = menuBtn("Конвертация");
    connect(menuConvert, &QPushButton::clicked, this, &KoTeKTools::showConvertPage);
    panelLayout->addWidget(menuConvert);
    

    QPushButton *menuAtmosphere = menuBtn("Атмосфера");
    connect(menuAtmosphere, &QPushButton::clicked, this, &KoTeKTools::showAtmospherePage);
    panelLayout->addWidget(menuAtmosphere);
    

    QPushButton *menuPainting = menuBtn("Покраска");
    connect(menuPainting, &QPushButton::clicked, this, &KoTeKTools::showPaintingPage);
    panelLayout->addWidget(menuPainting);
    

    QPushButton *menuExtra = menuBtn("Дополнительно");
    connect(menuExtra, &QPushButton::clicked, this, &KoTeKTools::showExtraPage);
    panelLayout->addWidget(menuExtra);
    

    QPushButton *menuTexturing = menuBtn("Текстурирование");
    connect(menuTexturing, &QPushButton::clicked, this, &KoTeKTools::showTexturingPage);
    panelLayout->addWidget(menuTexturing);
    

    QPushButton *menu3DViewer = menuBtn("3D просмотр");
    connect(menu3DViewer, &QPushButton::clicked, this, &KoTeKTools::show3DViewPage);
    panelLayout->addWidget(menu3DViewer);

    panelLayout->addStretch();

    // кнопка настроек
    QPushButton *settingsBtn = new QPushButton();
    settingsBtn->setIcon(QIcon("icons/settings.png"));
    settingsBtn->setIconSize(QSize(24, 24));
    settingsBtn->setCursor(Qt::PointingHandCursor);
    settingsBtn->setFixedSize(42, 42);
    settingsBtn->setStyleSheet("QPushButton { outline: none; background-color: rgba(35, 40, 70, 180); border: none; border-radius: 21px; } QPushButton:hover { background-color: rgba(45, 50, 90, 230); }");
    connect(settingsBtn, &QPushButton::clicked, this, &KoTeKTools::showSettingsPage);
    panelLayout->addWidget(settingsBtn);


    m_centerStack = new QStackedWidget();
    m_centerStack->setStyleSheet("QStackedWidget { background: transparent; }");


    QString inputStyle = "QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }";
    

    // главная страница с кругом
    m_circlePage = new QWidget();
    QVBoxLayout *cl = new QVBoxLayout(m_circlePage); cl->setAlignment(Qt::AlignCenter); cl->setSpacing(20);
    m_centerBtn = new QPushButton("Открыть\nпанель"); m_centerBtn->setCursor(Qt::PointingHandCursor); m_centerBtn->setFixedSize(260, 260);
    m_centerBtn->setStyleSheet("QPushButton { outline: none; background-color: rgba(28, 32, 55, 180); color: #9098c0; border: none; border-radius: 130px; font-family: 'MiSans'; font-size: 22px; font-weight: bold; } QPushButton:hover { background-color: rgba(35, 40, 70, 220); color: #b8c0e8; }");
    connect(m_centerBtn, &QPushButton::clicked, this, &KoTeKTools::openPanel);
    cl->addWidget(m_centerBtn);
    QLabel *hint = new QLabel("Нажмите на круг чтобы открыть меню\n\nСпасибо, что используете наш продукт!"); hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("QLabel { color: #404870; font-family: 'MiSans'; font-size: 13px; background: transparent; }");
    cl->addWidget(hint);
    m_centerStack->addWidget(m_circlePage);

    // пустая правая страница
    m_emptyPage = new QWidget();
    m_emptyPage->setStyleSheet("background: transparent;");
    m_centerStack->addWidget(m_emptyPage);

    // КОПИРОВАНИЕ
    m_copyPage = new QWidget();
    m_copyPage->setStyleSheet("background: transparent;");
    QVBoxLayout *copyOuter = new QVBoxLayout(m_copyPage); copyOuter->setContentsMargins(0, 0, 0, 0);

    QScrollArea *copyScroll = new QScrollArea();
    copyScroll->setWidgetResizable(true);
    copyScroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollArea > QWidget > QWidget { background: transparent; } QScrollBar:vertical { background: rgba(20, 24, 42, 150); width: 6px; border-radius: 3px; } QScrollBar::handle:vertical { background: rgba(80, 100, 180, 100); border-radius: 3px; min-height: 30px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget *copyInner = new QWidget();
    copyInner->setStyleSheet("background: transparent;");
    QVBoxLayout *copyL = new QVBoxLayout(copyInner); copyL->setContentsMargins(30, 25, 30, 15); copyL->setSpacing(14);

    QLabel *copyTitle = new QLabel("Копирование ресурсов");
    copyTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 16px; font-weight: bold; background: transparent; padding-bottom: 4px; }");
    copyL->addWidget(copyTitle);

    m_billboardPath = new QLineEdit(); m_billboardPath->setPlaceholderText("Путь к файлу билдборда (.btx)"); m_billboardPath->setReadOnly(true); m_billboardPath->setStyleSheet(inputStyle); m_billboardPath->setFixedHeight(42);
    copyL->addWidget(createGlassCard("Копирование билдбордов", m_billboardPath, m_btnBrowseBillboard, m_btnCopyBillboard, "Копировать"));
    connect(m_btnBrowseBillboard, &QPushButton::clicked, this, &KoTeKTools::onBrowseBillboard);
    connect(m_btnCopyBillboard, &QPushButton::clicked, this, &KoTeKTools::onCopyBillboard);

    m_logoPath = new QLineEdit(); m_logoPath->setPlaceholderText("Путь к файлу логотипа (.btx)"); m_logoPath->setReadOnly(true); m_logoPath->setStyleSheet(inputStyle); m_logoPath->setFixedHeight(42);
    copyL->addWidget(createGlassCard("Копирование логотипов", m_logoPath, m_btnBrowseLogo, m_btnCopyLogo, "Копировать"));
    connect(m_btnBrowseLogo, &QPushButton::clicked, this, &KoTeKTools::onBrowseLogo);
    connect(m_btnCopyLogo, &QPushButton::clicked, this, &KoTeKTools::onCopyLogos);

    m_listvaPath = new QLineEdit(); m_listvaPath->setPlaceholderText("Путь к файлу листвы (.btx)"); m_listvaPath->setReadOnly(true); m_listvaPath->setStyleSheet(inputStyle); m_listvaPath->setFixedHeight(42);
    copyL->addWidget(createGlassCard("Копирование листвы", m_listvaPath, m_btnBrowseListva, m_btnCopyListva, "Копировать"));
    connect(m_btnBrowseListva, &QPushButton::clicked, this, &KoTeKTools::onBrowseListva);
    connect(m_btnCopyListva, &QPushButton::clicked, this, &KoTeKTools::onCopyListva);

    m_notificationLabel = new QLabel(); m_notificationLabel->setAlignment(Qt::AlignCenter);
    m_notificationLabel->setStyleSheet("QLabel { background-color: rgba(30, 96, 64, 200); color: #c0f0d0; border: none; border-radius: 12px; padding: 10px 14px; font-family: 'MiSans'; font-size: 12px; font-weight: bold; }");
    m_notificationLabel->setVisible(false); m_notificationLabel->setFixedHeight(44);
    QGraphicsOpacityEffect *oe1 = new QGraphicsOpacityEffect(m_notificationLabel); oe1->setOpacity(0.0);
    m_notificationLabel->setGraphicsEffect(oe1);
    copyL->addWidget(m_notificationLabel);
    copyL->addStretch();

    copyScroll->setWidget(copyInner);
    copyOuter->addWidget(copyScroll);
    m_centerStack->addWidget(m_copyPage);

    // 3 = КОНВЕРТАЦИЯ
    m_convertPage = new QWidget();
    m_convertPage->setStyleSheet("background: transparent;");
    QVBoxLayout *convOuter = new QVBoxLayout(m_convertPage); convOuter->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollArea > QWidget > QWidget { background: transparent; } QScrollBar:vertical { background: rgba(20, 24, 42, 150); width: 6px; border-radius: 3px; } QScrollBar::handle:vertical { background: rgba(80, 100, 180, 100); border-radius: 3px; min-height: 30px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget *convInner = new QWidget();
    convInner->setStyleSheet("background: transparent;");
    QVBoxLayout *convL = new QVBoxLayout(convInner); convL->setContentsMargins(30, 25, 30, 15); convL->setSpacing(14);

    QLabel *convTitle = new QLabel("Конвертация файлов");
    convTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 16px; font-weight: bold; background: transparent; padding-bottom: 4px; }");
    convL->addWidget(convTitle);

    QGridLayout *convGrid = new QGridLayout();
    convGrid->setSpacing(14);
    convGrid->setContentsMargins(0, 0, 0, 0);

    m_bpcPath = new QLineEdit(); m_bpcPath->setPlaceholderText("Путь к файлу .bpc"); m_bpcPath->setReadOnly(true); m_bpcPath->setStyleSheet(inputStyle); m_bpcPath->setFixedHeight(42);
    convGrid->addWidget(createGlassCard("BPC / ZIP", m_bpcPath, m_btnBrowseBpc, m_btnConvertBpc, "Конвертировать"), 0, 0);
    connect(m_btnBrowseBpc, &QPushButton::clicked, this, &KoTeKTools::onBrowseBpc);
    connect(m_btnConvertBpc, &QPushButton::clicked, this, &KoTeKTools::onConvertBpcToZip);

    m_bpcMetaPath = new QLineEdit(); m_bpcMetaPath->setPlaceholderText("Путь к файлу .bpc"); m_bpcMetaPath->setReadOnly(true); m_bpcMetaPath->setStyleSheet(inputStyle); m_bpcMetaPath->setFixedHeight(42);
    convGrid->addWidget(createGlassCard("BPC / BPCMETA", m_bpcMetaPath, m_btnBrowseBpcMeta, m_btnConvertBpcMeta, "Конвертировать"), 0, 1);
    connect(m_btnBrowseBpcMeta, &QPushButton::clicked, this, &KoTeKTools::onBrowseBpcMeta);
    connect(m_btnConvertBpcMeta, &QPushButton::clicked, this, &KoTeKTools::onConvertBpcToMeta);

    m_ifpAniPath = new QLineEdit(); m_ifpAniPath->setPlaceholderText("Путь к файлу .ifp или .ani"); m_ifpAniPath->setReadOnly(true); m_ifpAniPath->setStyleSheet(inputStyle); m_ifpAniPath->setFixedHeight(42);
    convGrid->addWidget(createGlassCard("IFP / ANI", m_ifpAniPath, m_btnBrowseIfpAni, m_btnConvertIfpAni, "Конвертировать"), 1, 0);
    connect(m_btnBrowseIfpAni, &QPushButton::clicked, this, &KoTeKTools::onBrowseIfpAni);
    connect(m_btnConvertIfpAni, &QPushButton::clicked, this, &KoTeKTools::onConvertIfpAni);

    m_modPath = new QLineEdit(); m_modPath->setPlaceholderText("Путь к файлу .mod или .dff"); m_modPath->setReadOnly(true); m_modPath->setStyleSheet(inputStyle); m_modPath->setFixedHeight(42);
    convGrid->addWidget(createGlassCard("MOD / DFF", m_modPath, m_btnBrowseMod, m_btnConvertMod, "Конвертировать"), 1, 1);
    connect(m_btnBrowseMod, &QPushButton::clicked, this, &KoTeKTools::onBrowseMod);
    connect(m_btnConvertMod, &QPushButton::clicked, this, &KoTeKTools::onConvertMod);

    m_txdPath = new QLineEdit(); m_txdPath->setPlaceholderText("Путь к файлу .txd"); m_txdPath->setReadOnly(true); m_txdPath->setStyleSheet(inputStyle); m_txdPath->setFixedHeight(42);
    convGrid->addWidget(createGlassCard("TXD / ZIP", m_txdPath, m_btnBrowseTxd, m_btnConvertTxd, "Конвертировать"), 2, 0);
    connect(m_btnBrowseTxd, &QPushButton::clicked, this, &KoTeKTools::onBrowseTxd);
    connect(m_btnConvertTxd, &QPushButton::clicked, this, &KoTeKTools::onConvertTxd);

    m_btxPath = new QLineEdit(); m_btxPath->setPlaceholderText("Путь к файлу .btx или .png"); m_btxPath->setReadOnly(true); m_btxPath->setStyleSheet(inputStyle); m_btxPath->setFixedHeight(42);
    convGrid->addWidget(createGlassCard("BTX / PNG", m_btxPath, m_btnBrowseBtx, m_btnConvertBtx, "Конвертировать"), 2, 1);
    connect(m_btnBrowseBtx, &QPushButton::clicked, this, &KoTeKTools::onBrowseBtx);
    connect(m_btnConvertBtx, &QPushButton::clicked, this, &KoTeKTools::onConvertBtx);

    convL->addLayout(convGrid);

    QLabel *convNotify = new QLabel(); convNotify->setAlignment(Qt::AlignCenter);
    convNotify->setStyleSheet("QLabel { background-color: rgba(30, 96, 64, 200); color: #c0f0d0; border: none; border-radius: 12px; padding: 10px 14px; font-family: 'MiSans'; font-size: 12px; font-weight: bold; }");
    convNotify->setVisible(false); convNotify->setFixedHeight(44); convNotify->setObjectName("convNotify");
    QGraphicsOpacityEffect *oe2 = new QGraphicsOpacityEffect(convNotify); oe2->setOpacity(0.0);
    convNotify->setGraphicsEffect(oe2);
    convL->addWidget(convNotify);
    convL->addStretch();

    scroll->setWidget(convInner);
    convOuter->addWidget(scroll);
    m_centerStack->addWidget(m_convertPage);

    // 4 = АТМОСФЕРА
    m_atmospherePage = new QWidget();
    m_atmospherePage->setStyleSheet("background: transparent;");
    QVBoxLayout *atmoOuter = new QVBoxLayout(m_atmospherePage); atmoOuter->setContentsMargins(0, 0, 0, 0);

    QScrollArea *atmoScroll = new QScrollArea();
    atmoScroll->setWidgetResizable(true);
    atmoScroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollArea > QWidget > QWidget { background: transparent; } QScrollBar:vertical { background: rgba(20, 24, 42, 150); width: 6px; border-radius: 3px; } QScrollBar::handle:vertical { background: rgba(80, 100, 180, 100); border-radius: 3px; min-height: 30px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget *atmoInner = new QWidget();
    atmoInner->setStyleSheet("background: transparent;");
    QVBoxLayout *atmoL = new QVBoxLayout(atmoInner); atmoL->setContentsMargins(30, 25, 30, 15); atmoL->setSpacing(12);

    QLabel *skyTitle = new QLabel("Изменение неба");
    skyTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 16px; font-weight: bold; background: transparent; padding-bottom: 4px; }");
    atmoL->addWidget(skyTitle);

    QGridLayout *atmoGrid = new QGridLayout();
    atmoGrid->setSpacing(12);
    atmoGrid->setContentsMargins(0, 0, 0, 0);

    atmoGrid->addWidget(createAtmoCard("Верх неба", m_skyTopColor, m_btnSkyTopColor), 0, 0);
    connect(m_btnSkyTopColor, &QPushButton::clicked, this, &KoTeKTools::onPickSkyTopColor);
    atmoGrid->addWidget(createAtmoCard("Низ неба", m_skyBottomColor, m_btnSkyBottomColor), 0, 1);
    connect(m_btnSkyBottomColor, &QPushButton::clicked, this, &KoTeKTools::onPickSkyBottomColor);
    atmoGrid->addWidget(createAtmoCard("Облака", m_cloudsColor, m_btnCloudsColor), 1, 0);
    connect(m_btnCloudsColor, &QPushButton::clicked, this, &KoTeKTools::onPickCloudsColor);
    atmoGrid->addWidget(createAtmoCard("Солнце", m_sunColor, m_btnSunColor), 1, 1);
    connect(m_btnSunColor, &QPushButton::clicked, this, &KoTeKTools::onPickSunColor);

    atmoL->addLayout(atmoGrid);

    m_btnGenerateTimecyc = new QPushButton("Создать файл неба");
    m_btnGenerateTimecyc->setCursor(Qt::PointingHandCursor);
    m_btnGenerateTimecyc->setFixedHeight(46);
    m_btnGenerateTimecyc->setStyleSheet(
        "QPushButton { outline: none;"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1e6040, stop:1 #143828);"
        "  color: #c0f0d0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #24784e, stop:1 #18482e);"
        "  border-color: #3ea060;"
        "  color: #e0ffe0;"
        "}"
    );
    connect(m_btnGenerateTimecyc, &QPushButton::clicked, this, &KoTeKTools::onGenerateTimecyc);
    atmoL->addWidget(m_btnGenerateTimecyc);

    atmoL->addSpacing(10);

    QLabel *envTitle = new QLabel("Изменение окружения");
    envTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 16px; font-weight: bold; background: transparent; padding-bottom: 4px; }");
    atmoL->addWidget(envTitle);

    QGroupBox *envCard = new QGroupBox("Цвет окружения");
    envCard->setFlat(true);
    envCard->setStyleSheet(
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
    QVBoxLayout *envCardLay = new QVBoxLayout(envCard); envCardLay->setContentsMargins(4,4,4,4); envCardLay->setSpacing(8);
    QHBoxLayout *envRow = new QHBoxLayout();
    envRow->setSpacing(6);
    m_environmentColor = new QLineEdit();
    m_environmentColor->setPlaceholderText("#FFFFFF");
    m_environmentColor->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    m_environmentColor->setFixedHeight(42);
    envRow->addWidget(m_environmentColor);
    m_btnEnvironmentColor = new QPushButton();
    m_btnEnvironmentColor->setIcon(QIcon("icons/painting.png"));
    m_btnEnvironmentColor->setIconSize(QSize(28, 28));
    m_btnEnvironmentColor->setCursor(Qt::PointingHandCursor);
    m_btnEnvironmentColor->setFixedSize(42, 42);
    m_btnEnvironmentColor->setStyleSheet("QPushButton { background-color: rgba(35, 40, 70, 200); border: none; border-radius: 12px; } QPushButton:hover { background-color: rgba(45, 50, 90, 230); }");
    connect(m_btnEnvironmentColor, &QPushButton::clicked, this, &KoTeKTools::onPickEnvironmentColor);
    envRow->addWidget(m_btnEnvironmentColor);
    envCardLay->addLayout(envRow);
    atmoL->addWidget(envCard);

    m_btnGenerateEnvironment = new QPushButton("Создать файл окружения");
    m_btnGenerateEnvironment->setCursor(Qt::PointingHandCursor);
    m_btnGenerateEnvironment->setFixedHeight(46);
    m_btnGenerateEnvironment->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1e6040, stop:1 #143828);"
        "  color: #c0f0d0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #24784e, stop:1 #18482e);"
        "  color: #e0ffe0;"
        "}"
    );
    connect(m_btnGenerateEnvironment, &QPushButton::clicked, this, &KoTeKTools::onGenerateEnvironment);
    atmoL->addWidget(m_btnGenerateEnvironment);

    m_atmoNotifyLabel = new QLabel(); m_atmoNotifyLabel->setAlignment(Qt::AlignCenter);
    m_atmoNotifyLabel->setStyleSheet("QLabel { background-color: rgba(30, 96, 64, 200); color: #c0f0d0; border: none; border-radius: 12px; padding: 10px 14px; font-family: 'MiSans'; font-size: 12px; font-weight: bold; }");
    m_atmoNotifyLabel->setVisible(false); m_atmoNotifyLabel->setFixedHeight(44);
    QGraphicsOpacityEffect *oe3 = new QGraphicsOpacityEffect(m_atmoNotifyLabel); oe3->setOpacity(0.0);
    m_atmoNotifyLabel->setGraphicsEffect(oe3);
    atmoL->addWidget(m_atmoNotifyLabel);
    atmoL->addStretch();

    atmoScroll->setWidget(atmoInner);
    atmoOuter->addWidget(atmoScroll);
    m_centerStack->addWidget(m_atmospherePage);

    // 5 = ПОКРАСКА
    m_paintingPage = new QWidget();
    m_paintingPage->setStyleSheet("background: transparent;");
    QVBoxLayout *paintOuter = new QVBoxLayout(m_paintingPage); paintOuter->setContentsMargins(0, 0, 0, 0);

    QScrollArea *paintScroll = new QScrollArea();
    paintScroll->setWidgetResizable(true);
    paintScroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollArea > QWidget > QWidget { background: transparent; } QScrollBar:vertical { background: rgba(20, 24, 42, 150); width: 6px; border-radius: 3px; } QScrollBar::handle:vertical { background: rgba(80, 100, 180, 100); border-radius: 3px; min-height: 30px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget *paintInner = new QWidget();
    paintInner->setStyleSheet("background: transparent;");
    QVBoxLayout *paintL = new QVBoxLayout(paintInner); paintL->setContentsMargins(30, 25, 30, 15); paintL->setSpacing(12);

    QLabel *paintTitle = new QLabel("Покраска ресурсов");
    paintTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 16px; font-weight: bold; background: transparent; padding-bottom: 4px; }");
    paintL->addWidget(paintTitle);

    paintL->addWidget(createAtmoCard("Покраска кнопок", m_buttonsColor, m_btnButtonsColor));
    connect(m_btnButtonsColor, &QPushButton::clicked, this, &KoTeKTools::onPickButtonsColor);
    m_btnGenerateButtons = new QPushButton("Покрасить кнопки");
    m_btnGenerateButtons->setCursor(Qt::PointingHandCursor);
    m_btnGenerateButtons->setFixedHeight(46);
    m_btnGenerateButtons->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1e6040, stop:1 #143828);"
        "  color: #c0f0d0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #24784e, stop:1 #18482e);"
        "  color: #e0ffe0;"
        "}"
    );
    connect(m_btnGenerateButtons, &QPushButton::clicked, this, &KoTeKTools::onGenerateButtons);
    paintL->addWidget(m_btnGenerateButtons);

    paintL->addSpacing(10);

    paintL->addWidget(createAtmoCard("Покраска элементов худа", m_hudColor, m_btnHudColor));
    connect(m_btnHudColor, &QPushButton::clicked, this, &KoTeKTools::onPickHudColor);
    m_btnGenerateHud = new QPushButton("Покрасить элементы худа");
    m_btnGenerateHud->setCursor(Qt::PointingHandCursor);
    m_btnGenerateHud->setFixedHeight(46);
    m_btnGenerateHud->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1e6040, stop:1 #143828);"
        "  color: #c0f0d0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #24784e, stop:1 #18482e);"
        "  color: #e0ffe0;"
        "}"
    );
    connect(m_btnGenerateHud, &QPushButton::clicked, this, &KoTeKTools::onGenerateHud);
    paintL->addWidget(m_btnGenerateHud);

    paintL->addSpacing(10);

    QGroupBox *imgCard = new QGroupBox("Покраска изображений");
    imgCard->setFlat(true);
    imgCard->setStyleSheet(
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
    QVBoxLayout *imgCardLay = new QVBoxLayout(imgCard); imgCardLay->setContentsMargins(4,4,4,4); imgCardLay->setSpacing(8);
    QHBoxLayout *imgPathRow = new QHBoxLayout(); imgPathRow->setSpacing(6);
    m_paintImagePath = new QLineEdit(); m_paintImagePath->setPlaceholderText("Путь к PNG/JPG/ZIP");
    m_paintImagePath->setReadOnly(true);
    m_paintImagePath->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    m_paintImagePath->setFixedHeight(42);
    imgPathRow->addWidget(m_paintImagePath);
    m_btnBrowsePaintImage = new QPushButton();
    m_btnBrowsePaintImage->setIcon(QIcon("icons/photo.png"));
    m_btnBrowsePaintImage->setIconSize(QSize(24, 24));
    m_btnBrowsePaintImage->setCursor(Qt::PointingHandCursor);
    m_btnBrowsePaintImage->setFixedSize(42, 42);
    m_btnBrowsePaintImage->setStyleSheet("QPushButton { background-color: rgba(35, 40, 70, 200); border: none; border-radius: 12px; } QPushButton:hover { background-color: rgba(45, 50, 90, 230); }");
    connect(m_btnBrowsePaintImage, &QPushButton::clicked, this, &KoTeKTools::onBrowsePaintImage);
    imgPathRow->addWidget(m_btnBrowsePaintImage);
    imgCardLay->addLayout(imgPathRow);

    QHBoxLayout *imgHexRow = new QHBoxLayout(); imgHexRow->setSpacing(6);
    m_paintImageColor = new QLineEdit(); m_paintImageColor->setPlaceholderText("#FFFFFF");
    m_paintImageColor->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    m_paintImageColor->setFixedHeight(42);
    imgHexRow->addWidget(m_paintImageColor);
    m_btnPaintImageColor = new QPushButton();
    m_btnPaintImageColor->setIcon(QIcon("icons/painting.png"));
    m_btnPaintImageColor->setIconSize(QSize(28, 28));
    m_btnPaintImageColor->setCursor(Qt::PointingHandCursor);
    m_btnPaintImageColor->setFixedSize(42, 42);
    m_btnPaintImageColor->setStyleSheet("QPushButton { background-color: rgba(35, 40, 70, 200); border: none; border-radius: 12px; } QPushButton:hover { background-color: rgba(45, 50, 90, 230); }");
    connect(m_btnPaintImageColor, &QPushButton::clicked, this, &KoTeKTools::onPickPaintImageColor);
    imgHexRow->addWidget(m_btnPaintImageColor);
    imgCardLay->addLayout(imgHexRow);
    paintL->addWidget(imgCard);

    m_btnGeneratePaintImage = new QPushButton("Покрасить изображение");
    m_btnGeneratePaintImage->setCursor(Qt::PointingHandCursor);
    m_btnGeneratePaintImage->setFixedHeight(46);
    m_btnGeneratePaintImage->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1e6040, stop:1 #143828);"
        "  color: #c0f0d0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #24784e, stop:1 #18482e);"
        "  color: #e0ffe0;"
        "}"
    );
    connect(m_btnGeneratePaintImage, &QPushButton::clicked, this, &KoTeKTools::onGeneratePaintImage);
    paintL->addWidget(m_btnGeneratePaintImage);

    m_paintNotifyLabel = new QLabel(); m_paintNotifyLabel->setAlignment(Qt::AlignCenter);
    m_paintNotifyLabel->setStyleSheet("QLabel { background-color: rgba(30, 96, 64, 200); color: #c0f0d0; border: none; border-radius: 12px; padding: 10px 14px; font-family: 'MiSans'; font-size: 12px; font-weight: bold; }");
    m_paintNotifyLabel->setVisible(false); m_paintNotifyLabel->setFixedHeight(44);
    QGraphicsOpacityEffect *oe4 = new QGraphicsOpacityEffect(m_paintNotifyLabel); oe4->setOpacity(0.0);
    m_paintNotifyLabel->setGraphicsEffect(oe4);
    paintL->addWidget(m_paintNotifyLabel);
    paintL->addStretch();

    paintScroll->setWidget(paintInner);
    paintOuter->addWidget(paintScroll);
    m_centerStack->addWidget(m_paintingPage);

    // 6 = ДОПОЛНИТЕЛЬНО
    m_extraPage = new QWidget();
    m_extraPage->setStyleSheet("background: transparent;");
    QVBoxLayout *extraOuter = new QVBoxLayout(m_extraPage); extraOuter->setContentsMargins(0, 0, 0, 0);

    QScrollArea *extraScroll = new QScrollArea();
    extraScroll->setWidgetResizable(true);
    extraScroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollArea > QWidget > QWidget { background: transparent; } QScrollBar:vertical { background: rgba(20, 24, 42, 150); width: 6px; border-radius: 3px; } QScrollBar::handle:vertical { background: rgba(80, 100, 180, 100); border-radius: 3px; min-height: 30px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget *extraInner = new QWidget();
    extraInner->setStyleSheet("background: transparent;");
    QVBoxLayout *extraL = new QVBoxLayout(extraInner); extraL->setContentsMargins(30, 25, 30, 15); extraL->setSpacing(14);

    QLabel *extraTitle = new QLabel("Дополнительные инструменты");
    extraTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 16px; font-weight: bold; background: transparent; padding-bottom: 4px; }");
    extraL->addWidget(extraTitle);

    QGroupBox *bloodCard = new QGroupBox("Создание крови");
    bloodCard->setFlat(true);
    bloodCard->setStyleSheet(
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
    QVBoxLayout *bloodCardLay = new QVBoxLayout(bloodCard); bloodCardLay->setContentsMargins(4,4,4,4); bloodCardLay->setSpacing(8);
    QHBoxLayout *bloodRow = new QHBoxLayout(); bloodRow->setSpacing(6);
    m_bloodColor = new QLineEdit(); m_bloodColor->setPlaceholderText("#FF0000");
    m_bloodColor->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    m_bloodColor->setFixedHeight(42);
    bloodRow->addWidget(m_bloodColor);
    m_btnBloodColor = new QPushButton();
    m_btnBloodColor->setIcon(QIcon("icons/painting.png"));
    m_btnBloodColor->setIconSize(QSize(28, 28));
    m_btnBloodColor->setCursor(Qt::PointingHandCursor);
    m_btnBloodColor->setFixedSize(42, 42);
    m_btnBloodColor->setStyleSheet("QPushButton { background-color: rgba(35, 40, 70, 200); border: none; border-radius: 12px; } QPushButton:hover { background-color: rgba(45, 50, 90, 230); }");
    connect(m_btnBloodColor, &QPushButton::clicked, this, &KoTeKTools::onPickBloodColor);
    bloodRow->addWidget(m_btnBloodColor);
    bloodCardLay->addLayout(bloodRow);

    m_bloodSize = new QLineEdit(); m_bloodSize->setPlaceholderText("Размер крови");
    m_bloodSize->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    m_bloodSize->setFixedHeight(42);
    bloodCardLay->addWidget(m_bloodSize);
    extraL->addWidget(bloodCard);

    m_btnGenerateBlood = new QPushButton("Создать файл крови");
    m_btnGenerateBlood->setCursor(Qt::PointingHandCursor);
    m_btnGenerateBlood->setFixedHeight(46);
    m_btnGenerateBlood->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1e6040, stop:1 #143828);"
        "  color: #c0f0d0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #24784e, stop:1 #18482e);"
        "  color: #e0ffe0;"
        "}"
    );
    connect(m_btnGenerateBlood, &QPushButton::clicked, this, &KoTeKTools::onGenerateBlood);
    extraL->addWidget(m_btnGenerateBlood);

    extraL->addSpacing(15);

    QGroupBox *mapCard = new QGroupBox("Нарезание карты");
    mapCard->setFlat(true);
    mapCard->setStyleSheet(
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
    QVBoxLayout *mapCardLay = new QVBoxLayout(mapCard); mapCardLay->setContentsMargins(4,4,4,4); mapCardLay->setSpacing(8);
    QHBoxLayout *mapRow = new QHBoxLayout(); mapRow->setSpacing(6);
    m_mapImagePath = new QLineEdit(); m_mapImagePath->setPlaceholderText("Путь к изображению карты");
    m_mapImagePath->setReadOnly(true);
    m_mapImagePath->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    m_mapImagePath->setFixedHeight(42);
    mapRow->addWidget(m_mapImagePath);
    m_btnBrowseMapImage = new QPushButton();
    m_btnBrowseMapImage->setIcon(QIcon("icons/photo.png"));
    m_btnBrowseMapImage->setIconSize(QSize(24, 24));
    m_btnBrowseMapImage->setCursor(Qt::PointingHandCursor);
    m_btnBrowseMapImage->setFixedSize(42, 42);
    m_btnBrowseMapImage->setStyleSheet("QPushButton { background-color: rgba(35, 40, 70, 200); border: none; border-radius: 12px; } QPushButton:hover { background-color: rgba(45, 50, 90, 230); }");
    connect(m_btnBrowseMapImage, &QPushButton::clicked, this, &KoTeKTools::onBrowseMapImage);
    mapRow->addWidget(m_btnBrowseMapImage);
    mapCardLay->addLayout(mapRow);
    extraL->addWidget(mapCard);

    m_btnGenerateMapSlice = new QPushButton("Нарезать карту");
    m_btnGenerateMapSlice->setCursor(Qt::PointingHandCursor);
    m_btnGenerateMapSlice->setFixedHeight(46);
    m_btnGenerateMapSlice->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1e6040, stop:1 #143828);"
        "  color: #c0f0d0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #24784e, stop:1 #18482e);"
        "  color: #e0ffe0;"
        "}"
    );
    connect(m_btnGenerateMapSlice, &QPushButton::clicked, this, &KoTeKTools::onGenerateMapSlice);
    extraL->addWidget(m_btnGenerateMapSlice);

    m_extraNotifyLabel = new QLabel(); m_extraNotifyLabel->setAlignment(Qt::AlignCenter);
    m_extraNotifyLabel->setStyleSheet("QLabel { background-color: rgba(30, 96, 64, 200); color: #c0f0d0; border: none; border-radius: 12px; padding: 10px 14px; font-family: 'MiSans'; font-size: 12px; font-weight: bold; }");
    m_extraNotifyLabel->setVisible(false); m_extraNotifyLabel->setFixedHeight(44);
    QGraphicsOpacityEffect *oe5 = new QGraphicsOpacityEffect(m_extraNotifyLabel); oe5->setOpacity(0.0);
    m_extraNotifyLabel->setGraphicsEffect(oe5);
    extraL->addWidget(m_extraNotifyLabel);
    extraL->addStretch();

    extraScroll->setWidget(extraInner);
    extraOuter->addWidget(extraScroll);
    m_centerStack->addWidget(m_extraPage);

    // кнопка настроек
    m_settingsPage = new QWidget();
    m_settingsPage->setStyleSheet("background: transparent;");
    QVBoxLayout *settingsOuter = new QVBoxLayout(m_settingsPage); settingsOuter->setContentsMargins(0, 0, 0, 0);

    QScrollArea *settingsScroll = new QScrollArea();
    settingsScroll->setWidgetResizable(true);
    settingsScroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollBar:vertical { background: rgba(20, 24, 42, 150); width: 6px; border-radius: 3px; } QScrollBar::handle:vertical { background: rgba(80, 100, 180, 100); border-radius: 3px; min-height: 30px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget *settingsInner = new QWidget();
    settingsInner->setStyleSheet("background: transparent;");
    QVBoxLayout *settingsL = new QVBoxLayout(settingsInner); settingsL->setContentsMargins(30, 25, 30, 15); settingsL->setSpacing(12);

    QLabel *settingsTitle = new QLabel("Настройки");
    settingsTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 16px; font-weight: bold; background: transparent; padding-bottom: 4px; }");
    settingsL->addWidget(settingsTitle);

    QGroupBox *pathCard = new QGroupBox("Путь сохранения файлов");
    pathCard->setFlat(true);
    pathCard->setStyleSheet(
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
    QVBoxLayout *pathCardLay = new QVBoxLayout(pathCard); pathCardLay->setContentsMargins(4,4,4,4); pathCardLay->setSpacing(8);
    m_outputPath = new QLineEdit(); m_outputPath->setText(loadSavePath()); m_outputPath->setReadOnly(false);
    qputenv("KOTEK_OUTPUT_PATH", loadSavePath().toUtf8());
    m_outputPath->setFixedHeight(42);
    m_outputPath->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    pathCardLay->addWidget(m_outputPath);
    settingsL->addWidget(pathCard);

    QPushButton *browseFolderBtn = new QPushButton("Выбрать папку");
    browseFolderBtn->setCursor(Qt::PointingHandCursor); browseFolderBtn->setFixedHeight(40);
    browseFolderBtn->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1e6040, stop:1 #143828); color: #c0f0d0; border: none; border-radius: 12px; padding: 0px 20px; font-family: 'MiSans'; font-size: 13px; font-weight: bold; } QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #24784e, stop:1 #18482e); color: #e0ffe0; }");
    connect(browseFolderBtn, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку для сохранения", m_outputPath->text());
        if (!dir.isEmpty()) {
            m_outputPath->setText(dir);
            qputenv("KOTEK_OUTPUT_PATH", dir.toUtf8());
            savePathToFile(dir);
            m_settingsNotifyLabel->setText("Готово! Путь к файлам обновлён");
            m_settingsNotifyLabel->setVisible(true);
            QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(m_settingsNotifyLabel->graphicsEffect());
            if (e) e->setOpacity(1.0);
            QTimer::singleShot(3000, this, [this]() {
                QGraphicsOpacityEffect *ef = qobject_cast<QGraphicsOpacityEffect*>(m_settingsNotifyLabel->graphicsEffect());
                if (ef) ef->setOpacity(0.0);
                m_settingsNotifyLabel->setVisible(false);
            });
        }
    });
    settingsL->addWidget(browseFolderBtn);

    settingsL->addSpacing(15);

    // кнопка скрыть инфо
    QPushButton *toggleInfoBtn = new QPushButton("Скрыть информацию");
    toggleInfoBtn->setCursor(Qt::PointingHandCursor);
    toggleInfoBtn->setFixedHeight(42);
    toggleInfoBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(35, 40, 70, 160);"
        "  color: #b0b8e0;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 0px 16px;"
        "  font-family: 'MiSans';"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(45, 50, 90, 200);"
        "  color: #e0e8ff;"
        "}"
    );
    settingsL->addWidget(toggleInfoBtn);

    QLabel *infoText = new QLabel(
        "<div style='color: #c0f0d0; font-family: MiSans; font-size: 12px; line-height: 1.5; text-align: center;'>"
        "<b style='color: #e0ffe0; font-size: 14px;'>KoTeK Tools</b><br><br>"
        "Набор инструментов для быстрого создания модов и сборок.<br>"
        "Всё, что нужно для модов и сборок, в одном месте.<br><br>"
        "<b style='color: #e0ffe0;'>Что внутри:</b><br>"
        "• <b>Копирование</b> — билборды, логотипы, листва<br>"
        "• <b>Конвертация</b> — BPC, IFP/ANI, MOD/DFF, TXD, BTX ↔ PNG<br>"
        "• <b>Атмосфера</b> — небо и окружение<br>"
        "• <b>Покраска</b> — кнопки, элементы худа, изображения<br>"
        "• <b>Дополнительно</b> — генератор крови, нарезка карт<br>"
        "• <b>Поддержка ZIP</b> — IFP/ANI, MOD/DFF, BTX↔PNG<br><br>"
        "<a href='https://t.me/kotek_mods' style='color: #80b0ff; text-decoration: none;'>t.me/kotek_mods</a>"
        "&nbsp;&nbsp;|&nbsp;&nbsp;"
        "<a href='https://t.me/kotek_ya' style='color: #80b0ff; text-decoration: none;'>t.me/kotek_ya</a>"
        "</div>"
    );
    infoText->setWordWrap(true);
    infoText->setOpenExternalLinks(true);
    infoText->setAlignment(Qt::AlignCenter);
    infoText->setStyleSheet("QLabel { background: transparent; }");
    infoText->setVisible(true);
    settingsL->addWidget(infoText);

    // анимация
    QPropertyAnimation *textAnim = new QPropertyAnimation(infoText, "maximumHeight");
    textAnim->setDuration(250);
    textAnim->setEasingCurve(QEasingCurve::InOutCubic);

    connect(toggleInfoBtn, &QPushButton::clicked, this, [infoText, textAnim, toggleInfoBtn]() {
        if (infoText->isVisible()) {
            textAnim->setEndValue(0);
            textAnim->start();
            QTimer::singleShot(250, [infoText, toggleInfoBtn]() {
                infoText->setVisible(false);
                toggleInfoBtn->setText("Раскрыть информацию");
            });
        } else {
            infoText->setVisible(true);
            infoText->setMaximumHeight(0);
            textAnim->setEndValue(infoText->sizeHint().height());
            textAnim->start();
            toggleInfoBtn->setText("Скрыть информацию");
        }
    });

    m_settingsNotifyLabel = new QLabel(); m_settingsNotifyLabel->setAlignment(Qt::AlignCenter);
    m_settingsNotifyLabel->setStyleSheet("QLabel { background-color: rgba(30, 96, 64, 200); color: #c0f0d0; border: none; border-radius: 12px; padding: 10px 14px; font-family: 'MiSans'; font-size: 12px; font-weight: bold; }");
    m_settingsNotifyLabel->setVisible(false); m_settingsNotifyLabel->setFixedHeight(40);
    QGraphicsOpacityEffect *oe6 = new QGraphicsOpacityEffect(m_settingsNotifyLabel); oe6->setOpacity(0.0);
    m_settingsNotifyLabel->setGraphicsEffect(oe6);
    settingsL->addWidget(m_settingsNotifyLabel);
    settingsL->addStretch();

    settingsScroll->setWidget(settingsInner);
    settingsOuter->addWidget(settingsScroll);
    m_centerStack->addWidget(m_settingsPage);

    m_centerStack->setCurrentIndex(0);
    mainLayout->addWidget(m_sidePanel);
    mainLayout->addWidget(m_centerStack);

    // анимация панели
    m_panelAnim = new QPropertyAnimation(m_sidePanel, "maximumWidth");
    m_panelAnim->setDuration(280); m_panelAnim->setEasingCurve(QEasingCurve::InOutCubic);

    m_btnMinimize->move(width() - 88, 6);
    m_btnClose->move(width() - 44, 6);
    m_btnMinimize->raise();
    m_btnClose->raise();

    // 8 = 3Д ПРОСМОТР
    m_3DViewPage = new QWidget();
    m_3DViewPage->setStyleSheet("background: transparent;");

    QVBoxLayout *viewerOuter = new QVBoxLayout(m_3DViewPage);
    viewerOuter->setContentsMargins(0, 0, 0, 0);

    QScrollArea *viewerScroll = new QScrollArea();
    viewerScroll->setWidgetResizable(true);
    viewerScroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollBar:vertical { background: rgba(20, 24, 42, 150); width: 6px; border-radius: 3px; } QScrollBar::handle:vertical { background: rgba(80, 100, 180, 100); border-radius: 3px; min-height: 30px; }");

    QWidget *viewerInner = new QWidget();
    viewerInner->setStyleSheet("background: transparent;");

    QVBoxLayout *viewerL = new QVBoxLayout(viewerInner);
    viewerL->setContentsMargins(30, 25, 30, 15);
    viewerL->setSpacing(12);

    QLabel *viewerTitle = new QLabel("3D просмотр");
    viewerTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 16px; font-weight: bold; background: transparent; padding-bottom: 4px; }");
    viewerL->addWidget(viewerTitle);

    QGroupBox *dffCard = new QGroupBox("3D модель (DFF)");
    dffCard->setFlat(true);
    dffCard->setStyleSheet(
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

    QVBoxLayout *dffCardLay = new QVBoxLayout(dffCard);
    dffCardLay->setContentsMargins(4, 4, 4, 4);
    dffCardLay->setSpacing(8);

    QHBoxLayout *dffPathRow = new QHBoxLayout();
    dffPathRow->setSpacing(6);
    m_dffPath = new QLineEdit();
    m_dffPath->setPlaceholderText("Путь к файлу .dff");
    m_dffPath->setReadOnly(true);
    m_dffPath->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    m_dffPath->setFixedHeight(42);
    dffPathRow->addWidget(m_dffPath);

    m_btnBrowseDFF = new QPushButton();
    m_btnBrowseDFF->setIcon(QIcon("icons/file.png"));
    m_btnBrowseDFF->setIconSize(QSize(24, 24));
    m_btnBrowseDFF->setCursor(Qt::PointingHandCursor);
    m_btnBrowseDFF->setFixedSize(42, 42);
    m_btnBrowseDFF->setStyleSheet("QPushButton { background-color: rgba(35, 40, 70, 200); border: none; border-radius: 12px; } QPushButton:hover { background-color: rgba(45, 50, 90, 230); }");
    connect(m_btnBrowseDFF, &QPushButton::clicked, this, &KoTeKTools::onBrowseDFF);
    dffPathRow->addWidget(m_btnBrowseDFF);
    dffCardLay->addLayout(dffPathRow);

    QHBoxLayout *texPathRow = new QHBoxLayout();
    texPathRow->setSpacing(6);
    m_dffTexturePath = new QLineEdit();
    m_dffTexturePath->setPlaceholderText("Путь к текстуре (опционально)");
    m_dffTexturePath->setReadOnly(true);
    m_dffTexturePath->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    m_dffTexturePath->setFixedHeight(42);
    texPathRow->addWidget(m_dffTexturePath);

    m_btnBrowseDFFTexture = new QPushButton();
    m_btnBrowseDFFTexture->setIcon(QIcon("icons/photo.png"));
    m_btnBrowseDFFTexture->setIconSize(QSize(24, 24));
    m_btnBrowseDFFTexture->setCursor(Qt::PointingHandCursor);
    m_btnBrowseDFFTexture->setFixedSize(42, 42);
    m_btnBrowseDFFTexture->setStyleSheet("QPushButton { background-color: rgba(35, 40, 70, 200); border: none; border-radius: 12px; } QPushButton:hover { background-color: rgba(45, 50, 90, 230); }");
    connect(m_btnBrowseDFFTexture, &QPushButton::clicked, this, &KoTeKTools::onBrowseDFFTexture);
    texPathRow->addWidget(m_btnBrowseDFFTexture);

    m_btnApplyDFFTexture = new QPushButton("Применить");
    m_btnApplyDFFTexture->setCursor(Qt::PointingHandCursor);
    m_btnApplyDFFTexture->setFixedSize(80, 42);
    m_btnApplyDFFTexture->setStyleSheet("QPushButton { background-color: rgba(30, 96, 64, 200); color: #c0f0d0; border: none; border-radius: 12px; font-family: 'MiSans'; font-size: 12px; font-weight: bold; } QPushButton:hover { background-color: rgba(36, 120, 78, 200); }");
    connect(m_btnApplyDFFTexture, &QPushButton::clicked, this, &KoTeKTools::onApplyDFFTexture);
    texPathRow->addWidget(m_btnApplyDFFTexture);
    dffCardLay->addLayout(texPathRow);

    m_3dViewer = new DFFViewerWidget(this);
    m_3dViewer->setMinimumHeight(350);
    m_3dViewer->setStyleSheet("border-radius: 12px;");
    dffCardLay->addWidget(m_3dViewer);

    QHBoxLayout *settingsRow = new QHBoxLayout();
    settingsRow->setSpacing(12);

    m_gridCheckBox = new QCheckBox("Show Grid");
    m_gridCheckBox->setChecked(true);
    m_gridCheckBox->setStyleSheet("QCheckBox { color: #9098c0; font-family: 'MiSans'; font-size: 12px; spacing: 8px; } QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; border: 1px solid #606880; background: rgba(20, 24, 42, 200); } QCheckBox::indicator:checked { background: #1e6040; border-color: #3ea060; }");
    connect(m_gridCheckBox, &QCheckBox::toggled, this, &KoTeKTools::onGridToggled);
    settingsRow->addWidget(m_gridCheckBox);

    settingsRow->addStretch();
    dffCardLay->addLayout(settingsRow);

    m_3dInfoLabel = new QLabel();
    m_3dInfoLabel->setAlignment(Qt::AlignCenter);
    m_3dInfoLabel->setStyleSheet("QLabel { color: #9098c0; font-family: 'MiSans'; font-size: 11px; background: transparent; padding: 4px; }");
    dffCardLay->addWidget(m_3dInfoLabel);

    viewerL->addWidget(dffCard);
    viewerL->addStretch();

    viewerScroll->setWidget(viewerInner);
    viewerOuter->addWidget(viewerScroll);
    m_centerStack->addWidget(m_3DViewPage);

    // 7 = ТЕКСТУИРОВАНИЕ
    m_texturingPage = new QWidget();
    m_texturingPage->setStyleSheet("background: transparent;");

    QVBoxLayout *texturingOuter = new QVBoxLayout(m_texturingPage);
    texturingOuter->setContentsMargins(0, 0, 0, 0);

    QScrollArea *texturingScroll = new QScrollArea();
    texturingScroll->setWidgetResizable(true);
    texturingScroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollBar:vertical { background: rgba(20, 24, 42, 150); width: 6px; border-radius: 3px; } QScrollBar::handle:vertical { background: rgba(80, 100, 180, 100); border-radius: 3px; min-height: 30px; }");

    QWidget *texturingInner = new QWidget();
    texturingInner->setStyleSheet("background: transparent;");

    QVBoxLayout *texturingL = new QVBoxLayout(texturingInner);
    texturingL->setContentsMargins(30, 25, 30, 15);
    texturingL->setSpacing(12);

    QLabel *texturingTitle = new QLabel("Текстурирование");
    texturingTitle->setStyleSheet("QLabel { color: #d0d6f0; font-family: 'MiSans'; font-size: 16px; font-weight: bold; background: transparent; padding-bottom: 4px; }");
    texturingL->addWidget(texturingTitle);

    QGroupBox *ytdCard = new QGroupBox("Работа с текстурами (YTD)");
    ytdCard->setFlat(true);
    ytdCard->setStyleSheet(
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
    QVBoxLayout *ytdCardLay = new QVBoxLayout(ytdCard);
    ytdCardLay->setContentsMargins(4, 4, 4, 4);
    ytdCardLay->setSpacing(8);

    QHBoxLayout *ytdPathRow = new QHBoxLayout();
    ytdPathRow->setSpacing(6);
    m_ytdPath = new QLineEdit();
    m_ytdPath->setPlaceholderText("Путь к файлу .ytd");
    m_ytdPath->setReadOnly(true);
    m_ytdPath->setStyleSheet("QLineEdit { background-color: rgba(20, 24, 42, 200); color: #9098c0; border: none; border-radius: 12px; padding: 0px 14px; font-family: 'MiSans'; font-size: 12px; }");
    m_ytdPath->setFixedHeight(42);
    ytdPathRow->addWidget(m_ytdPath);

    m_btnBrowseYtd = new QPushButton();
    m_btnBrowseYtd->setIcon(QIcon("icons/file.png"));
    m_btnBrowseYtd->setIconSize(QSize(24, 24));
    m_btnBrowseYtd->setCursor(Qt::PointingHandCursor);
    m_btnBrowseYtd->setFixedSize(42, 42);
    m_btnBrowseYtd->setStyleSheet("QPushButton { background-color: rgba(35, 40, 70, 200); border: none; border-radius: 12px; } QPushButton:hover { background-color: rgba(45, 50, 90, 230); }");
    connect(m_btnBrowseYtd, &QPushButton::clicked, this, &KoTeKTools::onBrowseYtd);
    ytdPathRow->addWidget(m_btnBrowseYtd);
    ytdCardLay->addLayout(ytdPathRow);

    m_ytdTextureList = new QListWidget();
    m_ytdTextureList->setMaximumHeight(150);
    m_ytdTextureList->setStyleSheet(
        "QListWidget {"
        "  background-color: rgba(20, 24, 42, 200);"
        "  color: #9098c0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 8px;"
        "  font-family: 'MiSans';"
        "  font-size: 12px;"
        "  outline: none;"
        "}"
        "QListWidget::item {"
        "  padding: 8px 12px;"
        "  border-radius: 8px;"
        "  margin: 2px 0px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: rgba(80, 100, 180, 100);"
        "  color: #d0d8ff;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: rgba(35, 40, 70, 200);"
        "}"
    );
    connect(m_ytdTextureList, &QListWidget::currentRowChanged, this, &KoTeKTools::onYtdListSelectionChanged);
    ytdCardLay->addWidget(m_ytdTextureList);

    m_ytdPreviewLabel = new QLabel("Выберите текстуру для просмотра");
    m_ytdPreviewLabel->setAlignment(Qt::AlignCenter);
    m_ytdPreviewLabel->setMinimumHeight(250);
    m_ytdPreviewLabel->setMaximumHeight(350);
    m_ytdPreviewLabel->setStyleSheet(
        "QLabel {"
        "  background-color: rgba(20, 24, 42, 200);"
        "  color: #606880;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "}"
    );
    ytdCardLay->addWidget(m_ytdPreviewLabel);

    m_ytdInfoLabel = new QLabel();
    m_ytdInfoLabel->setAlignment(Qt::AlignCenter);
    m_ytdInfoLabel->setStyleSheet("QLabel { color: #9098c0; font-family: 'MiSans'; font-size: 12px; background: transparent; }");
    ytdCardLay->addWidget(m_ytdInfoLabel);

    QHBoxLayout *exportButtonsLayout = new QHBoxLayout();
    exportButtonsLayout->setSpacing(12);

    m_btnExportYtd = new QPushButton("Экспорт в PNG");
    m_btnExportYtd->setCursor(Qt::PointingHandCursor);
    m_btnExportYtd->setFixedHeight(42);
    m_btnExportYtd->setEnabled(false);
    m_btnExportYtd->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1e6040, stop:1 #143828);"
        "  color: #c0f0d0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #24784e, stop:1 #18482e);"
        "  color: #e0ffe0;"
        "}"
        "QPushButton:disabled {"
        "  background-color: rgba(35, 40, 70, 100);"
        "  color: #505870;"
        "}"
    );
    connect(m_btnExportYtd, &QPushButton::clicked, this, &KoTeKTools::onExportYtdTexture);
    exportButtonsLayout->addWidget(m_btnExportYtd);

    m_btnExportAllYtd = new QPushButton("Экспорт всех в ZIP");
    m_btnExportAllYtd->setCursor(Qt::PointingHandCursor);
    m_btnExportAllYtd->setFixedHeight(42);
    m_btnExportAllYtd->setEnabled(false);
    m_btnExportAllYtd->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #2a4a6e, stop:1 #1a3050);"
        "  color: #c0d0f0;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 0px 20px;"
        "  font-family: 'MiSans';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #3a6a8e, stop:1 #2a4070);"
        "  color: #e0f0ff;"
        "}"
        "QPushButton:disabled {"
        "  background-color: rgba(35, 40, 70, 100);"
        "  color: #505870;"
        "}"
    );
    connect(m_btnExportAllYtd, &QPushButton::clicked, this, &KoTeKTools::onExportAllYtdToZip);
    exportButtonsLayout->addWidget(m_btnExportAllYtd);

    exportButtonsLayout->addStretch();
    ytdCardLay->addLayout(exportButtonsLayout);

    texturingL->addWidget(ytdCard);
    texturingL->addStretch();

    texturingScroll->setWidget(texturingInner);
    texturingOuter->addWidget(texturingScroll);
    m_centerStack->addWidget(m_texturingPage);
}