#include "KoTeKTools.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLTexture>
#include <QImage>
#include <QDebug>
#include <cmath>
#include <QHash>
#include <QVector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// расчетттт норсмалей
static void computeNormalsForTriangles(
    const QVector<DFFViewerWidget::Vertex3D>& vertices,
    const QVector<unsigned int>& indices,
    QVector<DFFViewerWidget::Vertex3D>& outNormals,
    float smoothingAngle = 45.0f)
{
    outNormals.resize(vertices.size());

    // обнуление нормалей
    for (int i = 0; i < outNormals.size(); ++i) {
        outNormals[i].x = 0.0f;
        outNormals[i].y = 0.0f;
        outNormals[i].z = 0.0f;
    }

    struct TriNormal {
        float nx, ny, nz;
    };
    QVector<TriNormal> triNormals(indices.size() / 3);

    for (int i = 0; i < indices.size(); i += 3) {
        int i1 = indices[i];
        int i2 = indices[i+1];
        int i3 = indices[i+2];

        if (i1 >= vertices.size() || i2 >= vertices.size() || i3 >= vertices.size())
            continue;

        const DFFViewerWidget::Vertex3D& v1 = vertices[i1];
        const DFFViewerWidget::Vertex3D& v2 = vertices[i2];
        const DFFViewerWidget::Vertex3D& v3 = vertices[i3];

        float ax = v2.x - v1.x, ay = v2.y - v1.y, az = v2.z - v1.z;
        float bx = v3.x - v1.x, by = v3.y - v1.y, bz = v3.z - v1.z;

        float nx = ay * bz - az * by;
        float ny = az * bx - ax * bz;
        float nz = ax * by - ay * bx;

        float len = sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 0.0001f) {
            nx /= len; ny /= len; nz /= len;
        }

        triNormals[i/3].nx = nx;
        triNormals[i/3].ny = ny;
        triNormals[i/3].nz = nz;

        outNormals[i1].x += nx; outNormals[i1].y += ny; outNormals[i1].z += nz;
        outNormals[i2].x += nx; outNormals[i2].y += ny; outNormals[i2].z += nz;
        outNormals[i3].x += nx; outNormals[i3].y += ny; outNormals[i3].z += nz;
    }

    // нормализуем итоговые нормали
    for (int i = 0; i < outNormals.size(); ++i) {
        float len = sqrt(outNormals[i].x*outNormals[i].x + 
                        outNormals[i].y*outNormals[i].y + 
                        outNormals[i].z*outNormals[i].z);
        if (len > 0.0001f) {
            outNormals[i].x /= len;
            outNormals[i].y /= len;
            outNormals[i].z /= len;
        } else {
            outNormals[i].x = 0.0f; outNormals[i].y = 1.0f; outNormals[i].z = 0.0f;
        }
    }
}

// DFFLoader
DFFLoader::DFFLoader() : m_stream(&m_file) {
    m_stream.setByteOrder(QDataStream::LittleEndian);
    m_stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
}

bool DFFLoader::loadFile(const QString& filename, DFF::DFFFile& outFile) {
    outFile.clear();
    outFile.fileName = filename;
    
    m_file.setFileName(filename);
    if (!m_file.open(QIODevice::ReadOnly)) {
        m_lastError = "Cannot open file: " + filename;
        return false;
    }
    
    while (!m_stream.atEnd()) {
        Chunk chunk;
        if (!readChunk(chunk))
            break;
        
        if (chunk.type == CHUNK_CLUMP) {
            if (!parseClump(outFile, chunk)) {
                m_lastError = "Failed to parse clump";
                return false;
            }
        } else {
            skipChunk(chunk);
        }
    }
    
    m_file.close();
    return true;
}

bool DFFLoader::readChunk(Chunk& chunk) {
    if (m_stream.atEnd())
        return false;

    m_stream >> chunk.type >> chunk.size >> chunk.version;
    return true;
}

bool DFFLoader::skipChunk(const Chunk& chunk) {
    return m_stream.skipRawData(chunk.size) == chunk.size;
}

