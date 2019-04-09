//
// Created by richie on 7/6/17.
//

#ifndef MPLAYER_FILEDATASOURCE_H
#define MPLAYER_FILEDATASOURCE_H

#include "DataSource.h"
#include "stdio.h"

namespace peng {

class FileDataSource : public DataSource {
public:
    FileDataSource (FILE *fp);

    virtual ~FileDataSource ();

    virtual int getSize (off64_t *size);
    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

private:
    FILE* mFP;
    off64_t mSize;

};

}

#endif //MPLAYER_FILEDATASOURCE_H
