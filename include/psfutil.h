#ifndef PSFUTIL_H
#define PSFUTIL_H

#include <cstdint>
#include <vector>

/* PSF 1 format */
#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

#define PSF1_SEPARATOR  0xFFFF
#define PSF1_STARTSEQ   0xFFFE

struct psf1_header {
    unsigned char magic[2];     /* Magic number */
    unsigned char mode;         /* PSF font mode */
    unsigned char charsize;     /* Character size */
};

/* PSF 2 format */
#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE 0x01

/* max version recognized so far */
#define PSF2_MAXVERSION 0

/* UTF8 separators */
#define PSF2_SEPARATOR  0xFF
#define PSF2_STARTSEQ   0xFE

struct psf2_header {
    unsigned char magic[4];
    unsigned int version;
    unsigned int headersize;    /* offset of bitmaps in file */
    unsigned int flags;
    unsigned int length;        /* number of glyphs */
    unsigned int charsize;      /* number of bytes for each character */
    unsigned int height, width; /* max dimensions of glyphs */
    /* charsize = height * ((width + 7) / 8) */
};

namespace PSF {
    // Every char bitmap is width x height pixels
    struct SymbInfo {
        size_t width;  // pixels
        size_t height; // pixels
        size_t sizeBytes;   // Size in bytes
        size_t count;   // Number of symbols
    };

    int loadFile(const char *filename, SymbInfo &symbInfo, std::vector<uint8_t> &rawFontData);
}
#endif // PSFUTIL_H
