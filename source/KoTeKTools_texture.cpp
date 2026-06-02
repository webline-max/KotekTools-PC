#include "KoTeKTools.h"
#include "zlib.h"
#include <zip.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QApplication>
#include <cstring>
#include <QTemporaryDir>
#include <QRegularExpression>
#include <QRandomGenerator>

#pragma pack(push, 1)
struct RscHeader {
    unsigned int fourcc;
    unsigned int version;
    unsigned int sysFlags;
    unsigned int gfxFlags;
};

struct DDS_HEADER {
    unsigned int magic;
    unsigned int size;
    unsigned int flags;
    unsigned int height;
    unsigned int width;
    unsigned int pitchOrLinearSize;
    unsigned int depth;
    unsigned int mipMapCount;
    unsigned int reserved1[11];
    unsigned int pfSize;
    unsigned int pfFlags;
    unsigned int fourCC;
    unsigned int rgbBitCount;
    unsigned int rMask;
    unsigned int gMask;
    unsigned int bMask;
    unsigned int aMask;
    unsigned int caps;
    unsigned int caps2;
    unsigned int caps3;
    unsigned int caps4;
    unsigned int reserved2;
};
#pragma pack(pop)

#define RSC7_MAGIC 0x37435352
#define DDS_MAGIC 0x20534444
#define D3DFMT_DXT1 0x31545844
#define D3DFMT_DXT5 0x35545844

// ввчисление
int KoTeKTools::getSystemSizeFromFlag(unsigned int flag)
{
    int baseSize = 0x2000;
    baseSize <<= (flag & 0xf);
    int size = ((((flag >> 17) & 0x7f) + ((flag >> 11) & 0x3f) * 2 + 
                 ((flag >> 7) & 0xf) * 4 + ((flag >> 5) & 0x3) * 8 + 
                 ((flag >> 4) & 0x1) * 16)) * baseSize;
    for (int i = 0; i < 4; i++) {
        size += (((flag >> (24 + i)) & 1) == 1) ? (baseSize >> (1 + i)) : 0;
    }
    return size;
}

QByteArray KoTeKTools::decompressZlib(const QByteArray& data, int expectedSize)
{
    // zlib распаковка
    QByteArray result(expectedSize, 0);
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    
    if (inflateInit2(&strm, -15) != Z_OK) return QByteArray();
    
    strm.next_in = (Bytef*)data.data();
    strm.avail_in = data.size();
    strm.next_out = (Bytef*)result.data();
    strm.avail_out = expectedSize;
    
    int ret = inflate(&strm, Z_FINISH);
    inflateEnd(&strm);
    
    if (ret != Z_STREAM_END) return QByteArray();
    return result;
}

// dxt1
QByteArray KoTeKTools::decodeDXT1(const QByteArray& data, int width, int height)
{
    // dxt1 в rgba
    int bw = (width + 3) / 4;
    int bh = (height + 3) / 4;
    QByteArray result(width * height * 4, 0);
    
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            const unsigned char* block = (const unsigned char*)data.constData() + (by * bw + bx) * 8;
            unsigned char dst[64] = {0};
            
            unsigned short c0 = block[0] | (block[1] << 8);
            unsigned short c1 = block[2] | (block[3] << 8);
            unsigned int bits = block[4] | (block[5] << 8) | (block[6] << 16) | (block[7] << 24);
            
            
            unsigned char colors[4][3];
            colors[0][0] = ((c0 >> 11) & 0x1F) << 3;
            colors[0][1] = ((c0 >> 5) & 0x3F) << 2;
            colors[0][2] = (c0 & 0x1F) << 3;
            
            
            colors[1][0] = ((c1 >> 11) & 0x1F) << 3;
            colors[1][1] = ((c1 >> 5) & 0x3F) << 2;
            colors[1][2] = (c1 & 0x1F) << 3;
            
            
            if (c0 > c1) {
                for (int i = 0; i < 3; i++) {
                    colors[2][i] = (colors[0][i] * 2 + colors[1][i]) / 3;
                    colors[3][i] = (colors[0][i] + colors[1][i] * 2) / 3;
                }
            } else {
                for (int i = 0; i < 3; i++) colors[2][i] = (colors[0][i] + colors[1][i]) / 2;
                memset(colors[3], 0, 3);
            }
            
            for (int i = 0; i < 16; i++) {
                int idx = (bits >> (i * 2)) & 3;
                dst[i * 4 + 0] = colors[idx][0];
                dst[i * 4 + 1] = colors[idx][1];
                dst[i * 4 + 2] = colors[idx][2];
                dst[i * 4 + 3] = (idx == 3 && c0 <= c1) ? 0 : 255;
            }
            
            // копи 4х4 блок
            for (int y = 0; y < 4; y++) {
                int py = by * 4 + y;
                if (py >= height) break;
                for (int x = 0; x < 4; x++) {
                    int px = bx * 4 + x;
                    if (px >= width) break;
                    memcpy(result.data() + (py * width + px) * 4, dst + (y * 4 + x) * 4, 4);
                }
            }
        }
    }
    return result;
}