bool DFFLoader::parseClump(DFF::DFFFile& file, const Chunk& clumpChunk) {
    qint64 chunkEnd = m_stream.device()->pos() + clumpChunk.size;
    
    while (m_stream.device()->pos() < chunkEnd) {
        Chunk chunk;
        if (!readChunk(chunk))
            break;
        
        switch (chunk.type) {
        case CHUNK_FRAMELIST:
            if (!parseFrameList(file, chunk))
                return false;
            break;

        case CHUNK_GEOMETRYLIST:
            if (!parseGeometryList(file, chunk))
                return false;
            break;

        case CHUNK_ATOMIC:
            if (!parseAtomic(file, chunk))
                return false;
            break;

        default:
            skipChunk(chunk);
            break;
        }
    }

    return true;
}


bool DFFLoader::parseFrameList(DFF::DFFFile& file, const Chunk& chunk) {
    qint64 chunkEnd = m_stream.device()->pos() + chunk.size;
    
    Chunk structChunk;
    if (!readChunk(structChunk) || structChunk.type != CHUNK_STRUCT)
        return false;
    
    quint32 numFrames;
    
    m_stream >> numFrames;
    
    for (quint32 i = 0; i < numFrames; i++) {
        DFF::Frame frame;
        QMatrix4x4 mat;
        
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                float val;
                m_stream >> val;
                mat(row, col) = val;
            }
        }
        
        float x, y, z;
        m_stream >> x >> y >> z;
        mat(0, 3) = x;
        mat(1, 3) = y;
        mat(2, 3) = z;
        
        frame.matrix = mat;
        
        qint32 parentId;
        m_stream >> parentId;
        frame.parentId = parentId;
        
        quint32 flags;
        
        m_stream >> flags;
        
        file.frames.append(frame);
    }
    
    while (m_stream.device()->pos() < chunkEnd) {
        Chunk extChunk;
        if (!readChunk(extChunk))
            break;
        skipChunk(extChunk);
    }
    
    return true;
}

// парсер геометрий
bool DFFLoader::parseGeometryList(DFF::DFFFile& file, const Chunk& chunk) {
    qint64 chunkEnd = m_stream.device()->pos() + chunk.size;
    
    Chunk structChunk;
    if (!readChunk(structChunk) || structChunk.type != CHUNK_STRUCT)
        return false;
    
    quint32 numGeometries;
    m_stream >> numGeometries;
    
    // парсим каждую геометрию
    while (m_stream.device()->pos() < chunkEnd) {
        Chunk geomChunk;
        if (!readChunk(geomChunk))
            break;
        
        if (geomChunk.type == CHUNK_GEOMETRY) {
            DFF::Geometry geom;
            if (!parseGeometry(file, geomChunk, geom))
                return false;
            file.geometries.append(geom);
        } else {
            skipChunk(geomChunk);
        }
    }
    
    return true;
}

