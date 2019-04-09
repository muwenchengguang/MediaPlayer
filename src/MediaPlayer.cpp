#include "MessageQueue.h"
#include <string>
#include "RefBase.h"
#define LOG_TAG "TEST"
#include "log.h"

#include "FileDataSource.h"
#include "MediaExtractor.h"

using namespace peng;

static void test_ds() {
    LOGI("test ds");
    FILE* fp = fopen ("/sdcard/video.mp4", "rb");
    if (fp <= 0) {
        LOGI("open file failed");
        return;
    }
    sp<DataSource> ds = new FileDataSource(fp);
    off64_t size = 0;
    ds->getSize(&size);
    LOGI("file size:%lld", size);
}

static void save2File(sp<MediaExtractor>& extractor) {
    size_t count = extractor->countTracks();
    for (size_t i = 0; i < count; ++i) {
        sp<MediaSource> track = extractor->getTrack(i);
        if (track.get() != NULL) {
            char path[256] = {0};
            sprintf(path, "/sdcard/%d.264", i);
            FILE* fp = fopen(path, "wb");
            int ret;
            MediaBuffer* buffer = NULL;
            int count = 0;
            do {
                ret = track->read(&buffer);
                if (buffer != NULL) {
                    //LOGI("offset:%d-%d", buffer->offset(), buffer->size());
                }
                if (fp > 0 && buffer != NULL) {
                    //LOGI("buffer size:%d", buffer->size());
                    int writes = fwrite(buffer->data(), 1, buffer->size(), fp);
                    if (writes > 0) {
                        count += writes;
                        //LOGI("write %d bytes", count);
                    }
                }
                if (buffer != NULL) {
                    delete buffer;
                    buffer = NULL;
                }
            } while (ret >= 0);
            if (fp > 0) {
                LOGI("save to file");
                fclose(fp);
            } else {
                LOGE("save to file failed");
            }
        }
    }
}

void test_parser(char* filename) {
    DataSource::RegisterDefaultSniffers();
    FILE* fp = fopen (filename, "rb");
    if (fp <= 0) {
        LOGE("open file failed");
        return;
    }
    sp<DataSource> ds = new FileDataSource(fp);
    sp<MediaExtractor> extractor = MediaExtractor::create(ds, NULL);
    if (extractor.get() != NULL) {
        LOGI("parsed done");
    }
    LOGI("content:%s", extractor->toString().c_str());
    //save2File(extractor);
}

int main(int argc,char *argv[]) {

    LOGI("IN MAIN");
    //test_ds();
    test_parser(argv[1]);
    LOGI("OUT MAIN");
    return 0;
}
