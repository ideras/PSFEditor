/* psf.h
 *
 * a simple psf handling library.
 *
 * Ported to C++ by Ivan de Jesus Deras <ideras@gmail.com> 2019
 *
 * Gunnar Zötl <gz@tset.de> 2016
 * Released under the terms of the MIT license. See file LICENSE for details.
 *
 * info about the psf font file format(s) from
 * http://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html
 */

#ifndef psf_h
#define psf_h

#include <vector>
#include <fstream>

/* this first part is copied more or less verbatim from th above source */

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

/* representation of a single glyph, including unicode mapping information */

class PSFFont;

class PSFGlyph {
    friend class PSFFont;
public:
    PSFGlyph(): font(nullptr) {}

    /* init()
     *
     * initializes a glyph.
     *
     * Arguments:
     *   font the psf font this glyph belongs to.
     *   data the glyph bitmap data
     */
    void init(PSFFont *font, std::vector<unsigned char>&& data) {
        this->font = font;
        this->data = std::move(data);
    }

    /* init()
     *
     * initializes an empty glyph (one with all the pixels unset).
     *
     * Arguments:
     *   font the psf font this glyph belongs to.
     */
    void init(PSFFont *font);

    /* setPixel()
     *
     * sets a pixel in the glyph
     *
     * Arguments:
     *	x, y	coordinates of the pixel to set
     *	val		value to set the pixel to, 0 or 1
     *
     * Returns:
     *	true on success, false on failure.
     */
    bool setPixel(unsigned int x, unsigned int y, unsigned int val);

    /* getPixel()
     *
     * gets a pixel value from a glyph
     *
     * Arguments:
     *	x, y	coordinates of pixel to read
     *
     * Returns:
     *	1 if the pixel is set, 0 if not. Any pixels outside of the glyph
     *	dimensions are treated as unset.
     */
    unsigned int getPixel(unsigned int x, unsigned int y) const;

    /*
     * Return the container font of this glyph
     *
     * Returns:
     *	the container font of this glyph.
     */
    PSFFont *getFont() const { return font; }

    /*
     * Return the glyph unicode values
     *
     * Returns:
     *	the glyph unicode values
     */
    const std::vector<unsigned int>& getUnicodeValues() const { return unicode_vals; }

    /*
     * Return the glyph width
     *
     * Returns:
     *	the glyph width
     */
    unsigned int getWidth() const;

    /*
     * Return the glyph height
     *
     * Returns:
     *	the glyph height
     */
    unsigned int getHeight() const;

    /* addUnicodeVal
     *
     * adds a unicode value to a glyph. For a sequence, add PSF1_STARTSEQ and
     * then the unicode chars that make up the sequence. Note that if you add
     * a sequence, you can add more sequences but not more single unicode values.
     *
     * Arguments:
     *	uni		unicode value to add
     *
     * Returns:
     *	true on success, false on failure.
     */
    bool addUnicodeVal(unsigned int uni);

private:
    PSFFont *font; // The font containing the glyph
    std::vector<unsigned char> data;
    std::vector<unsigned int> unicode_vals;
};

enum class PSFVersion { V1, V2 };

/* representation of a complete psf font. */

class PSFFont {
    friend class PSFGlyph;
public:
    PSFFont() {}

    /*
     * Initializes a new psf font. Based upon the version,
     * the following applies:
     * 	if version==1:
     *		width must be 8
     *		charsize is precomputed as height
     *		space for 256 glyphs is preallocated
     *	if version==2:
     *		charsize is precomputed as ((width + 7) / 8) * height
     *		headersize and version fields are set to constant values (32 and 0)
     *		no space for glyphs is preallocated
     *
     * Arguments:
     *	version	psf version to create font file for
     *	width	width of char. Must be 8 for version 1 fonts
     *	height	height of char
     */
    void init(PSFVersion version, unsigned int width, unsigned int height);

    /*
     * Returns the width of the font
     *
     * Returns:
     *	the width of the font
     */
    unsigned int getWidth() {
        return ((version == PSFVersion::V1) ? 8 : header.psf2.width);
    }

    /*
     * Returns the height of the font
     *
     * Returns:
     *	the height of the font
     */
    unsigned int getHeight() {
        return ((version == PSFVersion::V1) ? header.psf1.charsize : header.psf2.height);
    }

    /*
     * Return the size in bytes of the glyph
     *
     * Returns:
     *	the size in bytes of the glyph
     */
    unsigned int getGlyphSize() const {
        return ((version == PSFVersion::V1) ? header.psf1.charsize : header.psf2.charsize);
    }

    /* getVersion()
     *
     * return the psf font version (1 or 2)
     *
     * Returns:
     *	the psf font version
     */
    PSFVersion getVersion() const { return version; }