// парсер геометрий
bool DFFLoader::parseGeometry(DFF::DFFFile& file, const Chunk& chunk, DFF::Geometry& geom) {
    qint64 chunkEnd = m_stream.device()->pos() + chunk.size;
    
    Chunk structChunk;
    if (!readChunk(structChunk) || structChunk.type != CHUNK_STRUCT)
        return false;
    
    m_stream >> geom.flags;
    quint32 numTriangles, numVertices, numMorphTargets;
    
    m_stream >> numTriangles >> numVertices >> numMorphTargets;
    
    // цвета для старых версий
    if (chunk.version < 0x34000) {
        float ambient, specular, diffuse;
        m_stream >> ambient >> specular >> diffuse;
    }
    
    if (geom.flags & rpGEOMETRYPRELIT) {
        geom.hasPrelitColors = true;
        for (quint32 i = 0; i < numVertices; i++) {
            unsigned char r, g, b, a;
            m_stream >> r >> g >> b >> a;
            geom.prelitColors.append(DFF::RGBA(r, g, b, a));
        }
    }
    
    // текстурные координаты
    int texCount = (geom.flags & rpGEOMETRYTEXTURED2) ? 2 : 

                   (geom.flags & rpGEOMETRYTEXTURED) ? 1 : 0;
    
    if (texCount > 0) {
        geom.hasTexCoords = true;
        for (int t = 0; t < texCount; t++) {
            for (quint32 i = 0; i < numVertices; i++) {
                float u, v;
                m_stream >> u >> v;
                geom.texCoords.append(DFF::Vector2(u, v));
            }
        }
    }
    
    // треугольники
    for (quint32 i = 0; i < numTriangles; i++) {
        unsigned short a, b, material, c;
        
        m_stream >> a >> b >> material >> c;
        geom.triangles.append(DFF::Triangle(a, b, c, material));
    }
    
    m_stream >> geom.boundingSphere.center.x >> geom.boundingSphere.center.y
    
             >> geom.boundingSphere.center.z >> geom.boundingSphere.radius;
    
    quint32 hasVertices, hasNormals;
    m_stream >> hasVertices >> hasNormals;
    geom.hasNormals = (hasNormals != 0);
    
    // вершины
    for (quint32 i = 0; i < numVertices; i++) {
        float x, y, z;
        m_stream >> x >> y >> z;
        geom.vertices.append(DFF::Vector3(x, y, z));
    }
    
    // нормали
    if (hasNormals) {
        for (quint32 i = 0; i < numVertices; i++) {
            float x, y, z;
            m_stream >> x >> y >> z;
            geom.normals.append(DFF::Vector3(x, y, z));
        }
    }
    
    while (m_stream.device()->pos() < chunkEnd) {
        Chunk subChunk;
        if (!readChunk(subChunk))
            break;
        
        if (subChunk.type == CHUNK_MATERIALLIST) {
            if (!parseMaterialList(file, subChunk, geom))
                return false;
        } else {
            skipChunk(subChunk);
        }
    }
    
    return true;
}

bool DFFLoader::parseMaterialList(DFF::DFFFile& file, const Chunk& chunk, DFF::Geometry& geom) {
    qint64 chunkEnd = m_stream.device()->pos() + chunk.size;
    
    Chunk structChunk;
    if (!readChunk(structChunk) || structChunk.type != CHUNK_STRUCT)
        return false;
    
    quint32 numMaterials;
    m_stream >> numMaterials;
    
    for (quint32 i = 0; i < numMaterials; i++) {
        qint32 matIndex;
        m_stream >> matIndex;
    }
    
    // парсер каждого материала
    while (m_stream.device()->pos() < chunkEnd) {
        Chunk matChunk;
        if (!readChunk(matChunk))
            break;
        
        if (matChunk.type == CHUNK_MATERIAL) {
            DFF::Material mat;
            if (parseMaterial(mat, matChunk.version)) {
                geom.materials.append(mat);
            }
        } else {
            skipChunk(matChunk);
        }
    }
    
    return true;
}

// парсинг материала
bool DFFLoader::parseMaterial(DFF::Material& mat, quint32 version) {
    qint64 chunkEnd = m_stream.device()->pos() + 12;
    
    Chunk structChunk;
    if (!readChunk(structChunk) || structChunk.type != CHUNK_STRUCT)
        return false;
    
    chunkEnd = m_stream.device()->pos() + structChunk.size;
    
    quint32 flags;
    m_stream >> flags;
    
    // цвет материала
    unsigned char r, g, b, a;
    m_stream >> r >> g >> b >> a;
    mat.color = DFF::RGBA(r, g, b, a);
    
    quint32 unk1, isTextured;
    m_stream >> unk1 >> isTextured;
    mat.isTextured = (isTextured != 0);
    
    // старые версии
    if (version < 0x34000) {
        m_stream >> mat.ambient >> mat.specular >> mat.diffuse;
    }
    
    while (m_stream.device()->pos() < chunkEnd) {
        Chunk subChunk;
        if (!readChunk(subChunk))
            break;
        
        if (subChunk.type == CHUNK_TEXTURE) {
            if (!parseTexture(mat))
                return false;
        } else {
            skipChunk(subChunk);
        }
    }
    
    return true;
}

