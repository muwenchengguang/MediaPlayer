//
// Created by richie on 7/6/17.
//

#ifndef MPLAYER_DATASOURCE_H
#define MPLAYER_DATASOURCE_H

#include "RefBase.h"
#include <list>
#include <string>
#include <stdint.h>
#include <sys/types.h>

namespace peng {

class DataSource : public RefBase {
public:
    DataSource() {}
    virtual ~DataSource() {};

    virtual ssize_t readAt(off64_t offset, void *data, size_t size) = 0;
    // Convenience methods:
    bool getUInt16(off64_t offset, uint16_t *x);
    bool getUInt24(off64_t offset, uint32_t *x); // 3 byte int, returned as a 32-bit int
    bool getUInt32(off64_t offset, uint32_t *x);

    // May return ERROR_UNSUPPORTED.
    virtual int getSize(off64_t *size);

    typedef bool (*SnifferFunc)(const sp<DataSource> &source, std::string& mimeType);

    static void RegisterDefaultSniffers();
    bool sniff(std::string& mimeType);

private:
    static std::list<SnifferFunc> gSniffers;

    static void RegisterSniffer_l(SnifferFunc func);
};

}

#endif //MPLAYER_DATASOURCE_H
