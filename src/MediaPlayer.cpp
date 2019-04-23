#include "MessageQueue.h"
#include <string>
#include "RefBase.h"
#define LOG_TAG "TEST"
#include "log.h"

#include "FileDataSource.h"
#include "MediaExtractor.h"
#include "mp4/track.h"
#include "media/decoder/ffmpeg_decoder.h"
#include "display/Render.h"
#include <unistd.h>
#include <Player.h>

#include <gtk/gtk.h>
#include <wnd.h>

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
            sprintf(path, "%d.264", i);
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

void test_decoder(char* filename) {
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
	size_t count = extractor->countTracks();
	for (size_t i = 0; i < count; ++i) {
		sp<MediaSource> track = extractor->getTrack(i);
		if (track.get() != NULL) {
			int codecType = Unknown;
			sp<MetaData> format = track->getFormat();
			format->findInt32(kKeyMIMEType, codecType);
			MediaBuffer* buffer = NULL;
			sp<FFMPEGVideoDecoder> decoder = NULL;
			if (codecType == AVC) {
				decoder = new FFMPEGVideoDecoder(codecType, track);
			} else {
				LOGW("ignore unsupported codec");
				continue;
			}
			int ret;
			do {
				ret = decoder->read(&buffer);
				if (ret == ERROR_END_OF_STREAM) {
					LOGE("end of stream");
				}
				if (buffer != NULL) {
					delete buffer;
					buffer = NULL;
				}
			} while (ret >= 0);
		}
	}
}

void testPlay(char* filename) {
	DataSource::RegisterDefaultSniffers();
	FILE* fp = fopen (filename, "rb");
	if (fp <= 0) {
		LOGE("open file failed");
		return;
	}
	sp<Player> player = new Player();
	player->setDataSource(fp);
	player->start();
	while (player->isPlaying()) {
	    usleep(1000*1000);
	}
	player->stop();
}

class ThreadProc: public SingleThread::ThreadProc {
public:
    ThreadProc() {
    }
    ~ThreadProc() {
    }
    virtual bool process(int id) {

        while (gtk_events_pending())
            gtk_main_iteration();
        usleep(10*1000);
        return true;
    }

private:
};

void play(char* filename, GtkMainWnd& wnd) {

    DataSource::RegisterDefaultSniffers();
    FILE* fp = fopen (filename, "rb");
    if (fp <= 0) {
        LOGE("open file failed");
        return;
    }
    sp<Player> player = new Player();
    player->setSurface(&wnd);
    player->setDataSource(fp);
    player->start();
    while (player->isPlaying()) {
        usleep(1000*1000);
    }
    player->stop();
}

int main(int argc,char *argv[]) {
    gtk_init(&argc, &argv);
    g_type_init();
    g_thread_init(NULL);

    SingleThread thread;
    ThreadProc proc;
    thread.registerThreadProc(proc);
    thread.start();

    GtkMainWnd wnd;


    LOGI("IN MAIN");
    play(argv[1], wnd);

    thread.stop();
    LOGI("OUT MAIN");
    return 0;
}
