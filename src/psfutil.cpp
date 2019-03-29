#include <iostream>
#include <fstream>
#include <QtGlobal>
#include "psfutil.h"

namespace PSF {

bool setGlyphFromText(PSFGlyph &glyph, const QString &txt) {
    unsigned w = glyph.getFont()->getWidth();
    unsigned y = 0;

    QStringList sl = txt.split('\n');
    foreach (QString s, sl) {
        bool ok;
        uint32_t val = 0;

        if (s.startsWith("0b")) {
            s = s.mid(2);
            val = s.toUInt(&ok, 2);
        } else if (s.startsWith("0x")) {
            s = s.mid(2);
            val = s.toUInt(&ok, 16);
        } else {
            ok = false;
        }

        if (!ok) {
            return false;
        }
        unsigned x = w - 1;
        for (unsigned i = 0; i < w; i++) {
            glyph.setPixel(x, y, (val & (1 << i)) != 0);
            x--;
        }
        y++;
    }
    return true;
}

bool setGlyphFromImage(PSFGlyph &glyph, const QImage &img) {
    int gw = static_cast<int>(glyph.getWidth());
    int gh = static_cast<int>(glyph.getHeight());
    int w = qMin(img.width(), gw);
    int h = qMin(img.height(), gh);

    for (int y = 0; y < h; y ++) {
        for (int x = 0; x < w; x++) {
            QRgb rgb = img.pixel(x, y);
            int a = qAlpha(rgb);
            int r = qRed(rgb);
            int g = qGreen(rgb);
            int b = qBlue(rgb);

            unsigned grey = ((r + g + b) / 3);
            unsigned pix = img.hasAlphaChannel()? ((a >= 128) && (grey < 128)) : (grey < 128);
            glyph.setPixel(x, y, pix);
        }
    }

    return true;
}

QString glyphToHexString(const PSFGlyph &glyph) {
    unsigned width = glyph.getWidth();
    unsigned height = glyph.getHeight();
    QString result = "";

    int dig = (width + 3) / 4;

    for (unsigned y = 0; y < height; y ++) {
        uint32_t rv = 0;
        for (unsigned x = 0; x < width; x++) {
            if (glyph.getPixel(x, y) != 0) {
                rv |= 1u << (width - (x + 1));
            }
        }
        result += QString("%1").arg(rv, dig, 16, QChar('0')).toUpper() + '\n';
    }

    return result;
}

QImage glyphToImage(const PSFGlyph &glyph) {
    unsigned width = glyph.getWidth();
    unsigned height = glyph.getHeight();
    QImage img(width, height, QImage::Format_RGB32);

    for (unsigned y = 0; y < height; y ++) {
        for (unsigned x = 0; x < width; x++) {
            if (glyph.getPixel(x, y)) {
                img.setPixel(x, y, qRgb(0x0, 0x0, 0x0));
            } else {
                img.setPixel(x, y, qRgb(0xff, 0xff, 0xff));
            }
        }
    }

    return img;
}

bool saveToVerilogMif(const PSFFont &font, const std::string& filename) {
    std::ofstream out(filename, std::ios::out | std::ios::trunc);

    if(!out.is_open()) {
        return false;
    }

    for (int i = 0; i < 256; i++) {
       out << glyphToHexString(font.getGlyph(i)).toStdString();
    }
    out.close();
    return true;
}

bool loadFromVerilogMif(PSFFont &font, int gw, int gh, const std::string &filename) {
    std::ifstream in(filename, std::ios::in|std::ios::binary);

    if(!in.is_open()) {
        return false;
    }

    int index = 0;
    int line = 1;

    font.init(PSFVersion::V1, gw, gh);

    while(!in.eof()) {
        PSFGlyph& glyph = font.getGlyph(index);
        std::string text;

        glyph.init(&font);
        for (int y = 0; (y < gh) && !in.eof(); y++) {
            std::getline(in, text);
            line++;
            unsigned val;

            try { val = std::stoi(text, nullptr, 16); }
            catch (std::invalid_argument& e) {
                std::cerr << "Invalid hex value '" << text << "' at line " << line << "\n";
                return false;
            }
            for (int x = 0; x < gw; x++) {
                unsigned pix = (val & (1 << (gw - x - 1))) != 0;
                if (!glyph.setPixel(x, y, pix)) {
                    std::cerr << "Glyph set pixel failed\n";
                    return false;
                }
            }
        }
        index++;
        if (index > 255) {
            std::cerr << "WARNING: Too many lines in file '" << filename << "'\n";
            break;
        }
    }
    in.close();

    if (index < 256) {
        std::cerr << "WARNING: Too few lines in file '" << filename << "'."
                  << " Expected 256 found " << index << '\n';
    }
    return true;
}

}