// парсинг текстур
bool DFFLoader::parseTexture(DFF::Material& mat) {
    Chunk structChunk;
    if (!readChunk(structChunk) || structChunk.type != CHUNK_STRUCT)
        return false;
    
    // параметры текстуры
    unsigned char filterMode, uvAddressing;
    unsigned short unknown;
    m_stream >> filterMode >> uvAddressing >> unknown;
    mat.filterMode = filterMode;
    mat.uvAddressing = uvAddressing;
    
    // имя текстуры
    Chunk nameChunk;
    if (!readChunk(nameChunk) || nameChunk.type != CHUNK_STRING)
        return false;
    
    QString name;
    for (quint32 i = 0; i < nameChunk.size; i++) {
        char c;
        m_stream >> c;
        if (c == '\0') break;
        name += c;
    }
    mat.textureName = name;
    
    Chunk maskChunk;
    if (readChunk(maskChunk) && maskChunk.type == CHUNK_STRING) {
        skipChunk(maskChunk);
    }
    
    // расширения
    Chunk extChunk;
    if (readChunk(extChunk) && extChunk.type == CHUNK_EXTENSION) {
        skipChunk(extChunk);
    }
    
    return true;
}

bool DFFLoader::parseAtomic(DFF::DFFFile& file, const Chunk& chunk) {
    qint64 chunkEnd = m_stream.device()->pos() + chunk.size;
    DFF::Atomic atomic;
    
    while (m_stream.device()->pos() < chunkEnd) {
        Chunk subChunk;
        if (!readChunk(subChunk))
            break;
        
        if (subChunk.type == CHUNK_STRUCT) {
            m_stream >> atomic.frameIndex >> atomic.geometryIndex;
            quint32 flags, unk;
            m_stream >> flags >> unk;
        } else {
            skipChunk(subChunk);
        }
    }
    
    file.atomics.append(atomic);
    return true;
}

// DFFViewerWidget
DFFViewerWidget::DFFViewerWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_rotationX(25.0f)
    , m_rotationY(35.0f)
    , m_distance(5.0f)
    , m_hasModel(false)
    , m_hasTexture(false)
    , m_showGrid(true)
    , m_vertexColorMode(0)
    , m_texture(nullptr)
    , m_currentTextureIndex(0)
    , m_minDistance(1.0f)
    , m_maxDistance(50.0f)
{
    setMinimumSize(500, 400);
    setFocusPolicy(Qt::StrongFocus);
    
    // настройка сглаживания
    QSurfaceFormat format;
    format.setSamples(16);
    format.setSwapInterval(1);
    setFormat(format);
}

DFFViewerWidget::~DFFViewerWidget()
{
    makeCurrent();
    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }
    doneCurrent();
}

