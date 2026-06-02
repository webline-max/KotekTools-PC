#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "txd.h"
#include <QDir>
#include <QFile>
#include <QApplication>
#include <QProcess>
#include <vector>
#include <cstring>
#include <algorithm>
#include <QDebug>
#include <QTemporaryDir>
#include <unordered_map>
#include <memory>
#include <zip.h>
#include <QDirIterator>

namespace Txd {

// таблицы для dxt распаковки
static constexpr uint8_t expand5[32] = {
    0,8,16,25,33,41,49,58,66,74,82,90,99,107,115,123,
    132,140,148,156,165,173,181,189,197,206,214,222,230,239,247,255
};

static constexpr uint8_t expand6[64] = {
    0,4,8,12,16,20,24,28,32,36,40,45,49,53,57,61,
    65,69,73,77,81,85,89,93,97,101,105,109,113,117,121,125,
    130,134,138,142,146,150,154,158,162,166,170,174,178,182,186,190,
    194,198,202,206,210,215,219,223,227,231,235,239,243,247,251,255
};

inline void decompressColour(uint8_t* rgba, const uint8_t* block) {
    uint16_t color0 = block[0] | (block[1] << 8);
    uint16_t color1 = block[2] | (block[3] << 8);
    
    int r0 = expand5[(color0 >> 11) & 0x1F];
    int g0 = expand6[(color0 >> 5) & 0x3F];
    int b0 = expand5[color0 & 0x1F];
    
    int r1 = expand5[(color1 >> 11) & 0x1F];
    int g1 = expand6[(color1 >> 5) & 0x3F];
    int b1 = expand5[color1 & 0x1F];
    
    uint8_t colors[4][4] = {
        {(uint8_t)r0, (uint8_t)g0, (uint8_t)b0, 255},
        {(uint8_t)r1, (uint8_t)g1, (uint8_t)b1, 255}
    };
    
    if (color0 > color1) {
        colors[2][0] = (2*r0 + r1) / 3;
        colors[2][1] = (2*g0 + g1) / 3;
        colors[2][2] = (2*b0 + b1) / 3;
        colors[2][3] = 255;
        colors[3][0] = (r0 + 2*r1) / 3;
        colors[3][1] = (g0 + 2*g1) / 3;
        colors[3][2] = (b0 + 2*b1) / 3;
        colors[3][3] = 255;
    } else {
        colors[2][0] = (r0 + r1) / 2;
        colors[2][1] = (g0 + g1) / 2;
        colors[2][2] = (b0 + b1) / 2;
        colors[2][3] = 255;
        colors[3][0] = 0;
        colors[3][1] = 0;
        colors[3][2] = 0;
        colors[3][3] = 0;  // прозрачный
    }
    
    uint32_t indices = block[4] | (block[5] << 8) | (block[6] << 16) | (block[7] << 24);
    
    for (int i = 0; i < 16; ++i) {
        int idx = (indices >> (2 * i)) & 3;
        rgba[4*i] = colors[idx][0];
        rgba[4*i+1] = colors[idx][1];
        rgba[4*i+2] = colors[idx][2];
        rgba[4*i+3] = colors[idx][3];
    }
}

 // dxt3
inline void decompressAlphaDXT3(uint8_t* rgba, const uint8_t* a) {
    for (int i = 0; i < 8; ++i) {
        rgba[8*i + 3] = (a[i] & 0xF) * 17;
        rgba[8*i + 8 + 3] = (a[i] >> 4) * 17;
    }
}

