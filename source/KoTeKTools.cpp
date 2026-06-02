#include "btx_converter.h"
#include "KoTeKTools.h"
#include "listva_names.h"
#include "zlib.h"
#include "dff.h"
#include "bpc.h"
#include "atmosphere.h"
#include "painting.h"
#include "map.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include "txd.h"
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QProcess>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QScrollArea>
#include <QSpacerItem>
#include <QColorDialog>
#include <QRegularExpression>
#include <QFontDatabase>
#include <QMouseEvent>
#include <QSettings>

KoTeKTools::KoTeKTools(QWidget *parent) : QWidget(parent), m_panelOpen(false), m_panelWidth(300), m_serversLoaded(false), m_busy(false), m_dragging(false), m_threadPool(new QThreadPool(this))
{
    // иконки
    m_iconClose = QIcon("icons/close.png");
    m_iconWrap = QIcon("icons/wrap.png");

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    // потоки
    m_threadPool->setMaxThreadCount(QThread::idealThreadCount());

    initTimers();
    initWatcher();
    setupUi();
    loadServersList();
}

KoTeKTools::~KoTeKTools()
{
    
    m_threadPool->clear();
    m_threadPool->waitForDone(3000);
    
}

QString KoTeKTools::getAppPath() { return QApplication::applicationDirPath(); }


  // загрузка пути сохранения
QString KoTeKTools::loadSavePath() {
    QSettings settings("KoTeK", "KoTeKTools");
    return settings.value("outputPath", QApplication::applicationDirPath()).toString();
}

 // сохранение пути
void KoTeKTools::savePathToFile(const QString &path) {
    QSettings settings("KoTeK", "KoTeKTools");
    settings.setValue("outputPath", path);
}

void KoTeKTools::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_btnMinimize) m_btnMinimize->move(width() - 88, 6);
    if (m_btnClose) m_btnClose->move(width() - 44, 6);
}

  // сворачивание окна
void KoTeKTools::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange && isMinimized()) {
        emit minimized();
    }
    QWidget::changeEvent(event);
}


 // таймер скрытия уведомлений
void KoTeKTools::initTimers()
{
    m_notificationTimer = new QTimer(this);
    m_notificationTimer->setSingleShot(true);
    connect(m_notificationTimer, &QTimer::timeout, this, &KoTeKTools::hideNotification);

    m_atmoTimer = new QTimer(this);
    m_atmoTimer->setSingleShot(true);
    connect(m_atmoTimer, &QTimer::timeout, this, [this]() {
        if (m_atmoNotifyLabel) {
            QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(m_atmoNotifyLabel->graphicsEffect());
            if (e) e->setOpacity(0.0);
            m_atmoNotifyLabel->setVisible(false);
        }
    });

    m_paintTimer = new QTimer(this);
    m_paintTimer->setSingleShot(true);
    connect(m_paintTimer, &QTimer::timeout, this, [this]() {
        if (m_paintNotifyLabel) {
            QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(m_paintNotifyLabel->graphicsEffect());
            if (e) e->setOpacity(0.0);
            m_paintNotifyLabel->setVisible(false);
        }
    });

    m_extraTimer = new QTimer(this);
    m_extraTimer->setSingleShot(true);
    connect(m_extraTimer, &QTimer::timeout, this, [this]() {
        if (m_extraNotifyLabel) {
            QGraphicsOpacityEffect *e = qobject_cast<QGraphicsOpacityEffect*>(m_extraNotifyLabel->graphicsEffect());
            if (e) e->setOpacity(0.0);
            m_extraNotifyLabel->setVisible(false);
        }
    });
}

void KoTeKTools::initWatcher()
{
  
    m_asyncWatcher = new QFutureWatcher<void>(this);
    connect(m_asyncWatcher, &QFutureWatcher<void>::finished, this, [this]() {
        setButtonsEnabled(true);
        QMutexLocker locker(&m_busyMutex);
        m_busy = false;
    });
}

void KoTeKTools::showSettingsPage() { m_centerStack->setCurrentIndex(7); }
void KoTeKTools::show3DViewPage() { m_centerStack->setCurrentIndex(8); }
void KoTeKTools::showTexturingPage() { m_centerStack->setCurrentIndex(9); }

void KoTeKTools::runAsync(std::function<void()> task)
{
    
    QMutexLocker locker(&m_busyMutex);
    if (m_busy) return;
    m_busy = true;
    locker.unlock();
    
    
    setButtonsEnabled(false);
    
    auto asyncTask = new AsyncTask([this, task = std::move(task)]() {
        task();
        QMetaObject::invokeMethod(this, [this]() {
            setButtonsEnabled(true);
            QMutexLocker locker(&m_busyMutex);
            m_busy = false;
        }, Qt::QueuedConnection);
    });
    
    m_threadPool->start(asyncTask);
}