    /* isVersion1()
     *
     * return true if the psf font uses version 1
     *
     * Returns:
     *	true if the psf font uses version 1, false otherwise
     */
    bool isVersion1() const { return version == PSFVersion::V1; }


    /* isVersion2()
     *
     * return true if the psf font uses version 2
     *
     * Returns:
     *	true if the psf font uses version 2, false otherwise
     */
    bool isVersion2() { return version == PSFVersion::V2; }

    /* loadFromFile()
     *
     * loads a psf font from a file handle
     *
     * Arguments:
     *	file	the file handle to load the font from
     *
     * Returns:
     *	a pointer to a psf_font structure containing the loaded font, or 0 on
     *	error.
     */
    bool loadFromFile(std::ifstream& file);

    /* loadFromFile()
     *
     * load a psf font from a file
     *
     * Arguments:
     *	filename	the name of the file to load the font from
     *
     * Returns:
     *	a pointer to a psf_font structure containing the loaded font, or 0 on
     *	error.
     */
    bool loadFromFile(const char *filename);

    /* saveToFile()
     *
     * saves a psf_font structure to a psf font file handle
     *
     * Arguments:
     *	file	the file handle to save to
     *
     * Returns:
     *	true on success, false on failure.
     */
    bool saveToFile(std::ofstream& file) const {
        if (version == PSFVersion::V1) {
            return psf1SaveToFile(file);
        } else {
            return psf2SaveToFile(file);
        }
    }

    /* saveToFile()
     *
     * saves a psf_font structure to a psf font file
     *
     * Arguments:
     *	filename	the name of the file to save to
     *
     * Returns:
     *	true on success, false on failure.
     */
    bool saveToFile(const char *filename) const;

    /* getGlyph
     *
     * returns a reference to a glyph within a psf font
     *
     *
     * Returns:
     *	a reference to a psf_glyph structure representing the <no>th glyph in the
     *	font. Throw an exception on error.
     */
    PSFGlyph& getGlyph(unsigned int no) {
        return glyphv[no]; // throw an exception if out of range
    }

    const PSFGlyph& getGlyph(unsigned int no) const {
        return glyphv[no]; // throw an exception if out of range
    }

    /* addGlyph
     *
     * initializes glyph number <no>, sees that it is available (if within the
     * valid range for glyph numbers for the font) and returns a reference to it.
     * If the glyph was in use prior to this call, it will be newly initialized.
     *
     * Arguments:
     *	no		the number of the glyph to initialize and return
     *
     * Returns:
     *	a reference to a psf_glyph structure representing the <no>th glyph in the
     *	font.  Throw an exception on error.
     */
    PSFGlyph& addGlyph(unsigned int no);

    /* getNumGlyphs()
     *
     * return the amount of glyphs in a psf font. Note that for psf1 fonts, this
     * number is always either 256 or 512.
     *
     * Returns:
     *	the number of glyphs in the font.
     */
    unsigned int getNumGlyphs() const {
        return glyphv.size();
    }

    /* hasUnicodeTable()
     *
     * checks whether a font has an unicode table
     *
     * Returns:
     *	true if the font has an unicode table, false if not.
     */
    bool hasUnicodeTable() {
        if (version == PSFVersion::V1) {
            return (header.psf1.mode & PSF1_MODEHASTAB) != 0;
        } else {
            return (header.psf2.flags & PSF2_HAS_UNICODE_TABLE) != 0;
        }
    }

private:
    bool readGlyphs(std::ifstream& file, unsigned int numglyphs, unsigned int glyphsize);
    bool psf1ReadUnicodeVals(std::ifstream& file, unsigned int numglyphs);
    bool psf1LoadFromFile(std::ifstream& file);
    bool psf1WriteUnicodeVals(std::ofstream& file) const;
    bool psf1SaveToFile(std::ofstream& file) const;
    bool psf2ReadRemainingFile(std::ifstream& file, std::vector<unsigned char>& data);
    bool psf2ReadUnicodeVals(std::ifstream& file, unsigned int numglyphs);
    bool psf2LoadFromFile(std::ifstream& file);
    bool psf2WriteUnicodeVals(std::ofstream& file) const;
    bool psf2SaveToFile(std::ofstream& file) const;
    bool writeDummy(std::ofstream& file, unsigned int size) const;
    bool writeGlyphs(std::ofstream& file) const;
    void resizeGlyphVector(unsigned int num);

private:
    PSFVersion version;

	union {
		struct psf1_header psf1;
		struct psf2_header psf2;
	} header;

    std::vector<PSFGlyph> glyphv;
};

#endif /* psf_h */