// dxt5
QByteArray KoTeKTools::decodeDXT5(const QByteArray& data, int width, int height)
{
    // dxt5 в rgba
    int bw = (width + 3) / 4;
    int bh = (height + 3) / 4;
    QByteArray result(width * height * 4, 0);
    
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            const unsigned char* block = (const unsigned char*)data.constData() + (by * bw + bx) * 16;
            unsigned char dst[64] = {0};
            
            // альфа
            unsigned char alpha0 = block[0];
            unsigned char alpha1 = block[1];
            unsigned long long alphaBits = block[2] | (block[3] << 8) | (block[4] << 16) | (block[5] << 24) |
                                           ((unsigned long long)block[6] << 32) | ((unsigned long long)block[7] << 40);
            
            unsigned char alphas[8];
            alphas[0] = alpha0;
            alphas[1] = alpha1;
            if (alpha0 > alpha1) {
                for (int i = 2; i < 8; i++) alphas[i] = (unsigned char)(((8 - i) * alpha0 + (i - 1) * alpha1) / 7);
            } else {
                for (int i = 2; i < 6; i++) alphas[i] = (unsigned char)(((6 - i) * alpha0 + (i - 1) * alpha1) / 5);
                alphas[6] = 0;
                alphas[7] = 255;
            }
            
            const unsigned char* colorBlock = block + 8;
            unsigned short c0 = colorBlock[0] | (colorBlock[1] << 8);
            unsigned short c1 = colorBlock[2] | (colorBlock[3] << 8);
            unsigned int colorBits = colorBlock[4] | (colorBlock[5] << 8) | (colorBlock[6] << 16) | (colorBlock[7] << 24);
            
            unsigned char colors[4][3];
            colors[0][0] = ((c0 >> 11) & 0x1F) << 3;
            colors[0][1] = ((c0 >> 5) & 0x3F) << 2;
            colors[0][2] = (c0 & 0x1F) << 3;
            colors[1][0] = ((c1 >> 11) & 0x1F) << 3;
            colors[1][1] = ((c1 >> 5) & 0x3F) << 2;
            colors[1][2] = (c1 & 0x1F) << 3;
            for (int i = 0; i < 3; i++) {
                colors[2][i] = (colors[0][i] * 2 + colors[1][i]) / 3;
                colors[3][i] = (colors[0][i] + colors[1][i] * 2) / 3;
            }
            
        
            for (int i = 0; i < 16; i++) {
                int alphaIdx = (alphaBits >> (i * 3)) & 7;
                int colorIdx = (colorBits >> (i * 2)) & 3;
                dst[i * 4 + 0] = colors[colorIdx][0];
                dst[i * 4 + 1] = colors[colorIdx][1];
                dst[i * 4 + 2] = colors[colorIdx][2];
                dst[i * 4 + 3] = alphas[alphaIdx];
            }
            
            for (int y = 0; y < 4; y++) {
                int py = by * 4 + y;
                if (py >= height) break;
                for (int x = 0; x < 4; x++) {
                    int px = bx * 4 + x;
                    if (px >= width) break;
                    memcpy(result.data() + (py * width + px) * 4, dst + (y * 4 + x) * 4, 4);
                }
            }
        }
    }
    return result;
}

