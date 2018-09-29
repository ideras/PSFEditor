#include "psfutil.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

namespace PSF {

int loadFile(const char *filename, SymbInfo &symbInfo, std::vector<uint8_t>& rawFontData) {
    std::ifstream in(filename, std::ios::in);

    if (!in.is_open()) {
        return -1;
    }

    unsigned char magic_bytes[4];
    in.read(reinterpret_cast<char *>(magic_bytes), sizeof(magic_bytes));

    if (magic_bytes[0] == PSF1_MAGIC0 && magic_bytes[1] == PSF1_MAGIC1) {
        struct psf1_header header;

        in.seekg(0);
        in.read(reinterpret_cast<char *>(&header), sizeof(struct psf1_header));

        symbInfo.width = 8;
        symbInfo.height = header.charsize;
        symbInfo.sizeBytes = header.charsize;
        symbInfo.count = 256;

        int total_bytes = symbInfo.count * header.charsize;
        rawFontData.resize(total_bytes);

        in.read(reinterpret_cast<char *>(rawFontData.data()), total_bytes);
        int total_read = in.gcount();

        if (total_read != total_bytes) {
            in.close();
            return -2;
        }
    } else if (magic_bytes[0] == PSF2_MAGIC0 && magic_bytes[1] == PSF2_MAGIC1 &&
               magic_bytes[2] == PSF2_MAGIC2 && magic_bytes[3] == PSF2_MAGIC3) {
        struct psf2_header header;

        in.seekg(0);
        in.read(reinterpret_cast<char *>(&header), sizeof(struct psf2_header));

        symbInfo.width = header.width;
        symbInfo.height = header.height;
        symbInfo.sizeBytes = header.charsize;
        symbInfo.count = header.length;

        int total_bytes = header.length * header.charsize;
        rawFontData.resize(total_bytes);

        in.read(reinterpret_cast<char *>(rawFontData.data()), total_bytes);
        int total_read = in.gcount();

        if (total_read != total_bytes) {
            in.close();
            return -2;
        }
    } else {
        in.close();
        return -3;
    }

    in.close();
    return 0;
}

}
