//
// Created by richie on 7/6/17.
//

#ifndef MPLAYER_METADATA_H
#define MPLAYER_METADATA_H

#include "RefBase.h"
#include <string>
#include <list>
#include "MediaBuffer.h"
#include <stdint.h>

namespace peng {

// The following keys map to int32_t data unless indicated otherwise.
enum {
    kKeyMIMEType          = 'mime',  // cstring
    kKeyWidth             = 'widt',  // int32_t, image pixel
    kKeyHeight            = 'heig',  // int32_t, image pixel
    kKeyDisplayWidth      = 'dWid',  // int32_t, display/presentation
    kKeyDisplayHeight     = 'dHgt',  // int32_t, display/presentation
    kKeySARWidth          = 'sarW',  // int32_t, sampleAspectRatio width
    kKeySARHeight         = 'sarH',  // int32_t, sampleAspectRatio height

    // a rectangle, if absent assumed to be (0, 0, width - 1, height - 1)
    kKeyCropRect          = 'crop',

    kKeyRotation          = 'rotA',  // int32_t (angle in degrees)
    kKeyIFramesInterval   = 'ifiv',  // int32_t
    kKeyStride            = 'strd',  // int32_t
    kKeySliceHeight       = 'slht',  // int32_t
    kKeyChannelCount      = '#chn',  // int32_t
    kKeyChannelMask       = 'chnm',  // int32_t
    kKeySampleRate        = 'srte',  // int32_t (audio sampling rate Hz)
    kKeyFrameRate         = 'frmR',  // int32_t (video frame rate fps)
    kKeyBitRate           = 'brte',  // int32_t (bps)
    kKeyESDS              = 'esds',  // raw data
    kKeyAACProfile        = 'aacp',  // int32_t
    kKeyAVCC              = 'avcc',  // raw data
    kKeyD263              = 'd263',  // raw data
    kKeyVorbisInfo        = 'vinf',  // raw data
    kKeyVorbisBooks       = 'vboo',  // raw data
    kKeyWantsNALFragments = 'NALf',
    kKeyIsSyncFrame       = 'sync',  // int32_t (bool)
    kKeyIsCodecConfig     = 'conf',  // int32_t (bool)
    kKeyTime              = 'time',  // int64_t (usecs)
    kKeyDecodingTime      = 'decT',  // int64_t (decoding timestamp in usecs)
    kKeyNTPTime           = 'ntpT',  // uint64_t (ntp-timestamp)
    kKeyTargetTime        = 'tarT',  // int64_t (usecs)
    kKeyDriftTime         = 'dftT',  // int64_t (usecs)
    kKeyAnchorTime        = 'ancT',  // int64_t (usecs)
    kKeyDuration          = 'dura',  // int64_t (usecs)
    kKeyColorFormat       = 'colf',
    kKeyPlatformPrivate   = 'priv',  // pointer
    kKeyDecoderComponent  = 'decC',  // cstring
    kKeyBufferID          = 'bfID',
    kKeyMaxInputSize      = 'inpS',
    kKeyThumbnailTime     = 'thbT',  // int64_t (usecs)
    kKeyTrackID           = 'trID',
    kKeyIsDRM             = 'idrm',  // int32_t (bool)
    kKeyEncoderDelay      = 'encd',  // int32_t (frames)
    kKeyEncoderPadding    = 'encp',  // int32_t (frames)

    kKeyAlbum             = 'albu',  // cstring
    kKeyArtist            = 'arti',  // cstring
    kKeyAlbumArtist       = 'aart',  // cstring
    kKeyComposer          = 'comp',  // cstring
    kKeyGenre             = 'genr',  // cstring
    kKeyTitle             = 'titl',  // cstring
    kKeyYear              = 'year',  // cstring
    kKeyAlbumArt          = 'albA',  // compressed image data
    kKeyAlbumArtMIME      = 'alAM',  // cstring
    kKeyAuthor            = 'auth',  // cstring
    kKeyCDTrackNumber     = 'cdtr',  // cstring
    kKeyDiscNumber        = 'dnum',  // cstring
    kKeyDate              = 'date',  // cstring
    kKeyWriter            = 'writ',  // cstring
    kKeyCompilation       = 'cpil',  // cstring
    kKeyLocation          = 'loc ',  // cstring
    kKeyTimeScale         = 'tmsl',  // int32_t

