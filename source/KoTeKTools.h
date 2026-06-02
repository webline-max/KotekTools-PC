#ifndef KOTEKTOOLS_H
#define KOTEKTOOLS_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QFrame>
#include <QStackedWidget>
#include <QProcess>
#include <QFutureWatcher>
#include <QThreadPool>
#include <QRunnable>
#include <QColorDialog>
#include <QRegularExpression>
#include <QIcon>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QApplication>
#include <QDir>
#include <QEvent>
#include <QMutex>
#include <QMutexLocker>
#include <QListWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QFile>
#include <QDataStream>
#include <QVector>
#include <QVector3D>
#include <QVector2D>
#include <QMap>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Async
class AsyncTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit AsyncTask(std::function<void()> func) : m_func(std::move(func)) {}
    void run() override { if (m_func) m_func(); }
private:
    std::function<void()> m_func;
};

// информвция о текстуре
struct TextureInfo {
    QString name;
    int width;
    int height;
    int mipCount;
    int format;
    QByteArray data;
    bool isValid = false;
};

// DFF структуры
namespace DFF {

struct Vector3 {
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct Vector2 {
    float u, v;
    Vector2() : u(0), v(0) {}
    Vector2(float u_, float v_) : u(u_), v(v_) {}
};

struct RGBA {
    unsigned char r, g, b, a;
    RGBA() : r(255), g(255), b(255), a(255) {}
    RGBA(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_)
        : r(r_), g(g_), b(b_), a(a_) {}
};

struct Sphere {
    Vector3 center;
    float radius;
    Sphere() : center(0,0,0), radius(1.0f) {}
};

struct Material {
    RGBA color;
    float ambient, specular, diffuse;
    QString textureName;
    QString alphaName;
    int filterMode;
    int uvAddressing;
    bool isTextured;
    int index;
    
    Material() : color(255,255,255,255), ambient(0.5f), specular(0.5f),
                 diffuse(0.5f), filterMode(0), uvAddressing(0), isTextured(false), index(0) {}
};

struct Triangle {
    int v1, v2, v3;
    int material;
    Triangle() : v1(0), v2(0), v3(0), material(0) {}
    Triangle(int v1_, int v2_, int v3_, int matIdx)
        : v1(v1_), v2(v2_), v3(v3_), material(matIdx) {}
};

struct MaterialSplit {
    quint32 materialIndex;
    QVector<quint32> indices;
};

struct Geometry {
    QVector<Vector3> vertices;
    QVector<Vector3> normals;
    QVector<Vector2> texCoords;
    QVector<RGBA> prelitColors;
    QVector<Triangle> triangles;
    QVector<Material> materials;
    QVector<MaterialSplit> materialSplits;
    Sphere boundingSphere;
    int flags;
    bool hasNormals;
    bool hasTexCoords;
    bool hasPrelitColors;
    
    Geometry() : flags(0), hasNormals(false), hasTexCoords(false), hasPrelitColors(false) {}
};

struct Frame {
    QMatrix4x4 matrix;
    QString name;
    int parentId;
    Frame() : parentId(-1) {}
};

struct Atomic {
    int frameIndex;
    int geometryIndex;
    Atomic() : frameIndex(0), geometryIndex(0) {}
};

struct DFFFile {
    QVector<Geometry> geometries;
    QVector<Frame> frames;
    QVector<Atomic> atomics;
    int version;
    QString fileName;
    
    void clear() {
        geometries.clear();
        frames.clear();
        atomics.clear();
        version = 0;
        fileName.clear();
    }
};

}

// DFFViewerWidged
class DFFViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    struct Vertex3D { float x, y, z; };
    struct UV2D { float u, v; };
    struct ColorRGBA { unsigned char r, g, b, a; };
    
    explicit DFFViewerWidget(QWidget *parent = nullptr);
    ~DFFViewerWidget();
    
    void loadDFF(const DFF::DFFFile& dff);
    void loadTexture(const QString& path);
    void setShowGrid(bool show);
    void setVertexColorMode(int mode);
    void clearModel();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void drawGrid();
    void drawModel();
    
    float m_minDistance;
    float m_maxDistance;
    Vertex3D m_modelCenter;
    float m_modelRadius;
    float m_rotationX;
    float m_rotationY;
    float m_distance;
    QPoint m_lastMousePos;
    bool m_hasModel;
    bool m_hasTexture;
    bool m_showGrid;
    int m_vertexColorMode;
    QOpenGLTexture* m_texture;
    QString m_currentTexturePath;
    
    QVector<Vertex3D> m_vertices;
    QVector<Vertex3D> m_normals;
    QVector<UV2D> m_texCoords;
    QVector<ColorRGBA> m_colors;
    QVector<unsigned int> m_indices;
    QStringList m_texturePaths;
    int m_currentTextureIndex;
    
    DFF::DFFFile m_currentDFF;
    
    friend void computeNormalsForTriangles(
        const QVector<DFFViewerWidget::Vertex3D>& vertices,
        const QVector<unsigned int>& indices,
        QVector<DFFViewerWidget::Vertex3D>& outNormals);
};