void DFFViewerWidget::loadDFF(const DFF::DFFFile& dff)
{
    m_currentDFF = dff;
    m_hasModel = !m_currentDFF.geometries.isEmpty();
    
    m_vertices.clear();
    m_normals.clear();
    m_texCoords.clear();
    m_colors.clear();
    m_indices.clear();
    
    if (!m_hasModel) return;
    
    const DFF::Geometry& geom = m_currentDFF.geometries[0];
    
    // копирование вершин
    for (const auto& v : geom.vertices) {
        Vertex3D vert;
        vert.x = v.x; vert.y = v.y; vert.z = v.z;
        m_vertices.append(vert);
    }
    
    // копирование нормалей
    for (const auto& n : geom.normals) {
        Vertex3D norm;
        norm.x = n.x; norm.y = n.y; norm.z = n.z;
        m_normals.append(norm);
    }
    
    // копирование текстурных координат
    for (const auto& uv : geom.texCoords) {
        UV2D tex;
        tex.u = uv.u; tex.v = uv.v;
        m_texCoords.append(tex);
    }
    
    // копирование цветов вершин
    for (const auto& c : geom.prelitColors) {
        ColorRGBA col;
        col.r = c.r; col.g = c.g; col.b = c.b; col.a = c.a;
        m_colors.append(col);
    }
    
    // копируем индексы треугольников
    for (const auto& tri : geom.triangles) {
        m_indices.append(tri.v1);
        m_indices.append(tri.v2);
        m_indices.append(tri.v3);
    }
    
    // генерация нормалей если их нет
    if (m_normals.isEmpty() && !m_vertices.isEmpty()) {
        computeNormalsForTriangles(m_vertices, m_indices, m_normals, 45.0f);
    }
    
    // вычисление центра модели
    if (!m_vertices.isEmpty()) {
        float minX = m_vertices[0].x, maxX = m_vertices[0].x;
        float minY = m_vertices[0].y, maxY = m_vertices[0].y;
        float minZ = m_vertices[0].z, maxZ = m_vertices[0].z;
        
        for (const auto& v : m_vertices) {
            minX = qMin(minX, v.x); maxX = qMax(maxX, v.x);
            minY = qMin(minY, v.y); maxY = qMax(maxY, v.y);
            minZ = qMin(minZ, v.z); maxZ = qMax(maxZ, v.z);
        }
        
        m_modelCenter.x = (minX + maxX) / 2.0f;
        m_modelCenter.y = (minY + maxY) / 2.0f;
        m_modelCenter.z = (minZ + maxZ) / 2.0f;
        
        float dx = maxX - minX;
        float dy = maxY - minY;
        float dz = maxZ - minZ;
        m_modelRadius = sqrt(dx*dx + dy*dy + dz*dz) / 2.0f;
        
        // минимальная дистанция
        m_minDistance = m_modelRadius * 1.0f;
        if (m_minDistance < 0.5f) m_minDistance = 0.5f;
        
        // максимальная дистанция
        m_maxDistance = m_modelRadius * 10.0f;
        if (m_maxDistance < 10.0f) m_maxDistance = 10.0f;
        
        // начальная дистанция
        m_distance = m_modelRadius * 4.0f;
        if (m_distance < 3.0f) m_distance = 3.0f;
        if (m_distance > m_maxDistance) m_distance = m_maxDistance;
    }
    
    // текстуры в директории
    QString basePath = QFileInfo(m_currentDFF.fileName).absolutePath();
    m_texturePaths.clear();
    
    for (const auto& mat : geom.materials) {
        if (mat.isTextured && !mat.textureName.isEmpty()) {
            QStringList extensions = {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds"};
            bool found = false;

            for (const QString& ext : extensions) {
                QString texPath = basePath + "/" + mat.textureName + ext;
                if (QFile::exists(texPath)) {
                    m_texturePaths.append(texPath);
                    found = true;
                    break;
                }
            }

            if (!found) {
                qDebug() << "Texture not found for material:" << mat.textureName;
            }
        }
    }
    
    // загружение первой найденной текстуры
    if (!m_texturePaths.isEmpty()) {
        loadTexture(m_texturePaths.first());
    }
    
    update();
}

void DFFViewerWidget::loadTexture(const QString& path)
{
    qDebug() << "Loading texture:" << path;
    
    makeCurrent();
    
    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }
    
    QByteArray pathBytes = path.toLocal8Bit();
    const char* filename = pathBytes.constData();
    
    // загружение текстуры через stb image
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    
    if (!data) {
        qWarning() << "Failed to load texture:" << path;
        qWarning() << "STB Error:" << stbi_failure_reason();
        m_hasTexture = false;
        doneCurrent();
        return;
    }
    
    qDebug() << "Texture loaded:" << width << "x" << height << "channels:" << channels;
    
    // создание текстуры OPENGL
    QImage image(data, width, height, width * 4, QImage::Format_RGBA8888);
    QImage imageCopy = image.copy();
    stbi_image_free(data);
    
    m_texture = new QOpenGLTexture(imageCopy);
    m_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_texture->setMaximumAnisotropy(16.0f);
    m_texture->setWrapMode(QOpenGLTexture::Repeat);
    m_texture->generateMipMaps();
    
    m_hasTexture = true;
    m_currentTexturePath = path;
    
    qDebug() << "OpenGL texture created successfully. ID:" << m_texture->textureId();
    
    doneCurrent();
    update();
}

void DFFViewerWidget::setShowGrid(bool show)
{
    m_showGrid = show;
    update();
}

