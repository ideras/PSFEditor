/* psf.c
 *
 * a simple psf handling library.
 *
 * Gunnar Zötl <gz@tset.de> 2016
 * Released under the terms of the MIT license. See file LICENSE for details.
 *
 * info about the psf font file format(s) from
 * http://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <QDebug>
#include "psf.h"
#include "mini_utf8.h"

void PSFFont::init(PSFVersion version, unsigned int width, unsigned int height)
{
    this->version = version;

    if (version == PSFVersion::V1) {
        header.psf1.charsize = height;
        header.psf1.magic[0] = PSF1_MAGIC0;
        header.psf1.magic[1] = PSF1_MAGIC1;
        glyphv.resize(256);
    } else {
        header.psf2.width = width;
        header.psf2.height = height;
        header.psf2.charsize = ((width + 7) / 8) * height;
        header.psf2.magic[0] = PSF2_MAGIC0;
        header.psf2.magic[1] = PSF2_MAGIC1;
        header.psf2.magic[2] = PSF2_MAGIC2;
        header.psf2.magic[3] = PSF2_MAGIC3;
        header.psf2.version = 0;
        header.psf2.headersize = sizeof(struct psf2_header);
    }
}

static int psf_read_byte(std::ifstream& file, unsigned int *bval)
{
    int rd = file.get();
	if (rd == EOF) {
		perror(__func__);
		return 0;
	}
	*bval = (unsigned int) rd & 0xff;
	return 1;
}

static int psf_write_byte(std::ofstream& file, unsigned int bval)
{
    file.put(bval & 0xff);
    if (file.bad()) {
		perror(__func__);
		return 0;
	}
	return 1;
}

static int psf_read_word(std::ifstream& file, unsigned int *wval)
{
	unsigned int byte0, byte1;
	if (!psf_read_byte(file, &byte0) || !psf_read_byte(file, &byte1)) {
		return 0;
	}
	*wval = byte0 + (byte1 << 8);
	return 1;
}

static int psf_write_word(std::ofstream& file, unsigned int wval)
{
	unsigned int byte0, byte1;
	byte0 = wval & 0xff;
	byte1 = (wval >> 8) & 0xff;
	return psf_write_byte(file, byte0) && psf_write_byte(file, byte1);
}

static int psf_read_int(std::ifstream& file, unsigned int *ival)
{
	unsigned int byte0, byte1, byte2, byte3;
	if (!psf_read_byte(file, &byte0) || !psf_read_byte(file, &byte1) || !psf_read_byte(file, &byte2) || !psf_read_byte(file, &byte3)) {
		return 0;
	}
	*ival = byte0 + (byte1 << 8) + (byte2 << 16) + (byte3 << 24);
	return 1;
}

static int psf_write_int(std::ofstream& file, unsigned int ival)
{
	unsigned int byte0, byte1, byte2, byte3;
	byte0 = ival & 0xff;
	byte1 = (ival >> 8) & 0xff;
	byte2 = (ival >> 16) & 0xff;
	byte3 = (ival >> 24) & 0xff;
	return psf_write_byte(file, byte0) && psf_write_byte(file, byte1) && psf_write_byte(file, byte2) && psf_write_byte(file, byte3);
}

bool PSFFont::readGlyphs(std::ifstream &file, unsigned int numglyphs, unsigned int glyphsize)
{
    glyphv.resize(numglyphs);
    for (unsigned index = 0; index < numglyphs; ++index) {
        std::vector<unsigned char> gdata(glyphsize);
        file.read(reinterpret_cast<char *>(gdata.data()), glyphsize);

        if (file.gcount() != glyphsize) {
            return false;
        }
        glyphv[index].init(this, std::move(gdata));
    }
    return true;
}

bool PSFFont::psf1ReadUnicodeVals(std::ifstream &file, unsigned int numglyphs)
{
    for (unsigned i = 0; i < numglyphs; ++i) {
        PSFGlyph& glyph = glyphv[i];
		unsigned int ucval;
		while (1) {
            if (!psf_read_word(file, &ucval)) { return false; }
			if (ucval == PSF1_SEPARATOR) { break; }
            glyph.addUnicodeVal(ucval);
		}
	}
    return true;
}

bool PSFFont::psf1LoadFromFile(std::ifstream& file)
{
	unsigned char magic[2];
	magic[0] = PSF1_MAGIC0;
    file.read(reinterpret_cast<char *>(&magic[1]), 1);
    if (file.gcount() != 1) {
		perror(__func__);
        return false;
	}
	if (magic[1] != PSF1_MAGIC1) {
		fprintf(stderr, "%s: invalid magic number", __func__);
        return false;
	}
	unsigned int mode, height;
	if (!psf_read_byte(file, &mode)) { return 0; }
	if (!psf_read_byte(file, &height)) { return 0; }

    init(PSFVersion::V1, 8, height);
    header.psf1.mode = static_cast<unsigned char>(mode);

	int numglyphs = (mode & PSF1_MODE512) ? 512 : 256;
    if (!readGlyphs(file, numglyphs, height)) {
        return false;
	}

    if (hasUnicodeTable() && !psf1ReadUnicodeVals(file, numglyphs)) {
        return false;
	}

    return true;
}

bool PSFFont::psf2ReadRemainingFile(std::ifstream &file, std::vector<unsigned char> &datav)
{
    long pos1 = file.tellg();
    file.seekg(0, file.end);
    long pos2 = file.tellg();
    long length = pos2 - pos1;
    file.seekg(pos1, file.beg);

    datav.resize(length);
    file.read(reinterpret_cast<char *>(datav.data()), length);

    return (file.gcount() == length);
}

bool PSFFont::psf2ReadUnicodeVals(std::ifstream &file, unsigned int numglyphs)
{
    std::vector<unsigned char> udata;

    if (!psf2ReadRemainingFile(file, udata)) {
        return false;
    }

    unsigned int size = udata.size();
    unsigned char *ptr = udata.data(), *end = udata.data() + size;
    for (unsigned i = 0; i < numglyphs; ++i) {
        PSFGlyph& glyph = glyphv[i];
		int ucval;
		while (1) {
			if (ptr >= end) {
				fprintf(stderr, "%s: unexpected end of file\n", __func__);
				return 0;
			}
			if (*ptr == PSF2_SEPARATOR) { ++ptr; break; }
			if (*ptr == PSF2_STARTSEQ) {
				++ptr;
				ucval = PSF1_STARTSEQ;
			} else {
                ucval = mini_utf8_decode(const_cast<const char **>(reinterpret_cast<char **>(&ptr)));
				if (ucval < 0) {
					fprintf(stderr, "%s: invalid utf8 char\n", __func__);
					return 0;
				}
			}
            glyph.addUnicodeVal(static_cast<unsigned>(ucval) & 0x1FFFFF);
		}
	}

	return 1;
}

bool PSFFont::psf2LoadFromFile(std::ifstream& file)
{
	unsigned char magic[4];
	magic[0] = PSF2_MAGIC0;
    file.read(reinterpret_cast<char *>(&magic[1]), 3);
    if (file.gcount() != 3) {
		perror(__func__);
		return false;
	}
	if (magic[0] != PSF2_MAGIC0 || magic[1] != PSF2_MAGIC1 || magic[2] != PSF2_MAGIC2 || magic[3] != PSF2_MAGIC3) {
		fprintf(stderr, "%s: invalid magic number", __func__);
		return false;
	}

	unsigned int version, headersize, flags, length, charsize, width, height;
	if (!psf_read_int(file, &version)) { return false; }
	if (!psf_read_int(file, &headersize)) { return false; }
	if (!psf_read_int(file, &flags)) { return false; }
	if (!psf_read_int(file, &length)) { return false; }
	if (!psf_read_int(file, &charsize)) { return false; }
	if (!psf_read_int(file, &height)) { return false; }
	if (!psf_read_int(file, &width)) { return false; }

    init(PSFVersion::V2, width, height);
    header.psf2.version = version;
    header.psf2.headersize = headersize;
    header.psf2.flags = flags;
    header.psf2.length = length;
    header.psf2.charsize = charsize;
    header.psf2.width = width;
    header.psf2.height = height;

    if (!readGlyphs(file, length, charsize)) {
        return false;
	}

    if (hasUnicodeTable() && !psf2ReadUnicodeVals(file, length)) {
        return false;
	}

    return true;
}

bool PSFFont::loadFromFile(std::ifstream &file)
{
    int byte = file.get();
	if (byte == PSF1_MAGIC0) {
        return psf1LoadFromFile(file);
	} else if (byte == PSF2_MAGIC0) {
        return psf2LoadFromFile(file);
	} else {
		fprintf(stderr, "%s: invalid magic number", __func__);
	}
    return false;
}

bool PSFFont::loadFromFile(const char *filename)
{
    std::ifstream file(filename, std::ios::in|std::ios::binary);
    if (!file.is_open()) {
		perror(__func__);
        return false;
	}
    return loadFromFile(file);
}

bool PSFFont::writeDummy(std::ofstream &file, unsigned int size) const
{
    for (unsigned i = 0; i < size; ++i) {
        file.put('\0');
        if (file.bad()) { return false; }
	}
    return true;
}

bool PSFFont::writeGlyphs(std::ofstream &file) const
{
    unsigned int numglyphs = glyphv.size();
    unsigned int glyphsize = getGlyphSize();

    for (unsigned i = 0; i < numglyphs; ++i) {
        const PSFGlyph& glyph = glyphv[i];

        if (glyph.data.size() > 0) {
            file.write(reinterpret_cast<const char *>(glyph.data.data()), glyphsize);
            if (file.bad()) {
                perror(__func__);
                return 0;
            }
        } else {
            if (!writeDummy(file, glyphsize)) {
                perror(__func__);
                return 0;
            }
        }
    }
    return 1;
}

bool PSFFont::psf1WriteUnicodeVals(std::ofstream &file) const
{
    for (unsigned i = 0; i < glyphv.size(); ++i) {
        for (unsigned ucv = 0; ucv < glyphv[i].unicode_vals.size(); ++ucv) {
            if (!psf_write_word(file, glyphv[i].unicode_vals[ucv])) { return false; }
        }
        if (!psf_write_word(file, PSF1_SEPARATOR)) { return false; }
    }
    return true;
}

bool PSFFont::psf1SaveToFile(std::ofstream &file) const
{
    if (!psf_write_byte(file, header.psf1.magic[0])) { return false; }
    if (!psf_write_byte(file, header.psf1.magic[1])) { return false; }
    if (!psf_write_byte(file, header.psf1.mode)) { return false; }
    if (!psf_write_byte(file, header.psf1.charsize)) { return false; }

    if (!writeGlyphs(file)) {
		return 0;
	}

    if (header.psf1.mode & (PSF1_MODEHASTAB | PSF1_MODEHASSEQ)) {
        if (!psf1WriteUnicodeVals(file)) {
			return 0;
		}
	}
	return 1;
}

bool PSFFont::psf2WriteUnicodeVals(std::ofstream &file) const
{
	char u8buf[8];
    unsigned int numglyphs = glyphv.size();

    for (unsigned i = 0; i < numglyphs; ++i) {
        for (unsigned ucv = 0; ucv < glyphv[i].unicode_vals.size(); ++ucv) {
            if (glyphv[i].unicode_vals[ucv] == PSF1_STARTSEQ) {
                if (!psf_write_byte(file, PSF2_STARTSEQ)) { return false; }
			} else {
                int len = mini_utf8_encode(glyphv[i].unicode_vals[ucv], u8buf, 8);
				if (len <= 0) {
					fprintf(stderr, "%s: invalid unicode value\n", __func__);
                    return false;
				}
                file.write(u8buf, len);
                if (file.bad()) {
					perror(__func__);
                    return false;
				}
			}
		}
		if (!psf_write_byte(file, PSF2_SEPARATOR)) { return 0; }
	}
    return true;
}

bool PSFFont::psf2SaveToFile(std::ofstream &file) const
{
    for (int i = 0; i < 4; ++i) {
        if (!psf_write_byte(file, header.psf2.magic[i])) { return false; }
    }
    if (!psf_write_int(file, header.psf2.version)) { return false; }
    if (!psf_write_int(file, header.psf2.headersize)) { return false; }
    if (!psf_write_int(file, header.psf2.flags)) { return false; }
    if (!psf_write_int(file, header.psf2.length)) { return false; }
    if (!psf_write_int(file, header.psf2.charsize)) { return false; }
    if (!psf_write_int(file, header.psf2.height)) { return false; }
    if (!psf_write_int(file, header.psf2.width)) { return false; }

    if (!writeGlyphs(file)) {
        return false;
    }

    if (header.psf2.flags & PSF2_HAS_UNICODE_TABLE) {
        if (!psf2WriteUnicodeVals(file)) {
            return false;
        }
    }

    return true;
}

bool PSFFont::saveToFile(const char *filename) const
{
    std::ofstream file(filename, std::ios::out|std::ios::binary);
    if (!file.is_open()) {
		perror(__func__);
		return 0;
	}
    return saveToFile(file);
}

void PSFFont::resizeGlyphVector(unsigned int num)
{
    unsigned int ng = getNumGlyphs();

    if (version == PSFVersion::V1) {
		if (num > 512) {
            throw std::out_of_range("Invalid size for PSF Version 1. No more than 512 chars are allowed\n");
		}
		unsigned int nng = num <= 256 ? 256 : 512;
		if (nng > ng) {
            glyphv.resize(nng);
			if (nng == 512) {
                header.psf1.mode |= PSF1_MODE512;
            }
		}
	} else {
		if (num > ng) {
            glyphv.resize(num);
            header.psf2.length = num;
		}
    }
}

PSFGlyph& PSFFont::addGlyph(unsigned int no)
{
    if (no >= getNumGlyphs()) {
        resizeGlyphVector(no + 1);
	}
    glyphv[no].init(this);
    return glyphv[no];
}

void PSFGlyph::init(PSFFont *font)
{
    this->font = font;
    data.clear();
    data.resize(font->getGlyphSize());
}

bool PSFGlyph::setPixel(unsigned int x, unsigned int y, unsigned int val)
{
    if (data.empty()) { return false; }
    unsigned int w = font->getWidth();
    unsigned int h = font->getHeight();
    if (x >= w || y >= h) { return false; }
	unsigned int byte = y * ((w + 7) >> 3) + (x >> 3);
	unsigned int mask = 0x80 >> (x & 7);
	if (val) {
        data[byte] |= mask;
	} else {
        data[byte] &= ~mask;
	}
    return true;
}

unsigned int PSFGlyph::getPixel(unsigned int x, unsigned int y) const
{
    if (data.empty()) { return false; }
    unsigned int w = font->getWidth();
    unsigned int h = font->getHeight();
	if (x >= w || y >= h) { return 0; }
	unsigned int byte = y * ((w + 7) >> 3) + (x >> 3);
	unsigned int mask = 0x80 >> (x & 7);
    return (data[byte] & mask) != 0;
}

unsigned int PSFGlyph::getWidth() const
{
    return font->getWidth();
}

unsigned int PSFGlyph::getHeight() const
{
    return font->getHeight();
}

bool PSFGlyph::addUnicodeVal(unsigned int uni)
{
    if (font->isVersion1() && uni > 0xFFFF) {
		fprintf(stderr, "%s: unicode value too big for psf1\n", __func__);
        return false;
	}
    unicode_vals.push_back(uni);
    if (font->isVersion1()) {
        font->header.psf1.mode |= PSF1_MODEHASTAB;
		if (uni == PSF1_STARTSEQ) {
            font->header.psf1.mode |= PSF1_MODEHASSEQ;
		}
	} else {
        font->header.psf2.flags |= PSF2_HAS_UNICODE_TABLE;
	}

    return true;
}
