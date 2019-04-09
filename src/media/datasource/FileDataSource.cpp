//
// Created by richie on 7/6/17.
//

#include "FileDataSource.h"
#define LOG_TAG "FileDataSource"

#include "log.h"

namespace peng {

FileDataSource::FileDataSource (FILE *fp) : mFP(fp), mSize(0) {
    fseek (mFP, 0, SEEK_END);
    mSize = ftell (mFP);
    fseek (mFP, 0, SEEK_SET);
    LOGI("constructed");
}

FileDataSource::~FileDataSource () {
    LOGI("destroyed");
}


int FileDataSource::getSize (off64_t *size) {
    *size = mSize;
    return 0;
}

ssize_t FileDataSource::readAt (off64_t offset, void *data, size_t size) {
    fseek (mFP, offset, SEEK_SET);
    return fread (data, 1, size, mFP);
}


}