void DFFViewerWidget::setVertexColorMode(int mode)
{
    m_vertexColorMode = mode;
    update();
}

void DFFViewerWidget::clearModel()
{
    makeCurrent();
    m_vertices.clear();
    m_normals.clear();
    m_texCoords.clear();
    m_colors.clear();
    m_indices.clear();
    m_texturePaths.clear();

    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }

    m_hasModel = false;
    m_hasTexture = false;
    m_minDistance = 0.1f;
    m_maxDistance = 50.0f;
    m_distance = 5.0f;
    doneCurrent();
    update();
}

void DFFViewerWidget::initializeGL()
{
    initializeOpenGLFunctions();
    
    // цвет фона
    glClearColor(0.055f, 0.063f, 0.125f, 1.0f);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
    glEnable(GL_MULTISAMPLE);
    
    glDisable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    // окружающее освещение
    GLfloat ambient[] = {0.35f, 0.35f, 0.4f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    
    // основной источник света
    GLfloat mainPos[] = {2.0f, 3.0f, 2.0f, 0.0f};
    GLfloat mainDiffuse[] = {1.0f, 0.95f, 0.9f, 1.0f};
    GLfloat mainSpecular[] = {0.4f, 0.4f, 0.4f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, mainPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, mainDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, mainSpecular);
    
    // заполняющий свет
    GLfloat fillPos[] = {-0.5f, 0.5f, -1.0f, 0.0f};
    GLfloat fillDiffuse[] = {0.3f, 0.35f, 0.4f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, fillPos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fillDiffuse);
    
    // свойства материала
    GLfloat matAmbient[] = {0.7f, 0.7f, 0.75f, 1.0f};
    GLfloat matDiffuse[] = {0.8f, 0.8f, 0.85f, 1.0f};
    GLfloat matSpecular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat matShininess[] = {64.0f};
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void DFFViewerWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void DFFViewerWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    float aspect = float(width()) / float(height());
    float fov = 45.0f;
    float zNear = 0.1f;
    float zFar = 1000.0f;
    float f = 1.0f / tan(fov * M_PI / 360.0f);
    
    float proj[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (zFar + zNear) / (zNear - zFar), -1,
        0, 0, (2 * zFar * zNear) / (zNear - zFar), 0
    };
    glLoadMatrixf(proj);
    
    // камера
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -m_distance);
    glRotatef(m_rotationX, 1, 0, 0);
    glRotatef(m_rotationY, 0, 1, 0);
    
    GLfloat mainPos[] = {2.0f, 3.0f, 2.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, mainPos);
    
    // рисование сетки
    if (m_showGrid) {
        drawGrid();
    }
    
    drawModel();
}

void DFFViewerWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
}

void DFFViewerWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        // вращение левой кнопкой
        QPoint delta = event->pos() - m_lastMousePos;
        m_rotationY += delta.x() * 0.5f;
        m_rotationX += delta.y() * 0.5f;
        m_rotationX = qBound(-90.0f, m_rotationX, 90.0f);
        update();
    }
    else if (event->buttons() & Qt::MiddleButton) {
        // зум
        QPoint delta = event->pos() - m_lastMousePos;
        m_distance += delta.y() * 0.05f;
        m_distance = qBound(m_minDistance, m_distance, m_maxDistance);
        update();
    }

    m_lastMousePos = event->pos();
}

void DFFViewerWidget::wheelEvent(QWheelEvent *event)
{
    // зум колесиком
    m_distance -= event->angleDelta().y() / 120.0f * 0.5f;
    m_distance = qBound(m_minDistance, m_distance, m_maxDistance);
    update();
}

