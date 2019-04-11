#ifndef _PLAY_PCM_H
#define _PLAY_PCM_H
#include <alsa/asoundlib.h>
#include "log.h"
class PCMPlayer {
public:
    PCMPlayer(int sample_rate, int channels):
            channels_(channels),
            sample_rate_(sample_rate),
            init_(false) {

        snd_pcm_hw_params_t *params;
        int rc = snd_pcm_open(&handle_, "default",SND_PCM_STREAM_PLAYBACK, 0);
        if (rc < 0) {
            fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
            exit(1);
        }

        /* Allocate a hardware parameters object. */
        snd_pcm_hw_params_alloca(&params);
        /* Fill it in with default values. */
        snd_pcm_hw_params_any(handle_, params);
        /* Set the desired hardware parameters. */
        /* Interleaved mode */
        snd_pcm_hw_params_set_access(handle_, params, SND_PCM_ACCESS_RW_INTERLEAVED);

        /* Signed 16-bit little-endian format */
        snd_pcm_hw_params_set_format(handle_, params, SND_PCM_FORMAT_S16_LE);

        /* Two channels (stereo) */
        snd_pcm_hw_params_set_channels(handle_, params, channels_);

        /* 44100 bits/second sampling rate (CD quality) */
        unsigned int val = sample_rate_;
        int dir;
        snd_pcm_hw_params_set_rate_near(handle_, params, &val, &dir); 

        /* Set period size to 32 frames. */
        snd_pcm_uframes_t frames = 0;
        snd_pcm_hw_params_set_period_size_near(handle_, params, &frames, &dir);
        
        /* Write the parameters to the driver */
        rc = snd_pcm_hw_params(handle_, params);
        if (rc < 0) {
            fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
            exit(1);
        }

        /* Use a buffer large enough to hold one period */
        snd_pcm_hw_params_get_period_size(params, &frames, &dir);
        frame_size_ = frames;
    }

    void play(short* sample, int samples, int sample_rate, int channels) {
        SetSamplerateAndChannel(sample_rate, channels);

        int pos = 0;
        int size = frame_size_*channels_;
        while (pos < samples) {
            int rc = snd_pcm_writei(handle_, &sample[pos], frame_size_);
            if (rc == -EPIPE) {
                /* EPIPE means underrun */
                fprintf(stderr, "underrun occurred\n");
                snd_pcm_prepare(handle_);
            } else if (rc < 0) {
                fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
            }  else if (rc != (int)frame_size_) {
                fprintf(stderr, "short write, write %d frames\n", rc);
            }
            pos += size;
        }
    }

    ~PCMPlayer() {
        snd_pcm_drain(handle_);
        snd_pcm_close(handle_);
    }
    
protected:
    void SetSamplerateAndChannel(int sample_rate, int channels) {
        if (sample_rate_ != sample_rate || channels_ != channels) {
            PRINT("SetSamplerateAndChannel: sample rate = %d  chs = %d", sample_rate, channels);
            sample_rate_ = sample_rate;
            channels_ = channels;

            /* Allocate a hardware parameters object. */
            snd_pcm_hw_params_alloca(&params_);
            /* Fill it in with default values. */
            snd_pcm_hw_params_any(handle_, params_);
            /* Set the desired hardware parameters. */
            /* Interleaved mode */
            snd_pcm_hw_params_set_access(handle_, params_, SND_PCM_ACCESS_RW_INTERLEAVED);

            /* Signed 16-bit little-endian format */
            snd_pcm_hw_params_set_format(handle_, params_, SND_PCM_FORMAT_S16_LE);

            int dir;
            unsigned int val = sample_rate_;
            snd_pcm_hw_params_set_rate_near(handle_, params_, &val, &dir);

            /* Two channels (stereo) */
            snd_pcm_hw_params_set_channels(handle_, params_, channels_);

            /* Set period size to 32 frames. */
            snd_pcm_uframes_t frames = 0;
            snd_pcm_hw_params_set_period_size_near(handle_, params_, &frames, &dir);

            /* Write the parameters to the driver */
            int rc = snd_pcm_hw_params(handle_, params_);
            if (rc < 0) {
                fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
                exit(1);
            }

            /* Use a buffer large enough to hold one period */
            snd_pcm_hw_params_get_period_size(params_, &frames, &dir);
            frame_size_ = frames;

            PRINT("SetSamplerateAndChannel--<");
            sleep(1);
        }
    }

private:
    int channels_;
    int sample_rate_;
    int frame_size_;
    bool init_;
    snd_pcm_t *handle_;
    snd_pcm_hw_params_t *params_;
};

#endif

