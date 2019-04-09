//
// Created by richie on 7/6/17.
//

#ifndef MPLAYER_MP4EXTRACTORHELPER_H
#define MPLAYER_MP4EXTRACTORHELPER_H

#include "DataSource.h"

namespace peng {

#define FOURCC(c1, c2, c3, c4) \
    (c1 << 24 | c2 << 16 | c3 << 8 | c4)


typedef enum {
    BOX_FTYP = 0,
    BOX_MDAT,    BOX_FREE,    BOX_MOOV,    BOX_MVHD,    BOX_TRAK,
    BOX_UDTA,    BOX_TKHD,    BOX_TREF,    BOX_MDIA,    BOX_MDHD,
    BOX_HDLR,    BOX_MINF,    BOX_DINF,    BOX_STBL,    BOX_EDTS,
    BOX_SMHD,    BOX_VMHD,    BOX_HMHD,    BOX_NMHD,    BOX_STSD,
    BOX_MP4A,    BOX_MP4V,    BOX_ESDS,    BOX_STTS,    BOX_STSS,
    BOX_CTTS,    BOX_STSC,    BOX_STSZ,    BOX_STCO,    BOX_AVCC,
    BOX_HVCC,

    BOX_INVALID,
}BoxType;

BoxType fourccType(std::string name);
std::string fourccName(BoxType type);



void MakeFourCCString(uint32_t x, char *s);

uint16_t U16_AT(const uint8_t *ptr);
uint32_t U32_AT(const uint8_t *ptr);
uint64_t U64_AT(const uint8_t *ptr);

uint64_t ntoh64(uint64_t x);
uint64_t hton64(uint64_t x);
bool SniffMP4Help(const sp<DataSource> &source, std::string& mimeType);

}


#endif //MPLAYER_MP4EXTRACTORHELPER_H
