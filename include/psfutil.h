#ifndef PSFUTIL_H
#define PSFUTIL_H

#include <cstdint>
#include <vector>
#include <QString>
#include <QImage>
#include <QMetaType>
#include "psf.h"

enum class FileType {
    MIF, // Verilog MIF
    PSF  // PSF File
};

namespace PSF {
    // Glyph utilities
    bool setGlyphFromText(PSFGlyph& glyph, const QString& txt);
    bool setGlyphFromImage(PSFGlyph& glyph, const QImage& img);
    QString glyphToHexString(const PSFGlyph& glyph);
    QImage glyphToImage(const PSFGlyph& glyph);

    // Verilog MIF utilities
    bool saveToVerilogMif(const PSFFont& font, const std::string& filename);
    bool loadFromVerilogMif(PSFFont& font, unsigned gw, unsigned gh, const std::string& filename);
}

Q_DECLARE_METATYPE(PSFGlyph*)

#endif // PSFUTIL_H
