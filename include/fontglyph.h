#ifndef FONTGLYPH_H
#define FONTGLYPH_H

#include <cstdint>
#include <vector>
#include <QImage>
#include <QMetaType>

class FontGlyph
{
public:
    using RowDataType = std::vector<bool>;

    class RowRef {
    public:
        RowRef(FontGlyph &fg, size_t ri): fg(fg), ri(ri) {}
        RowRef(const RowRef&) = default;

        RowRef& operator=(uint32_t v);

        std::vector<bool>::reference operator[](size_t ci) {
            ensureSize();
            return fg.data[ri][ci];
        }
    private:
        void ensureSize() {
            if (fg.data[ri].size() < fg.width) {
                fg.data[ri].resize(fg.width);
            }
        }

    private:
        FontGlyph &fg;
        size_t ri;
    };

    FontGlyph(int w, int h): width(w), height(h) {
        data.resize(height);

        for (auto& rd : data) {
            rd.resize(width);
        }
    }

    FontGlyph(FontGlyph&& rhs):
        width(rhs.width),
        height(rhs.height),
        data(std::move(rhs.data)) {}

    FontGlyph& operator =(FontGlyph&& rhs) {
        width = rhs.width;
        height = rhs.height;
        data = std::move(rhs.data);

        return *this;
    }

    RowRef operator[](size_t ri) { return RowRef(*this, ri); }
    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    QSize getSize() { return QSize(width, height); }
    size_t getTotalBytes() { return ((width + 7)/8) * height; }

    void clear() {
        for (size_t ri = 0; ri < height; ri++) {
            for (size_t ci = 0; ci < width; ci++) {
                data[ri][ci] = false;
            }
        }
    }

    bool updateFromText(const QString& txt);
    bool updateFromImage(const QImage& img);
    QString toHexString() const;
    QString toBinaryString() const;
    QImage toImage() const;

private:
    int msb() const { return ((((width + 7) / 8) * 8) - 1); }

private:
    size_t width;
    size_t height;
    std::vector<RowDataType> data;
};

Q_DECLARE_METATYPE(FontGlyph*)

#endif // FONTGLYPH_H
