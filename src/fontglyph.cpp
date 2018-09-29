#include "fontglyph.h"

bool FontGlyph::updateFromText(const QString &txt) {
    std::vector<uint32_t> indata;

    QStringList sl = txt.split('\n');
    foreach (QString s, sl) {
        bool ok;
        uint32_t val;

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
        indata.push_back(val);
    }
    int limit = qMin(height, indata.size());

    for (size_t ri = 0; ri < limit; ri++) {
        (*this)[ri] = indata[ri];
    }

    return true;
}

bool FontGlyph::updateFromImage(const QImage &img) {
    int w = qMin(img.width(), static_cast<int>(width));
    int h = qMin(img.height(),  static_cast<int>(height));

    for (int row = 0; row < h; row ++) {
        for (int col = 0; col < w; col++) {
            QRgb rgb = img.pixel(col, row);
            int a = qAlpha(rgb);
            int r = qRed(rgb);
            int g = qGreen(rgb);
            int b = qBlue(rgb);

            if (img.hasAlphaChannel()) {
                data[row][col] = a >= 128;
            } else {
                data[row][col] = ((r + g + b)/3) < 128;
            }
        }
    }

    return true;
}

QString FontGlyph::toHexString() const {
    QString result = "";

    int numdig = (width + 3) / 4;

    for (size_t ri = 0; ri < height; ri ++) {
        uint32_t rv = 0;
        for (size_t ci = 0, p = msb(); ci < width; p--, ci++) {
            if (data[ri][ci]) {
                rv |= 1 << p;
            }
        }
        result += QString("%1").arg(rv, numdig, 16, QChar('0')).toUpper() + '\n';
    }

    return result;
}

QString FontGlyph::toBinaryString() const {
    QString result = "0b";

    for (size_t ri = 0; ri < height; ri ++) {
        for (size_t ci = 0; ci < width; ci++) {
            result += data[ri][ci]? "1" : "0";
        }
        result += "\n";
    }

    return result;
}

QImage FontGlyph::toImage() const {
    QImage img(width, height, QImage::Format_RGB32);

    for (size_t ri = 0; ri < height; ri ++) {
        for (size_t ci = 0; ci < width; ci++) {
            if (data[ri][ci]) {
                img.setPixel(ci, ri, qRgb(0x0, 0x0, 0x0));
            } else {
                img.setPixel(ci, ri, qRgb(0xff, 0xff, 0xff));
            }
        }
    }

    return img;
}

FontGlyph::RowRef &FontGlyph::RowRef::operator=(uint32_t v) {
    ensureSize();

    for (size_t i = 0, p = fg.msb(); i<fg.width; i++, p--) {
        if ((v & (1 << p)) != 0) {
            fg.data[ri][i] = true;
        } else {
            fg.data[ri][i] = false;
        }
    }
    return *this;
}