// загрузка и парсинг 
void KoTeKTools::loadYTD(const QString& path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) { QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл"); return; }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (data.size() < sizeof(RscHeader)) { QMessageBox::warning(this, "Ошибка", "Файл слишком маленький"); return; }
    
    const RscHeader* hdr = (const RscHeader*)data.constData();
    if (hdr->fourcc != RSC7_MAGIC) { QMessageBox::warning(this, "Ошибка", "Это не YTD файл"); return; }
    
    int sysSize = getSystemSizeFromFlag(hdr->sysFlags);
    int gfxSize = getSystemSizeFromFlag(hdr->gfxFlags);
    
    // распаковка zlib
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QByteArray decompressed = decompressZlib(data.mid(sizeof(RscHeader)), sysSize + gfxSize);
    QApplication::restoreOverrideCursor();
    
    if (decompressed.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Не удалось распаковать YTD файл"); return; }
    
    QByteArray virtualData = decompressed.left(sysSize);
    
    QByteArray physicalData = decompressed.mid(sysSize, gfxSize);
    
    if (virtualData.size() < 0x40) { QMessageBox::warning(this, "Ошибка", "Некорректные данные"); return; }
    
    // количество текстур
    unsigned short count = *(unsigned short*)(virtualData.data() + 0x28);
    
    unsigned long long valuesPtr = *(unsigned long long*)(virtualData.data() + 0x30);  
    int valuesOff = (int)(valuesPtr - 0x50000000);
    
    m_ytdTextures.clear();
    m_ytdTextureList->clear();
    
    for (int i = 0; i < count; i++) {
        if (valuesOff + i * 8 + 8 > virtualData.size()) break;
        
        unsigned long long texPtr = *(unsigned long long*)(virtualData.data() + valuesOff + i * 8);
        int texOff = (int)(texPtr - 0x50000000);
        if (texOff + 0x80 > virtualData.size()) continue;
        
        TextureInfo tex;
        unsigned long long namePtr = *(unsigned long long*)(virtualData.data() + texOff + 0x28);
        int nameOff = (int)(namePtr - 0x50000000);
        tex.name = QString::fromUtf8(virtualData.data() + nameOff);
        tex.width = *(unsigned short*)(virtualData.data() + texOff + 0x50);
        tex.height = *(unsigned short*)(virtualData.data() + texOff + 0x52);
        tex.mipCount = *(unsigned char*)(virtualData.data() + texOff + 0x5D);
        tex.format = *(unsigned int*)(virtualData.data() + texOff + 0x58);
        
        unsigned long long dataPtr = *(unsigned long long*)(virtualData.data() + texOff + 0x70);
        unsigned int dataOff = (unsigned int)(dataPtr - 0x60000000);
        
        // считаем размер данных
        int dataSize = 0;
        int w = tex.width, h = tex.height;
        for (int m = 0; m < tex.mipCount; m++) {
            int bw = (w + 3) / 4;
            int bh = (h + 3) / 4;
            int blockSize = (tex.format == D3DFMT_DXT1) ? 8 : 16;
            dataSize += bw * bh * blockSize;
            w = (w > 1) ? w / 2 : 1;
            h = (h > 1) ? h / 2 : 1;
        }
        
        if (dataOff + dataSize <= (unsigned int)physicalData.size() && dataSize > 0) {
            QByteArray pixelData = physicalData.mid(dataOff, dataSize);
            
            // DDS
            DDS_HEADER header;
            memset(&header, 0, sizeof(header));
            header.magic = DDS_MAGIC;
            header.size = 124;
            header.flags = 0x81007;
            header.height = tex.height;
            header.width = tex.width;
            header.pitchOrLinearSize = pixelData.size();
            header.mipMapCount = 1;
            header.pfSize = 32;
            header.pfFlags = 0x4;
            header.fourCC = (tex.format == D3DFMT_DXT1) ? D3DFMT_DXT1 : D3DFMT_DXT5;
            header.caps = 0x1000;
            
            tex.data.append((char*)&header, sizeof(header));
            tex.data.append(pixelData);
            tex.isValid = true;
            m_ytdTextures.append(tex);
        }
    }
    
    if (m_ytdTextures.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Не найдено текстур"); return; }
    
    for (const auto& tex : m_ytdTextures) m_ytdTextureList->addItem(QString("%1 (%2x%3)").arg(tex.name).arg(tex.width).arg(tex.height));
    
    m_btnExportYtd->setEnabled(true);
    m_btnExportAllYtd->setEnabled(true);
    m_ytdInfoLabel->setText(QString("Загружено %1 текстур").arg(m_ytdTextures.size()));
}


// превью текстуры
void KoTeKTools::onYtdListSelectionChanged(int index)
{
    if (index < 0 || index >= m_ytdTextures.size()) return;
    
    const TextureInfo& tex = m_ytdTextures[index];
    if (!tex.isValid || tex.data.size() < sizeof(DDS_HEADER)) return;
    
    const DDS_HEADER* header = (const DDS_HEADER*)tex.data.constData();
    int width = header->width;
    int height = header->height;
    
    QByteArray compressed = tex.data.mid(sizeof(DDS_HEADER));
    QByteArray rgba;
    
    if (header->fourCC == D3DFMT_DXT1) rgba = decodeDXT1(compressed, width, height);
    else rgba = decodeDXT5(compressed, width, height);
    
    if (!rgba.isEmpty()) {
        QImage img((unsigned char*)rgba.data(), width, height, QImage::Format_RGBA8888);
        QPixmap pix = QPixmap::fromImage(img);
        if (!pix.isNull()) {
            m_ytdPreviewLabel->setPixmap(pix.scaled(m_ytdPreviewLabel->width() - 40, m_ytdPreviewLabel->height() - 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_ytdInfoLabel->setText(QString("%1 | %2x%3 | %4 | Mips: %5")
                .arg(tex.name).arg(width).arg(height)
                .arg(header->fourCC == D3DFMT_DXT1 ? "DXT1" : "DXT5").arg(tex.mipCount));
        }
    }
}

// сохранение текстуры
bool KoTeKTools::saveToPNG(const TextureInfo& tex, const QString& path)
{
    if (!tex.isValid || tex.data.size() < sizeof(DDS_HEADER)) return false;
    
    const DDS_HEADER* header = (const DDS_HEADER*)tex.data.constData();
    QByteArray compressed = tex.data.mid(sizeof(DDS_HEADER));
    QByteArray rgba;
    
    if (header->fourCC == D3DFMT_DXT1) rgba = decodeDXT1(compressed, header->width, header->height);
    else rgba = decodeDXT5(compressed, header->width, header->height);
    
    if (rgba.isEmpty()) return false;
    
    QImage img((unsigned char*)rgba.data(), header->width, header->height, QImage::Format_RGBA8888);
    return img.save(path, "PNG");
}

void KoTeKTools::onExportAllYtdToZip()
{
    if (m_ytdTextures.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет текстур для экспорта");
        return;
    }

    QString zipPath = QFileDialog::getSaveFileName(this, "Сохранить ZIP архив", 
        QFileInfo(m_ytdPath->text()).baseName() + "_textures.zip", 
        "ZIP Files (*.zip)");
    
    if (zipPath.isEmpty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Временная папка
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, "Ошибка", "Не удалось создать временную папку");
        return;
    }

    int exportedCount = 0;

    // Сохраняем каждую текстуру как PNG
    for (int i = 0; i < m_ytdTextures.size(); ++i) {
        const TextureInfo& tex = m_ytdTextures[i];
        if (!tex.isValid) continue;

        QString safeName = tex.name;
        safeName.replace(QRegularExpression("[<>:\"/\\|?*]"), "_");
        QString pngPath = tempDir.path() + "/" + safeName + ".png";

        if (saveToPNG(tex, pngPath)) {
            exportedCount++;
        }
    }

    if (exportedCount == 0) {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, "Ошибка", "Не удалось экспортировать ни одной текстуры");
        return;
    }

    // Создаем ZIP через libzip
    int error;
    zip_t *newZip = zip_open(zipPath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (newZip) {
        QDir dir(tempDir.path());
        QStringList files = dir.entryList(QStringList() << "*.png", QDir::Files);
        
        for (const QString& fileName : files) {
            QString filePath = tempDir.path() + "/" + fileName;
            zip_source_t *source = zip_source_file(newZip, filePath.toUtf8().constData(), 0, 0);
            if (source) {
                zip_file_add(newZip, fileName.toUtf8().constData(), source, ZIP_FL_OVERWRITE);
            }
        }
        zip_close(newZip);
    } else {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, "Ошибка", "Не удалось создать ZIP архив");
        return;
    }

    QApplication::restoreOverrideCursor();

    m_ytdInfoLabel->setText(QString("Экспортировано %1 текстур в ZIP: %2")
        .arg(exportedCount).arg(QFileInfo(zipPath).fileName()));

    QMessageBox::information(this, "Успех", 
        QString("Экспортировано %1 текстур в ZIP архив\n%2")
        .arg(exportedCount).arg(zipPath));
}

// экспорт текстуры
void KoTeKTools::onExportYtdTexture()
{
    int idx = m_ytdTextureList->currentRow();
    if (idx < 0) { QMessageBox::warning(this, "Ошибка", "Выберите текстуру для экспорта"); return; }
    
    QString path = QFileDialog::getSaveFileName(this, "Сохранить PNG", m_ytdTextures[idx].name + ".png", "PNG Files (*.png)");
    if (path.isEmpty()) return;
    
    if (saveToPNG(m_ytdTextures[idx], path)) m_ytdInfoLabel->setText("Сохранено: " + QFileInfo(path).fileName());
    else QMessageBox::warning(this, "Ошибка", "Не удалось сохранить PNG");
}

void KoTeKTools::onBrowseYtd()
{
    QString path = QFileDialog::getOpenFileName(this, "Выбрать YTD файл", "", "YTD Files (*.ytd)");
    if (!path.isEmpty()) { m_ytdPath->setText(path); loadYTD(path); }
}