void DFFViewerWidget::drawGrid()
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    float gridSize = 10.0f;
    float bigStep = 1.0f;
    float smallStep = 0.1f;
    
    // маленькие линии сетки
    glLineWidth(0.5f);
    glBegin(GL_LINES);
    
    float smallColor = 0.15f;
    for (float i = -gridSize; i <= gridSize; i += smallStep) {
        if (fmod(fabs(i), bigStep) < 0.001f) continue;
        
        glColor3f(smallColor, smallColor, smallColor);
        glVertex3f(i, 0, -gridSize);
        glVertex3f(i, 0, gridSize);
        glVertex3f(-gridSize, 0, i);
        glVertex3f(gridSize, 0, i);
    }
    
    // большие линии сетки
    glLineWidth(0.8f);
    float bigColor_r = 0x49 / 255.0f;
    float bigColor_g = 0x49 / 255.0f;
    float bigColor_b = 0x49 / 255.0f;
    
    for (float i = -gridSize; i <= gridSize; i += bigStep) {
        glColor3f(bigColor_r, bigColor_g, bigColor_b);
        glVertex3f(i, 0, -gridSize);
        glVertex3f(i, 0, gridSize);
        glVertex3f(-gridSize, 0, i);
        glVertex3f(gridSize, 0, i);
    }
    
    glEnd();
    
    // оси координат
    glLineWidth(1.2f);
    
    // ось X
    float green_r = 0x5b / 255.0f;
    float green_g = 0x7c / 255.0f;
    float green_b = 0x2f / 255.0f;

    glBegin(GL_LINES);
    glColor3f(green_r, green_g, green_b);
    glVertex3f(-gridSize, 0, 0);
    glVertex3f(gridSize, 0, 0);
    glEnd();
    
    // ось Z
    float red_r = 0x88 / 255.0f;
    float red_g = 0x3d / 255.0f;
    float red_b = 0x47 / 255.0f;

    glBegin(GL_LINES);
    glColor3f(red_r, red_g, red_b);
    glVertex3f(0, 0, -gridSize);
    glVertex3f(0, 0, gridSize);
    glEnd();
    
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void DFFViewerWidget::drawModel()
{
    if (!m_hasModel || m_vertices.isEmpty() || m_indices.isEmpty()) {
        return;
    }
    
    glPushMatrix();
    
    // в DFF ось Y вверх, в OpenGL Z вверх
    glRotatef(-90, 1, 0, 0);
    
    bool useTexture = m_hasTexture && m_texture && !m_texCoords.isEmpty();
    bool usePrelit = (m_vertexColorMode == 1) && !m_colors.isEmpty();
    
    // привязывает текстуру
    if (useTexture) {
        glEnable(GL_TEXTURE_2D);
        if (m_texture->isCreated()) {
            m_texture->bind();
        } else {
            qWarning() << "Texture not created!";
            useTexture = false;
        }
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
    }
    
    glBegin(GL_TRIANGLES);
    
    // все треугольники рисует
    for (int i = 0; i < m_indices.size(); i += 3) {
        int i1 = m_indices[i];
        int i2 = m_indices[i+1];
        int i3 = m_indices[i+2];
        
        if (i1 >= m_vertices.size() || i2 >= m_vertices.size() || i3 >= m_vertices.size())
            continue;
        
        const Vertex3D& v1 = m_vertices[i1];
        const Vertex3D& v2 = m_vertices[i2];
        const Vertex3D& v3 = m_vertices[i3];
        
        // вершина 1
        if (usePrelit && i1 < m_colors.size()) {
            const ColorRGBA& c1 = m_colors[i1];
            glColor4ub(c1.r, c1.g, c1.b, c1.a);
        } else if (!useTexture) {
            glColor3f(0.7f, 0.7f, 0.75f);
        }
        
        if (i1 < m_normals.size()) {
            const Vertex3D& n1 = m_normals[i1];
            glNormal3f(n1.x, n1.y, n1.z);
        }
        
        if (useTexture && i1 < m_texCoords.size()) {
            const UV2D& uv1 = m_texCoords[i1];
            glTexCoord2f(uv1.u, uv1.v);
        }

        glVertex3f(v1.x, v1.y, v1.z);
        
        // вершина 2
        if (usePrelit && i2 < m_colors.size()) {
            const ColorRGBA& c2 = m_colors[i2];
            glColor4ub(c2.r, c2.g, c2.b, c2.a);
        } else if (!useTexture) {
            glColor3f(0.7f, 0.7f, 0.75f);
        }

        if (i2 < m_normals.size()) {
            const Vertex3D& n2 = m_normals[i2];
            glNormal3f(n2.x, n2.y, n2.z);
        }

        if (useTexture && i2 < m_texCoords.size()) {
            const UV2D& uv2 = m_texCoords[i2];
            glTexCoord2f(uv2.u, uv2.v);
        }

        glVertex3f(v2.x, v2.y, v2.z);
        
        // вершина 3
        if (usePrelit && i3 < m_colors.size()) {
            const ColorRGBA& c3 = m_colors[i3];
            glColor4ub(c3.r, c3.g, c3.b, c3.a);
        } else if (!useTexture) {
            glColor3f(0.7f, 0.7f, 0.75f);
        }

        if (i3 < m_normals.size()) {
            const Vertex3D& n3 = m_normals[i3];
            glNormal3f(n3.x, n3.y, n3.z);
        }

        if (useTexture && i3 < m_texCoords.size()) {
            const UV2D& uv3 = m_texCoords[i3];
            glTexCoord2f(uv3.u, uv3.v);
        }

        glVertex3f(v3.x, v3.y, v3.z);
    }
    
    glEnd();
    
    if (useTexture && m_texture && m_texture->isCreated()) {
        m_texture->release();
    }
    
    glPopMatrix();
}

