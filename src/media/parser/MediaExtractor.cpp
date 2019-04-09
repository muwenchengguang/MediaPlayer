//
// Created by richie on 7/6/17.
//

#include "MediaExtractor.h"
#include "MediaDefs.h"
#include <string>
#include <string.h>
#include "mp4/MP4Extractor.h"

#define LOG_TAG "MediaExtractor"
#include "log.h"

namespace peng {

sp<MediaExtractor> MediaExtractor::create (const sp <DataSource> &source, const char *mime) {
    std::string tmp;
    if (mime == NULL) {
        if (!source->sniff(tmp)) {
            LOGW("FAILED to autodetect media content.");
            return NULL;
        }

        mime = tmp.c_str();
        LOGI("Autodetected media content as '%s'", mime);
    }

    MediaExtractor *ret = NULL;
    if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG4)
        || !strcasecmp(mime, "audio/mp4")) {
        ret = new MP4Extractor(source);
    }/* else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG)) {
        ret = new MP3Extractor(source, meta);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_NB)
               || !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_WB)) {
        ret = new AMRExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_FLAC)) {
        ret = new FLACExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WAV)) {
        ret = new WAVExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_OGG)) {
        ret = new OggExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MATROSKA)) {
        ret = new MatroskaExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2TS)) {
        ret = new MPEG2TSExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WVM)) {
        // Return now.  WVExtractor should not have the DrmFlag set in the block below.
        return new WVMExtractor(source);
        // MStar Android Patch Begin
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC) ||
               !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC_ADTS)) {
        // MStar Android Patch End
        ret = new AACExtractor(source, meta);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2PS)) {
        ret = new MPEG2PSExtractor(source);
        // MStar Android Patch Begin
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_AVI)) {
        ret = new AVIExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2PS)) {
        ret = new MPEG2PSExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_RM)) {
        ret = new RMExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_FLV)) {
        ret = new FLVExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_ASF)) {
        ret = new ASFExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AC3)) {
        ret = new AC3Extractor(source, meta);
        // MStar Android Patch End
    }

    if (ret != NULL) {
        if (isDrm) {
            ret->setDrmFlag(true);
        } else {
            ret->setDrmFlag(false);
        }
    }*/

    return ret;
}

}