// DFFLoader
class DFFLoader {
public:
    DFFLoader();
    bool loadFile(const QString& filename, DFF::DFFFile& outFile);
    QString getLastError() const { return m_lastError; }
    
private:
    struct Chunk {
        quint32 type;
        quint32 size;
        quint32 version;
    };
    
    QString m_lastError;
    QFile m_file;
    QDataStream m_stream;
    
    bool readChunk(Chunk& chunk);
    bool skipChunk(const Chunk& chunk);
    
    bool parseClump(DFF::DFFFile& file, const Chunk& clumpChunk);
    bool parseFrameList(DFF::DFFFile& file, const Chunk& chunk);
    bool parseGeometryList(DFF::DFFFile& file, const Chunk& chunk);
    bool parseGeometry(DFF::DFFFile& file, const Chunk& chunk, DFF::Geometry& geom);
    bool parseMaterialList(DFF::DFFFile& file, const Chunk& chunk, DFF::Geometry& geom);
    bool parseMaterial(DFF::Material& mat, quint32 version);
    bool parseTexture(DFF::Material& mat);
    bool parseAtomic(DFF::DFFFile& file, const Chunk& chunk);
    bool parseExtension(DFF::DFFFile& file, const Chunk& chunk, DFF::Geometry& geom);
    
    static const quint32 CHUNK_STRUCT = 1;
    static const quint32 CHUNK_STRING = 2;
    static const quint32 CHUNK_EXTENSION = 3;
    static const quint32 CHUNK_TEXTURE = 6;
    static const quint32 CHUNK_MATERIAL = 7;
    static const quint32 CHUNK_MATERIALLIST = 8;
    static const quint32 CHUNK_FRAMELIST = 14;
    static const quint32 CHUNK_GEOMETRY = 15;
    static const quint32 CHUNK_CLUMP = 16;
    static const quint32 CHUNK_ATOMIC = 20;
    static const quint32 CHUNK_GEOMETRYLIST = 26;
    static const quint32 CHUNK_RIGHTTORENDER = 0x0000001F;
    static const quint32 CHUNK_MATERIALSPLIT = 0x00000020;
    
    static const quint32 rpGEOMETRYTEXTURED = 0x00000004;
    static const quint32 rpGEOMETRYPRELIT = 0x00000008;
    static const quint32 rpGEOMETRYNORMALS = 0x00000010;
    static const quint32 rpGEOMETRYTEXTURED2 = 0x00000080;
};

// Kotek Tools
class KoTeKTools : public QWidget
{
    Q_OBJECT

public:
    KoTeKTools(QWidget *parent = nullptr);
    ~KoTeKTools();

    static QString outputPath() {
        QString p = QString::fromUtf8(qgetenv("KOTEK_OUTPUT_PATH"));
        return p.isEmpty() ? QApplication::applicationDirPath() : p;
    }

signals:
    void minimized();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void openPanel();
    void closePanel();
    void showCopyPage();
    void showConvertPage();
    void showAtmospherePage();
    void showPaintingPage();
    void showExtraPage();
    void showSettingsPage();
    void onExportAllYtdToZip();
    void show3DViewPage();
    void showTexturingPage();
    void onBrowseBillboard();
    void onCopyBillboard();
    void onBrowseLogo();
    void onCopyLogos();
    void onBrowseListva();
    void onCopyListva();
    void onBrowseBpc();
    void onConvertBpcToZip();
    void onBrowseIfpAni();
    void onConvertIfpAni();
    void onBrowseMod();
    void onConvertMod();
    void onBrowseBpcMeta();
    void onConvertBpcToMeta();
    void onBrowseTxd();
    void onConvertTxd();
    void onBrowseBtx();
    void onConvertBtx();
    void onBrowseYtd();
    void onExportYtdTexture();
    void onYtdListSelectionChanged(int index);
    void onBrowseDFF();
    void onBrowseDFFTexture();
    void onApplyDFFTexture();
    void onGridToggled(bool checked);
    void onVertexColorModeChanged(int index);
    void onPickSkyTopColor();
    void onPickSkyBottomColor();
    void onPickSunColor();
    void onPickCloudsColor();
    void onPickEnvironmentColor();
    void onGenerateTimecyc();
    void onGenerateEnvironment();
    void onPickHudColor();
    void onPickButtonsColor();
    void onPickPaintImageColor();
    void onGenerateHud();
    void onGenerateButtons();
    void onBrowsePaintImage();
    void onGeneratePaintImage();
    void onPickBloodColor();
    void onGenerateBlood();
    void onBrowseMapImage();
    void onGenerateMapSlice();
    void hideNotification();

private:
    void setupUi();
    void backToEmpty();
    void clearAllFields();
    void initTimers();
    void initWatcher();
    QGroupBox* createGlassCard(const QString &title, QWidget *content,
                               QPushButton *&browseBtn, QPushButton *&actionBtn,
                               const QString &btnText);
    QGroupBox* createAtmoCard(const QString &title, QLineEdit *&hexEdit, QPushButton *&colorBtn);
    QString getAppPath();
    QString loadSavePath();
    void savePathToFile(const QString &path);
    void loadServersList();
    bool parseJsonFile(const QString &path);
    void showNotify(const QString &msg);
    void showNotifyConv(const QString &msg);
    void showAtmoNotify(const QString &msg);
    void showPaintNotify(const QString &msg);
    void showExtraNotify(const QString &msg);
    void runAsync(std::function<void()> task);
    void setButtonsEnabled(bool enabled);
    void loadYTD(const QString& path);
    int getSystemSizeFromFlag(unsigned int flag);
    QByteArray decompressZlib(const QByteArray& data, int expectedSize);
    QByteArray decodeDXT1(const QByteArray& data, int width, int height);
    QByteArray decodeDXT5(const QByteArray& data, int width, int height);
    bool saveToPNG(const TextureInfo& tex, const QString& path);
    void loadDFF(const QString& path);