// Kotek Tools
void KoTeKTools::onBrowseDFF()
{
    QString path = QFileDialog::getOpenFileName(this, "Выбрать DFF файл", "", "DFF Files (*.dff);;All Files (*)");

    if (!path.isEmpty()) {
        m_dffPath->setText(path);
        loadDFF(path);
    }
}

void KoTeKTools::onBrowseDFFTexture()
{
    QString path = QFileDialog::getOpenFileName(this, "Выбрать текстуру", "", 
        "Images (*.png *.jpg *.jpeg *.bmp *.tga *.dds);;All Files (*)");

    if (!path.isEmpty()) {
        m_dffTexturePath->setText(path);

        if (m_3dViewer) {
            m_3dViewer->loadTexture(path);
            m_3dInfoLabel->setText("Текстура загружена: " + QFileInfo(path).fileName());
        }
    }
}

void KoTeKTools::onApplyDFFTexture()
{
    if (m_3dViewer && !m_dffTexturePath->text().isEmpty()) {
        QString path = m_dffTexturePath->text();

        if (QFile::exists(path)) {
            m_3dViewer->loadTexture(path);
            m_3dInfoLabel->setText("Текстура применена: " + QFileInfo(path).fileName());
        } else {
            m_3dInfoLabel->setText("Ошибка: файл текстуры не найден!");
            QMessageBox::warning(this, "Ошибка", "Файл текстуры не найден:\n" + path);
        }
    }
}

void KoTeKTools::onGridToggled(bool checked)
{
    if (m_3dViewer) {
        m_3dViewer->setShowGrid(checked);
    }
}

void KoTeKTools::onVertexColorModeChanged(int index)
{
    if (m_3dViewer) {
        m_3dViewer->setVertexColorMode(index);
    }
}

void KoTeKTools::loadDFF(const QString& path)
{
    if (!m_3dViewer) return;
    
    DFFLoader loader;
    DFF::DFFFile dff;
    
    if (!loader.loadFile(path, dff)) {
        m_3dInfoLabel->setText("Ошибка загрузки: " + loader.getLastError());
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить DFF файл:\n" + loader.getLastError());
        return;
    }
    
    m_3dViewer->loadDFF(dff);
    
    int verts = dff.geometries.isEmpty() ? 0 : dff.geometries[0].vertices.size();
    int tris = dff.geometries.isEmpty() ? 0 : dff.geometries[0].triangles.size();
    int mats = dff.geometries.isEmpty() ? 0 : dff.geometries[0].materials.size();
    
    m_3dInfoLabel->setText(QString("Загружено: %1 | Вершин: %2 | Треугольников: %3 | Материалов: %4")
        .arg(QFileInfo(path).fileName()).arg(verts).arg(tris).arg(mats));
}