    // video profile and level
    kKeyVideoProfile      = 'vprf',  // int32_t
    kKeyVideoLevel        = 'vlev',  // int32_t

    // Set this key to enable authoring files in 64-bit offset
    kKey64BitFileOffset   = 'fobt',  // int32_t (bool)
    kKey2ByteNalLength    = '2NAL',  // int32_t (bool)

    // Identify the file output format for authoring
    // Please see <media/mediarecorder.h> for the supported
    // file output formats.
    kKeyFileType          = 'ftyp',  // int32_t

    // Track authoring progress status
    // kKeyTrackTimeStatus is used to track progress in elapsed time
    kKeyTrackTimeStatus   = 'tktm',  // int64_t

    kKeyRealTimeRecording = 'rtrc',  // bool (int32_t)
    kKeyNumBuffers        = 'nbbf',  // int32_t

    // Ogg files can be tagged to be automatically looping...
    kKeyAutoLoop          = 'autL',  // bool (int32_t)

    kKeyValidSamples      = 'valD',  // int32_t

    kKeyIsUnreadable      = 'unre',  // bool (int32_t)

    // An indication that a video buffer has been rendered.
    kKeyRendered          = 'rend',  // bool (int32_t)

    // The language code for this media
    kKeyMediaLanguage     = 'lang',  // cstring

    // To store the timed text format data
    kKeyTextFormatData    = 'text',  // raw data

    kKeyRequiresSecureBuffers = 'secu',  // bool (int32_t)

    kKeyIsADTS            = 'adts',  // bool (int32_t)
    kKeyAACAOT            = 'aaot',  // int32_t

    // If a MediaBuffer's data represents (at least partially) encrypted
    // data, the following fields aid in decryption.
    // The data can be thought of as pairs of plain and encrypted data
    // fragments, i.e. plain and encrypted data alternate.
    // The first fragment is by convention plain data (if that's not the
    // case, simply specify plain fragment size of 0).
    // kKeyEncryptedSizes and kKeyPlainSizes each map to an array of
    // size_t values. The sum total of all size_t values of both arrays
    // must equal the amount of data (i.e. MediaBuffer's range_length()).
    // If both arrays are present, they must be of the same size.
    // If only encrypted sizes are present it is assumed that all
    // plain sizes are 0, i.e. all fragments are encrypted.
    // To programmatically set these array, use the MetaData::setData API, i.e.
    // const size_t encSizes[];
    // meta->setData(
    //  kKeyEncryptedSizes, 0 /* type */, encSizes, sizeof(encSizes));
    // A plain sizes array by itself makes no sense.
    kKeyEncryptedSizes    = 'encr',  // size_t[]
    kKeyPlainSizes        = 'plai',  // size_t[]
    kKeyCryptoKey         = 'cryK',  // uint8_t[16]
    kKeyCryptoIV          = 'cryI',  // uint8_t[16]
    kKeyCryptoMode        = 'cryM',  // int32_t

    kKeyCryptoDefaultIVSize = 'cryS',  // int32_t

    kKeyPssh              = 'pssh',  // raw data

