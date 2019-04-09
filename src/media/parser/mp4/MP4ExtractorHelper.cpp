//
// Created by richie on 7/6/17.
//

#include "MP4ExtractorHelper.h"
#include "MediaDefs.h"
#include <arpa/inet.h>
#include <string.h>

#define LOG_TAG "MP4Extractor"
#include "log.h"

namespace peng {

void MakeFourCCString(uint32_t x, char *s) {
    s[0] = x >> 24;
    s[1] = (x >> 16) & 0xff;
    s[2] = (x >> 8) & 0xff;
    s[3] = x & 0xff;
    s[4] = '\0';
}

// XXX warning: these won't work on big-endian host.
uint64_t ntoh64(uint64_t x) {
    return ((uint64_t)ntohl(x & 0xffffffff) << 32) | ntohl(x >> 32);
}

uint64_t hton64(uint64_t x) {
    return ((uint64_t)htonl(x & 0xffffffff) << 32) | htonl(x >> 32);
}


uint16_t U16_AT(const uint8_t *ptr) {
    return ptr[0] << 8 | ptr[1];
}

uint32_t U32_AT(const uint8_t *ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

uint64_t U64_AT(const uint8_t *ptr) {
    return ((uint64_t)U32_AT(ptr)) << 32 | U32_AT(ptr + 4);
}

static bool isCompatibleBrand(uint32_t fourcc) {
    static const uint32_t kCompatibleBrands[] = {
            FOURCC('i', 's', 'o', 'm'),
            FOURCC('i', 's', 'o', '2'),
            FOURCC('a', 'v', 'c', '1'),
            FOURCC('3', 'g', 'p', '4'),
            FOURCC('m', 'p', '4', '1'),
            FOURCC('m', 'p', '4', '2'),

            // Won't promise that the following file types can be played.
            // Just give these file types a chance.
            FOURCC('q', 't', ' ', ' '),  // Apple's QuickTime
            FOURCC('M', 'S', 'N', 'V'),  // Sony's PSP

            FOURCC('3', 'g', '2', 'a'),  // 3GPP2
            FOURCC('3', 'g', '2', 'b'),
    };

    for (size_t i = 0;
         i < sizeof(kCompatibleBrands) / sizeof(kCompatibleBrands[0]);
         ++i) {
        if (kCompatibleBrands[i] == fourcc) {
            return true;
        }
    }

    return false;
}

static bool BetterSniffMPEG4(const sp<DataSource> &source, std::string& mimeType) {
    // We scan up to 128 bytes to identify this file as an MP4.
    static const off64_t kMaxScanOffset = 128ll;

    long offset = 0l;
    bool foundGoodFileType = false;
    long moovAtomEndOffset = -1l;
    bool done = false;

    while (!done && offset < kMaxScanOffset) {
        uint32_t hdr[2];
        if (source->readAt(offset, hdr, 8) < 8) {
            return false;
        }

        uint64_t chunkSize = ntohl(hdr[0]);
        uint32_t chunkType = ntohl(hdr[1]);
        off64_t chunkDataOffset = offset + 8;

        if (chunkSize == 1) {
            if (source->readAt(offset + 8, &chunkSize, 8) < 8) {
                return false;
            }

            chunkSize = ntoh64(chunkSize);
            chunkDataOffset += 8;

            if (chunkSize < 16) {
                // The smallest valid chunk is 16 bytes long in this case.
                return false;
            }
        } else if (chunkSize < 8) {
            // The smallest valid chunk is 8 bytes long.
            return false;
        }

        off64_t chunkDataSize = offset + chunkSize - chunkDataOffset;

        char chunkstring[5];
        MakeFourCCString(chunkType, chunkstring);
        LOGI("saw chunk type %s, size %lld @ %ld", chunkstring, chunkSize, offset);
        switch (chunkType) {
            case FOURCC('f', 't', 'y', 'p'):
            {
                if (chunkDataSize < 8) {
                    return false;
                }

                uint32_t numCompatibleBrands = (chunkDataSize - 8) / 4;
                for (size_t i = 0; i < numCompatibleBrands + 2; ++i) {
                    if (i == 1) {
                        // Skip this index, it refers to the minorVersion,
                        // not a brand.
                        continue;
                    }

                    uint32_t brand;
                    if (source->readAt(
                            chunkDataOffset + 4 * i, &brand, 4) < 4) {
                        return false;
                    }

                    brand = ntohl(brand);

                    if (isCompatibleBrand(brand)) {
                        foundGoodFileType = true;
                        break;
                    }
                }

                if (!foundGoodFileType) {
                    return false;
                }

                break;
            }

            case FOURCC('m', 'o', 'o', 'v'):
            {
                moovAtomEndOffset = offset + chunkSize;

                done = true;
                break;
            }

            default:
                break;
        }

        offset += chunkSize;
    }

    if (!foundGoodFileType) {
        return false;
    }

    mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG4;

    return true;
}

static bool LegacySniffMPEG4(
        const sp<DataSource> &source, std::string& mimeType) {
    uint8_t header[8];

    ssize_t n = source->readAt(4, header, sizeof(header));
    if (n < (ssize_t)sizeof(header)) {
        return false;
    }

    if (!memcmp(header, "ftyp3gp", 7) || !memcmp(header, "ftypmp42", 8)
        || !memcmp(header, "ftyp3gr6", 8) || !memcmp(header, "ftyp3gs6", 8)
        || !memcmp(header, "ftyp3ge6", 8) || !memcmp(header, "ftyp3gg6", 8)
        || !memcmp(header, "ftypisom", 8) || !memcmp(header, "ftypM4V ", 8)
        || !memcmp(header, "ftypM4A ", 8) || !memcmp(header, "ftypf4v ", 8)
        || !memcmp(header, "ftypkddi", 8) || !memcmp(header, "ftypM4VP", 8)) {
        mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG4;

        return true;
    }

    return false;
}

bool SniffMP4Help(const sp<DataSource> &source, std::string& mimeType) {
    if (BetterSniffMPEG4(source, mimeType)) {
        return true;
    }

    if (LegacySniffMPEG4(source, mimeType)) {
        LOGW("Identified supported mpeg4 through LegacySniffMPEG4.");
        return true;
    }
    return false;
}


typedef struct {
    BoxType type;
    const char* name;
}FourccRefItem;

static const FourccRefItem sFourccRefItem[] {
        {BOX_FTYP, "ftyp"},    {BOX_MDAT, "mdat"},    {BOX_FREE, "free"},
        {BOX_MOOV, "moov"},    {BOX_MVHD, "mvhd"},    {BOX_TRAK, "trak"},
        {BOX_UDTA, "udta"},    {BOX_TKHD, "tkhd"},    {BOX_TREF, "tref"},
        {BOX_MDIA, "mdia"},    {BOX_MDHD, "mdhd"},    {BOX_HDLR, "hdlr"},
        {BOX_MINF, "minf"},    {BOX_DINF, "dinf"},    {BOX_STBL, "stbl"},
        {BOX_EDTS, "edts"},    {BOX_SMHD, "smhd"},    {BOX_VMHD, "vmhd"},
        {BOX_HMHD, "hmhd"},    {BOX_NMHD, "nmhd"},    {BOX_STSD, "stsd"},
        {BOX_MP4A, "mp4a"},    {BOX_MP4V, "mp4v"},    {BOX_ESDS, "esds"},
        {BOX_STTS, "stts"},    {BOX_STSS, "stss"},    {BOX_CTTS, "ctts"},
        {BOX_STSC, "stsc"},    {BOX_STSZ, "stsz"},    {BOX_STCO, "stco"},
        {BOX_AVCC, "avcC"},    {BOX_HVCC, "hvcC"},
};


BoxType fourccType(std::string name) {
    for (int i = 0; i < sizeof(sFourccRefItem)/sizeof(FourccRefItem); i++) {
        if (name.compare(sFourccRefItem[i].name) == 0) {
            return sFourccRefItem[i].type;
        }
    }
    return BOX_INVALID;
}

std::string fourccName(BoxType type) {
    for (int i = 0; i < sizeof(sFourccRefItem)/sizeof(FourccRefItem); i++) {
        if (type == sFourccRefItem[i].type) {
            return sFourccRefItem[i].name;
        }
    }
    return "";
}

}
