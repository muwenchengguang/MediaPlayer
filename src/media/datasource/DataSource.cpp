//
// Created by richie on 7/6/17.
//

#include "DataSource.h"
#include "../core/MediaError.h"
#include <arpa/inet.h>
#include "mp4/MP4Extractor.h"

namespace peng {

std::list<DataSource::SnifferFunc> DataSource::gSniffers;

bool DataSource::getUInt16(off64_t offset, uint16_t *x) {
    *x = 0;

    uint8_t byte[2];
    if (readAt(offset, byte, 2) != 2) {
        return false;
    }

    *x = (byte[0] << 8) | byte[1];

    return true;
}

bool DataSource::getUInt24(off64_t offset, uint32_t *x) {
    *x = 0;

    uint8_t byte[3];
    if (readAt(offset, byte, 3) != 3) {
        return false;
    }

    *x = (byte[0] << 16) | (byte[1] << 8) | byte[2];

    return true;
}

bool DataSource::getUInt32(off64_t offset, uint32_t *x) {
    *x = 0;

    uint32_t tmp;
    if (readAt(offset, &tmp, 4) != 4) {
        return false;
    }

    *x = ntohl(tmp);

    return true;
}

int DataSource::getSize(off64_t *size) {
    *size = 0;

    return ERROR_UNSUPPORTED;
}

bool DataSource::sniff (std::string& mimeType) {
    mimeType = "";

    for (std::list<SnifferFunc>::iterator it = gSniffers.begin();
         it != gSniffers.end(); ++it) {
        std::string newMimeType;
        if ((*it)(this, newMimeType)) {
            mimeType = newMimeType;
            break;
        }
    }

    return mimeType.length () > 0;
}


void DataSource::RegisterDefaultSniffers () {
    RegisterSniffer_l(SniffMP4);
}

// static
void DataSource::RegisterSniffer_l(SnifferFunc func) {
    for (std::list<SnifferFunc>::iterator it = gSniffers.begin();
         it != gSniffers.end(); ++it) {
        if (*it == func) {
            return;
        }
    }

    gSniffers.push_back(func);
}

}