    // MStar Android Patch Begin
    // Mstar decode config data
    kKeyMsDecConfigData = 'mdcd', // raw data
    // Mstar wma profile level
    kKeyMsWMAProfile = 'mwpr', // int32_t
    // Mstar rm profile level
    kKeyMsRaProfile           ='mrpr', // raw data
    // Mstar adpcm profile level
    kKeyMsAdpcmProfile = 'adpr', // raw data
    // Mstar wma block align
    kKeyMsWMABlockAlign = 'mwba', // int32_t
    // Mstar wma encode options
    kKeyMsWMAEncodeOptions = 'mweo', // int32_t
    // Mstar wma advanced encode options
    kKeyMsWMAAdvancedEncodeOptions = 'mwao', // int32_t
    // Mstar bits per sample
    kKeyMsBitsPerSample = 'mbps', // int32_t
    // Mstar wma channel mask
    kKeyMsWMAChannelMask = 'mwcm', // int32_t
    // Mstar pts mode instead of sts mode
    kKeyMsTimeStampMode = 'mtsm', // int32_t
    // Mstar 4K1KLR mode instead of 3D mode
    kKeyMs4K1KLRMode = '41lr', // int32_t
    // Mstar frame rate num
    kKeyMsFrameRateNum = 'mfrn', // int32_t
    // Mstar frame rate den
    kKeyMsFrameRateDen = 'mfrd', // int32_t
    // Mstar mstplayer frame rate num
    kKeyMsMstFrameRateNum = 'mmfn', // int32_t
    // Mstar mstplayer frame rate den
    kKeyMsMstFrameRateDen = 'mmfd', // int32_t
    // Mstar video interlace
    kKeyMsInterlace = 'intl', // int32_t
    // Mstar Error Code
    kKeyMsErrorCode = 'errc', // int32_t
    // Mstar Error Count
    kKeyMsErrorCount = 'ecnt', // int32_t
    // Mstar Drop Count
    kKeyMsDropCount = 'drpc', // int32_t
    // Mstar Skip Count
    kKeyMsSkipCount = 'skpc', // int32_t
    // Mstar Codec Type
    kKeyMsCodecType = 'cdtp', // int32_t
    // Mstar VOP frame rate
    kKeyMsVopFrameRate = 'vofR', // int32_t
    // Mstar Aspect Width
    kKeyMsAspectWidth = 'aswh', // int32_t
    // Mstar Aspect Height
    kKeyMsAspectHeight = 'asht', // int32_t
    // Mstar 3D Mode
    kKeyMsDisp3DMode = 'd3md', // int32_t
    // Mstar 3D Layout
    kKeyMs3DLayout = 'd3lo', //int32_t
    // Mstar Vsync bridge Sharemem Addr
    kKeyMsSHMAddr = 'shma', // int32_t
    // Mstar crop left
    kKeyMsCropLeft = 'crpl', // int32_t
    // Mstar crop right
    kKeyMsCropRight = 'crpr', // int32_t
    // Mstar crop top
    kKeyMsCropTop = 'crpt', // int32_t
    // Mstar crop bottom
    kKeyMsCropBottom = 'crpb', // int32_t
    // Mstar data on MIU
    kKeyMsDataOnMIU = 'dmiu', // int32_t
    // Mstar current index
    kKeyMsCurIndex = 'cidx', // int32_t
    // Mstar FRC mode
    kKeyMsFrcMode = 'frcm', // int32_t
    // Mstar Overlay ID
    kKeyMsOverlayID = 'ovid', // int32_t
    // MStar S/PDIF time diff for DSP s/pdif sync.
    kKeyMsSpdifPts = 'mspp', // int64_t
    // MStar WMA Specific
    kyeyMsWMACodecSpecificDataSize = 'wmsz', // int32_t
    // MStar WMA Specific
    kyeyMsWMACodecSpecificDataContent = 'wmct', // int32_t
    // MStar Bypass Specific
    kKeySubMIMEType = 'smim',  // cstring
    // MStar OMX decoder report mime type
    kKeyDecdoerMIMEType = 'dmim',  // cstring
    // To store camera id
    kKeyCameraId ='caid', //int32_t
    // pcm audio format
    kKeyMsPcmAudFmt ='pcmf', //int32_t,
    // audio codec ouput pcm length
    kKeyMstOutPutLen ='opml', //int32_t
    // Encorder node
    kKeyMstarEncodeNodeId = 'ENID', //int32_t
    //MStar Tee Path
    kKeyMsTeePath = 'mtph',
    //MSS stream name
    kKeyMssStreamName = 'mssN',
    // MStar Android Patch End
    kKeySPS,
    kKeyPPS,
};

enum {
    kTypeESDS        = 'esds',
    kTypeAVCC        = 'avcc',
    kTypeD263        = 'd263',
};


class MetaData : public RefBase {
public:
    MetaData() {};
    virtual ~MetaData() {};

    void setInt32(int key, int32_t value);
    void setCString(int key, std::string value);
    int findInt32(int key, int32_t& value);
    int findCString(int key, std::string& value);
    void setData(int key, const sp<MediaBuffer>& buffer);
    int findData(int key, sp<MediaBuffer> & buffer);
    void setInt64(int key, int64_t value);
    int findInt64(int key, int64_t& value);

private:
typedef struct {
    int key;
    std::string str_value;
    int32_t int_value;
    int64_t int64_value;
    sp<MediaBuffer> buffer;
} Item;

    std::list<Item> mItems;

};

}


#endif //MPLAYER_METADATA_H