void KoTeKTools::setButtonsEnabled(bool enabled)
{
    // кнопки хз
    m_centerBtn->setEnabled(enabled);
    if (m_btnBrowseBillboard) m_btnBrowseBillboard->setEnabled(enabled);
    if (m_btnCopyBillboard) m_btnCopyBillboard->setEnabled(enabled);
    if (m_btnBrowseLogo) m_btnBrowseLogo->setEnabled(enabled);
    if (m_btnCopyLogo) m_btnCopyLogo->setEnabled(enabled);
    if (m_btnBrowseListva) m_btnBrowseListva->setEnabled(enabled);
    if (m_btnCopyListva) m_btnCopyListva->setEnabled(enabled);
    if (m_btnBrowseBpc) m_btnBrowseBpc->setEnabled(enabled);
    if (m_btnConvertBpc) m_btnConvertBpc->setEnabled(enabled);
    if (m_btnBrowseBpcMeta) m_btnBrowseBpcMeta->setEnabled(enabled);
    if (m_btnConvertBpcMeta) m_btnConvertBpcMeta->setEnabled(enabled);
    if (m_btnBrowseIfpAni) m_btnBrowseIfpAni->setEnabled(enabled);
    if (m_btnConvertIfpAni) m_btnConvertIfpAni->setEnabled(enabled);
    if (m_btnBrowseMod) m_btnBrowseMod->setEnabled(enabled);
    if (m_btnConvertMod) m_btnConvertMod->setEnabled(enabled);
    if (m_btnBrowseTxd) m_btnBrowseTxd->setEnabled(enabled);
    if (m_btnConvertTxd) m_btnConvertTxd->setEnabled(enabled);
    if (m_btnBrowseBtx) m_btnBrowseBtx->setEnabled(enabled);
    if (m_btnConvertBtx) m_btnConvertBtx->setEnabled(enabled);
    if (m_btnMinimize) m_btnMinimize->setEnabled(enabled);
    if (m_btnClose) m_btnClose->setEnabled(enabled);
    if (m_btnBrowseYtd) m_btnBrowseYtd->setEnabled(enabled);
    if (m_btnExportYtd) m_btnExportYtd->setEnabled(enabled);
    if (m_btnBrowseDFF) m_btnBrowseDFF->setEnabled(enabled);
    if (m_btnBrowseDFFTexture) m_btnBrowseDFFTexture->setEnabled(enabled);
    if (m_btnApplyDFFTexture) m_btnApplyDFFTexture->setEnabled(enabled);
}

void KoTeKTools::loadServersList()
{
    // загрузка списка серверов
    QString jsonPath = getAppPath() + "/servers.json";
    if (QFile::exists(jsonPath)) {
        QFileInfo fi(jsonPath);
        if (fi.lastModified().secsTo(QDateTime::currentDateTime()) < 86400)
            if (parseJsonFile(jsonPath)) return;
    }

    QProcess *ps = new QProcess(this);
    connect(ps, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, ps, jsonPath](int code, QProcess::ExitStatus) {
        if (code == 0) parseJsonFile(jsonPath);
        ps->deleteLater();
    });
    ps->start("powershell", QStringList() << "-Command" << QString("Invoke-WebRequest -Uri 'https://api.blackrussia.online/servers.json' -OutFile '%1'").arg(jsonPath));
}

bool KoTeKTools::parseJsonFile(const QString &path)
{
    // парсинг json
    QFile f(path); if (!f.open(QIODevice::ReadOnly)) return false;
    QByteArray d = f.readAll(); f.close();

    QJsonDocument doc = QJsonDocument::fromJson(d);
    if (!doc.isArray()) return false;
    QJsonArray arr = doc.array(); if (arr.isEmpty()) return false;

    m_serverFirstNames.clear();
    m_serverFirstNames.reserve(arr.size());
    for (const QJsonValue &v : arr) m_serverFirstNames.append(v.toObject()["firstname"].toString().toLower());
    m_serversLoaded = true;
    return true;
}

void KoTeKTools::openPanel()      { m_centerStack->setCurrentIndex(1); m_panelAnim->stop(); m_panelAnim->setStartValue(0); m_panelAnim->setEndValue(m_panelWidth); m_panelAnim->start(); m_panelOpen = true; }
void KoTeKTools::closePanel()     { m_panelAnim->stop(); m_panelAnim->setStartValue(m_panelWidth); m_panelAnim->setEndValue(0); m_panelAnim->start(); m_panelOpen = false; m_centerStack->setCurrentIndex(0); clearAllFields(); hideNotification(); }
void KoTeKTools::showCopyPage()   { m_centerStack->setCurrentIndex(2); }
void KoTeKTools::showConvertPage(){ m_centerStack->setCurrentIndex(3); }
void KoTeKTools::showAtmospherePage(){ m_centerStack->setCurrentIndex(4); }
void KoTeKTools::showPaintingPage(){ m_centerStack->setCurrentIndex(5); }
void KoTeKTools::showExtraPage()  { m_centerStack->setCurrentIndex(6); }
void KoTeKTools::backToEmpty()    { m_centerStack->setCurrentIndex(1); clearAllFields(); hideNotification(); }

void KoTeKTools::clearAllFields()
{
    
    m_billboardPath->clear(); m_logoPath->clear(); m_listvaPath->clear();
    m_bpcPath->clear(); m_bpcMetaPath->clear(); m_ifpAniPath->clear();
    m_modPath->clear(); m_txdPath->clear(); m_btxPath->clear();
    m_skyTopColor->clear(); m_skyBottomColor->clear(); m_sunColor->clear();
    m_cloudsColor->clear(); m_environmentColor->clear();
    m_hudColor->clear(); m_buttonsColor->clear();
    m_paintImageColor->clear(); m_paintImagePath->clear();
    m_bloodColor->clear(); m_bloodSize->clear(); m_mapImagePath->clear();
    
}