    // ui
    QFrame *m_sidePanel;
    QPropertyAnimation *m_panelAnim;
    bool m_panelOpen;
    QStackedWidget *m_centerStack;
    QWidget *m_circlePage, *m_emptyPage, *m_copyPage, *m_convertPage, *m_atmospherePage, *m_paintingPage, *m_extraPage, *m_settingsPage, *m_3DViewPage, *m_texturingPage;
    QPushButton *m_centerBtn;
    QPushButton *m_btnMinimize;
    QPushButton *m_btnClose;

    // поля копирования
    QLineEdit *m_billboardPath, *m_logoPath, *m_listvaPath;
    QPushButton *m_btnBrowseBillboard, *m_btnCopyBillboard;
    QPushButton *m_btnBrowseLogo, *m_btnCopyLogo;
    QPushButton *m_btnBrowseListva, *m_btnCopyListva;

    // поля конвертации
    QLineEdit *m_bpcPath, *m_ifpAniPath, *m_modPath, *m_bpcMetaPath, *m_txdPath;
    QPushButton *m_btnBrowseBpc, *m_btnConvertBpc;
    QPushButton *m_btnBrowseIfpAni, *m_btnConvertIfpAni;
    QPushButton *m_btnBrowseMod, *m_btnConvertMod;
    QPushButton *m_btnBrowseBpcMeta, *m_btnConvertBpcMeta;
    QPushButton *m_btnBrowseTxd, *m_btnConvertTxd;
    QLineEdit *m_btxPath;
    QPushButton *m_btnBrowseBtx, *m_btnConvertBtx;

    // поля атмосферы
    QLineEdit *m_skyTopColor, *m_skyBottomColor, *m_sunColor, *m_cloudsColor, *m_environmentColor;
    QPushButton *m_btnSkyTopColor, *m_btnSkyBottomColor, *m_btnSunColor, *m_btnCloudsColor, *m_btnEnvironmentColor;
    QPushButton *m_btnGenerateTimecyc, *m_btnGenerateEnvironment;

    // поля покраски
    QLineEdit *m_hudColor, *m_buttonsColor, *m_paintImageColor, *m_paintImagePath;
    QPushButton *m_btnHudColor, *m_btnButtonsColor, *m_btnPaintImageColor;
    QPushButton *m_btnGenerateHud, *m_btnGenerateButtons;
    QPushButton *m_btnBrowsePaintImage, *m_btnGeneratePaintImage;

    // поля дополнительно
    QLineEdit *m_bloodColor, *m_bloodSize, *m_mapImagePath;
    QPushButton *m_btnBloodColor, *m_btnGenerateBlood;
    QPushButton *m_btnBrowseMapImage, *m_btnGenerateMapSlice;

    // настройки
    QLineEdit *m_outputPath;

    // уведомления
    QLabel *m_notificationLabel, *m_atmoNotifyLabel, *m_paintNotifyLabel, *m_extraNotifyLabel, *m_settingsNotifyLabel;
    QTimer *m_notificationTimer, *m_atmoTimer, *m_paintTimer, *m_extraTimer;

    // данные
    QList<QString> m_serverFirstNames;
    bool m_serversLoaded;
    int m_panelWidth;
    QFutureWatcher<void> *m_asyncWatcher;
    QThreadPool *m_threadPool;
    QMutex m_busyMutex;
    bool m_busy;

    // окно
    QIcon m_iconClose;
    QIcon m_iconWrap;
    QPoint m_dragPosition;
    bool m_dragging;
    
    // ytd
    QLineEdit *m_ytdPath;
    QPushButton *m_btnBrowseYtd, *m_btnExportYtd, *m_btnExportAllYtd;
    QListWidget *m_ytdTextureList;
    QLabel *m_ytdPreviewLabel;
    QLabel *m_ytdInfoLabel;
    QVector<TextureInfo> m_ytdTextures;
    
    // 3д просмотр
    QLineEdit *m_dffPath, *m_dffTexturePath;
    QPushButton *m_btnBrowseDFF, *m_btnBrowseDFFTexture, *m_btnApplyDFFTexture;
    QCheckBox *m_gridCheckBox;
    QComboBox *m_vertexColorCombo;
    QLabel *m_3dInfoLabel;
    DFFViewerWidget *m_3dViewer;
};

#endif