  // dxt5
inline void decompressAlphaDXT5(uint8_t* rgba, const uint8_t* a) {
    uint8_t a0 = a[0], a1 = a[1];
    uint8_t al[8];
    al[0] = a0;
    al[1] = a1;
    
    if (a0 > a1) {
        for (int i = 2; i < 8; ++i)
            al[i] = ((8-i) * a0 + (i-1) * a1 + 3) / 7;
    } else {
        for (int i = 2; i < 6; ++i)
            al[i] = ((6-i) * a0 + (i-1) * a1 + 2) / 5;
        al[6] = 0;
        al[7] = 255;
    }
    
    uint64_t bits = 0;
    for (int i = 0; i < 6; ++i)
        bits |= (uint64_t)a[2+i] << (8*i);
    
    for (int i = 0; i < 16; ++i)
        rgba[4*i+3] = al[(bits >> (3*i)) & 7];
}

inline void decompressDXT(const uint8_t* s, int w, int h, int fmt, uint8_t* d) {
    // распаковка dxt 4x4
    int bs = (fmt == 1) ? 8 : 16;
    int bx = (w + 3) / 4, by = (h + 3) / 4;
    
    for (int y = 0; y < by; ++y) {
        int yy = y * 4;
        for (int x = 0; x < bx; ++x) {
            const uint8_t* b = s + (y * bx + x) * bs;
            uint8_t r[64] = {};
            
            if (fmt == 1) {
                decompressColour(r, b);
            } else {
                if (fmt == 3) decompressAlphaDXT3(r, b);
                else decompressAlphaDXT5(r, b);
                
                uint8_t c[64];
                decompressColour(c, b + 8);
                for (int i = 0; i < 16; ++i) {
                    r[4*i] = c[4*i];
                    r[4*i+1] = c[4*i+1];
                    r[4*i+2] = c[4*i+2];
                }
            }
            
            int xx = x * 4;
            for (int py = 0; py < 4; ++py) {
                int cy = yy + py;
                if (cy >= h) break;
                
                for (int px = 0; px < 4; ++px) {
                    int cx = xx + px;
                    if (cx >= w) break;
                    
                    memcpy(d + (cy * w + cx) * 4, r + (py * 4 + px) * 4, 4);
                }
            }
        }
    }
}

inline uint32_t read32(const uint8_t*& d) { 
    uint32_t v = *(const uint32_t*)d; 
    d += 4; 
    return v; 
}

inline bool isDXT(uint32_t f) { 
    return f == 0x31545844 || f == 0x33545844 || f == 0x35545844; 
}

// bgra в rgba
static void bgraToRgba(const uint8_t* src, uint8_t* dst, int w, int h) {
    int total = w * h;
    for (int i = 0; i < total; ++i) { 
        int i4 = i * 4;
        dst[i4]   = src[i4 + 2];  // R
        dst[i4+1] = src[i4 + 1];  // G
        dst[i4+2] = src[i4];      // B
        dst[i4+3] = src[i4 + 3];  // A
    }
}

// bgrx в rgba
static void bgrxToRgba(const uint8_t* src, uint8_t* dst, int w, int h) {
    int total = w * h;
    for (int i = 0; i < total; ++i) { 
        int i4 = i * 4;
        dst[i4]   = src[i4 + 2];  // R
        dst[i4+1] = src[i4 + 1];  // G
        dst[i4+2] = src[i4];      // B
        dst[i4+3] = 255;          // A
    }
}

// палитра 256 цветов
static void pal8ToRgba(const uint8_t* src, const uint8_t* palette, int w, int h, uint8_t* dst) {
    int total = w * h;
    for (int i = 0; i < total; ++i) {
        int idx = src[i] * 4;
        int i4 = i * 4;
        dst[i4]   = palette[idx + 2];  // R
        dst[i4+1] = palette[idx + 1];  // G
        dst[i4+2] = palette[idx];      // B
        dst[i4+3] = 255;               // A
    }
}

// bgr в rgba
static void bgrToRgba(const uint8_t* src, uint8_t* dst, int w, int h) {
    int total = w * h;
    for (int i = 0; i < total; ++i) { 
        int i3 = i * 3;
        int i4 = i * 4;
        dst[i4]   = src[i3 + 2];
        dst[i4+1] = src[i3 + 1];
        dst[i4+2] = src[i3];
        dst[i4+3] = 255;
    }
}

// 8-бит в rgba
static void lum8ToRgba(const uint8_t* src, uint8_t* dst, int w, int h) {
    int total = w * h;
    for (int i = 0; i < total; ++i) { 
        int i4 = i * 4;
        dst[i4] = dst[i4+1] = dst[i4+2] = src[i];
        dst[i4+3] = 255;
    }
}

// rgba 4444 в rgba 8888
static void rgba4444ToRgba(const uint8_t* src, uint8_t* dst, int w, int h) {
    int total = w * h;
    for (int i = 0; i < total; ++i) {
        uint16_t p = src[i*2] | (src[i*2+1] << 8);
        int i4 = i * 4;
        dst[i4]   = ((p >> 8) & 0xF) * 17;
        dst[i4+1] = ((p >> 4) & 0xF) * 17;
        dst[i4+2] = (p & 0xF) * 17;
        dst[i4+3] = ((p >> 12) & 0xF) * 17;
    }
}

// rgb 565 в rgba 8888
static void rgb565ToRgba(const uint8_t* src, uint8_t* dst, int w, int h) {
    int total = w * h;
    for (int i = 0; i < total; ++i) {
        uint16_t p = src[i*2] | (src[i*2+1] << 8);
        int i4 = i * 4;
        dst[i4]   = expand5[(p >> 11) & 0x1F];
        dst[i4+1] = expand6[(p >> 5) & 0x3F];
        dst[i4+2] = expand5[p & 0x1F];
        dst[i4+3] = 255;
    }
}

QByteArray convertToZip(const QByteArray &txdData) {
    // парсинг txd
    const uint8_t* ptr = (const uint8_t*)txdData.constData();
    const uint8_t* end = ptr + txdData.size();
    
    // временная папка
    QTemporaryDir tmpDir(QApplication::applicationDirPath() + "/txd_temp_XXXXXX");
    if (!tmpDir.isValid()) return QByteArray();
    
    QString tmpPath = tmpDir.path();
    int texCount = 0;

    while (ptr + 12 <= end) {
        uint32_t chunkId = read32(ptr);
        uint32_t chunkSize = read32(ptr);
        read32(ptr);
        
        if (chunkId == 0x16) continue;
        
        if (chunkId == 0x15 && chunkSize >= 88) {
            const uint8_t* texStart = ptr;
            
            if (read32(ptr) != 0x01 || read32(ptr) < 88) {
                ptr = texStart + chunkSize;
                continue;
            }
            
            read32(ptr);
            ptr += 8;
            
            // имя текстуры
            char name[33] = {};
            memcpy(name, ptr, 32);
            name[32] = '\0';
            ptr += 64;
            
            read32(ptr);
            uint32_t fourCC = read32(ptr);  // формат
            
            uint16_t w = *(const uint16_t*)ptr; ptr += 2;
            uint16_t h = *(const uint16_t*)ptr; ptr += 2;
            ptr += 4;
            
            if (w == 0 || h == 0 || w > 4096 || h > 4096) {
                ptr = texStart + chunkSize;
                continue;
            }
            
            uint32_t dataSize = read32(ptr);
            const uint8_t* texData = ptr;
            ptr += dataSize;
            
            std::vector<uint8_t> rgba(w * h * 4);
            bool processed = false;

            // определение формата
            if (isDXT(fourCC)) {
                int fmt = (fourCC == 0x31545844) ? 1 : 
                         (fourCC == 0x33545844) ? 3 : 5;
                decompressDXT(texData, w, h, fmt, rgba.data());
                processed = true;
            }
            else if (fourCC == 0x0) {
                const int paletteSize = 256 * 4;
                if (dataSize >= w * h + paletteSize) {
                    const uint8_t* palette = texData;
                    const uint8_t* pixels = texData + paletteSize;
                    pal8ToRgba(pixels, palette, w, h, rgba.data());
                    processed = true;
                }
            }
            else if (dataSize == w * h * 4) {
                bool hasAlpha = false;
                int checkLimit = std::min(w * h, 100);
                for (int i = 0; i < checkLimit; ++i) {
                    if (texData[i*4+3] != 0 && texData[i*4+3] != 255) {
                        hasAlpha = true;
                        break;
                    }
                }
                
                if (hasAlpha) bgraToRgba(texData, rgba.data(), w, h);
                else bgrxToRgba(texData, rgba.data(), w, h);
                processed = true;
            }
            else if (dataSize == w * h * 3) {
                bgrToRgba(texData, rgba.data(), w, h);
                processed = true;
            }
            else if (dataSize == w * h * 2) {
                rgba4444ToRgba(texData, rgba.data(), w, h);
                bool hasAlpha = false;
                for (int i = 0; i < w * h; ++i) {
                    if (rgba[i * 4 + 3] != 255) {
                        hasAlpha = true;
                        break;
                    }
                }
                if (!hasAlpha) rgb565ToRgba(texData, rgba.data(), w, h);
                processed = true;
            }
            else if (dataSize == w * h) {
                lum8ToRgba(texData, rgba.data(), w, h);
                processed = true;
            }

            if (processed) {
                // сохранение png
                QString outFile = tmpPath + "/" + QString::fromLatin1(name) + ".png";
                stbi_write_png(outFile.toLocal8Bit().constData(), w, h, 4, 
                             rgba.data(), w * 4);
                ++texCount;
            }
            
            ptr = texStart + chunkSize;
        } else {
            ptr += chunkSize;
        }
    }
    
    if (texCount == 0) return QByteArray();
    
    // png в zip
    QString zipPath = tmpPath + "/result.zip";
    
    int error;
    zip_t *zip = zip_open(zipPath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (zip) {
        QDirIterator it(tmpPath, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QString filePath = it.filePath();
            QString fileName = QFileInfo(filePath).fileName();
            
            zip_source_t *source = zip_source_file(zip, filePath.toUtf8().constData(), 0, 0);
            if (source) {
                zip_file_add(zip, fileName.toUtf8().constData(), source, ZIP_FL_OVERWRITE);
            }
        }
        zip_close(zip);
    }
    
    QByteArray result;
    if (QFile::exists(zipPath)) {
        QFile zf(zipPath);
        if (zf.open(QIODevice::ReadOnly)) {
            result = zf.readAll();
            zf.close();
        }
    }
    
    return result;
}

}