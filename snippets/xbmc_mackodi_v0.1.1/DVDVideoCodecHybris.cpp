/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
//#undef NDEBUG
//#include <assert.h>
#include "system.h"
#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif
#include "DVDVideoCodecHybris.h"

//#define REPORT_FUNCTION() do { printf("%s \n", __PRETTY_FUNCTION__); fflush(stdout); } while(0)
#define REPORT_FUNCTION() 

#include "Application.h"
#include "ApplicationMessenger.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "DVDDemuxers/DVDDemux.h"
#include "DVDStreamInfo.h"
#include "DVDClock.h"
#include "DVDCodecs/DVDCodecs.h"
#include "DVDCodecs/DVDCodecUtils.h"
#include "settings/Settings.h"
#include "settings/DisplaySettings.h"
#include "settings/AdvancedSettings.h"

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include <dirent.h>

#include "utils/log.h"

#ifdef CLASSNAME
#undef CLASSNAME
#endif
#define CLASSNAME "CDVDVideoCodecHybris"

#define OK 0

// android/frameworks/av/media/libstagefright/MediaDefs.cpp
const char *MEDIA_MIMETYPE_VIDEO_VP8 = "video/x-vnd.on2.vp8";
const char *MEDIA_MIMETYPE_VIDEO_VP9 = "video/x-vnd.on2.vp9";
const char *MEDIA_MIMETYPE_VIDEO_AVC = "video/avc";
const char *MEDIA_MIMETYPE_VIDEO_MPEG4 = "video/mp4v-es";
const char *MEDIA_MIMETYPE_VIDEO_HEVC  = "video/hevc";
const char *MEDIA_MIMETYPE_VIDEO_H263 = "video/3gpp";
const char *MEDIA_MIMETYPE_VIDEO_MPEG2 = "video/mpeg2";
const char *MEDIA_MIMETYPE_VIDEO_RAW = "video/raw";
const char *MEDIA_MIMETYPE_VIDEO_MJPEG = "video/mjpeg";
const char *MEDIA_MIMETYPE_VIDEO_REALVIDEO =  "video/vnd.rn-realvideo";
const char *MEDIA_MIMETYPE_VIDEO_FLV =  "video/flv";
const char *MEDIA_MIMETYPE_VIDEO_VC1 =  "video/vc1";
const char *MEDIA_MIMETYPE_VIDEO_WMV3 =  "video/x-ms-wmv";
const char *MEDIA_MIMETYPE_VIDEO_VP6 =  "video/vp6";

// Rockchip/ON2 hw supported codings
// /usr/local/include/android/libon2/vpu_api.h
//    OMX_ON2_VIDEO_CodingMPEG2,      /**< AKA: H.262 */^M
//    OMX_ON2_VIDEO_CodingH263,       /**< H.263 */^M
//    OMX_ON2_VIDEO_CodingMPEG4,      /**< MPEG-4 */^M
//    OMX_ON2_VIDEO_CodingWMV,        /**< Windows Media Video (WMV1,WMV2,WMV3)*/^M
//    OMX_ON2_VIDEO_CodingRV,         /**< all versions of Real Video */^M
//    OMX_ON2_VIDEO_CodingAVC,        /**< H.264/AVC */^M
//    OMX_ON2_VIDEO_CodingMJPEG,      /**< Motion JPEG */^M
//    OMX_ON2_VIDEO_CodingVP8,        /**< VP8 */^M
//    OMX_ON2_VIDEO_CodingVP9,        /**< VP9 */^M
//    OMX_ON2_VIDEO_CodingVC1 = 0x01000000, /**< Windows Media Video (WMV1,WMV2,WMV3)*/^M
//    OMX_ON2_VIDEO_CodingFLV1,       /**< Sorenson H.263 */^M
//    OMX_ON2_VIDEO_CodingDIVX3,      /**< DIVX3 */^M
//    OMX_ON2_VIDEO_CodingVP6,^M
//    OMX_RK_VIDEO_CodingHEVC,        /**< H.265/HEVC */^M

#define HAL_PIXEL_FORMAT_YCrCb_NV12      0x20

typedef bool FrameAvailableCb;

class CDVDMediaCodecOnFrameAvailable : public CEvent
{
private:
  GLConsumerWrapperHybris m_surfaceTexture;
  FrameAvailableCb frame_available_cb;
  void *frame_available_context;

public:
  void OnFrameAvailable(GLConsumerWrapperHybris surfaceTexture) {
    REPORT_FUNCTION();
    Set();
    //printf("F");fflush(stdout);
  }

public:
  CDVDMediaCodecOnFrameAvailable(GLConsumerWrapperHybris surfaceTexture) : 
    m_surfaceTexture(surfaceTexture) ,
    frame_available_cb(false),
    frame_available_context(NULL) {
    //m_surfaceTexture->setOnFrameAvailableListener(*this);
    REPORT_FUNCTION();
    set_frame_available_cb(true,m_surfaceTexture);
  }

  ~CDVDMediaCodecOnFrameAvailable() { 
    REPORT_FUNCTION();
    set_frame_available_cb(false,m_surfaceTexture);
  }

  static void on_frame_available_cb(GLConsumerWrapperHybris wrapper, void* context) {
    REPORT_FUNCTION();
    if (context != NULL) {
      CDVDMediaCodecOnFrameAvailable* p = static_cast<CDVDMediaCodecOnFrameAvailable*>(context);
      p->on_frame_available();
    } else
      CLog::Log(LOGERROR, "%s::%s - %s", 
              CLASSNAME, __func__, 
              "context is NULL, can't call on_frame_available()" );
  }

  void on_frame_available()
  {
    REPORT_FUNCTION();
    if (frame_available_cb) {
      OnFrameAvailable(frame_available_context);
    } else
      CLog::Log(LOGERROR, "%s::%s - %s", 
              CLASSNAME, __func__, 
        "frame_available_cb is NULL, can't call frame_available_cb()" );
  }

  void set_frame_available_cb(FrameAvailableCb cb, void *context) {
    REPORT_FUNCTION();
    frame_available_cb = cb;
    frame_available_context = context;

    gl_consumer_set_frame_available_cb(m_surfaceTexture, &CDVDMediaCodecOnFrameAvailable::on_frame_available_cb, static_cast<void*>(this));
  }
};

CDVDMediaCodecInfo::CDVDMediaCodecInfo( int index, unsigned int texture,
  MediaCodecDelegate codec, GLConsumerWrapperHybris surfacetexture,
  std::shared_ptr<CDVDMediaCodecOnFrameAvailable> &frameready)
: m_refs(1)
, m_valid(true)
, m_isReleased(true)
, m_index(index)
, m_texture(texture)
, m_timestamp(0)
, m_codec(codec)
, m_surfacetexture(surfacetexture)
, m_frameready(frameready) {
  // paranoid checks
  REPORT_FUNCTION();
  assert(m_index >= 0);
  assert(m_texture > 0);
  assert(m_codec != NULL);
  assert(m_surfacetexture != NULL);
  assert(m_frameready != NULL);
}

CDVDMediaCodecInfo::~CDVDMediaCodecInfo() {
  REPORT_FUNCTION();
  assert(m_refs == 0);
}

CDVDMediaCodecInfo* CDVDMediaCodecInfo::Retain() {
  //REPORT_FUNCTION();
  AtomicIncrement(&m_refs);
  m_isReleased = false;
  return this;
}

long CDVDMediaCodecInfo::Release() {
  //REPORT_FUNCTION();
  long count = AtomicDecrement(&m_refs);
  if (count == 1)
    ReleaseOutputBuffer(false);
  if (count == 0)
    delete this;
  return count;
}

void CDVDMediaCodecInfo::Validate(bool state) {
  CSingleLock lock(m_section);
  //REPORT_FUNCTION();
  m_valid = state;
}

void CDVDMediaCodecInfo::ReleaseOutputBuffer(bool render) {
  CSingleLock lock(m_section);
  //REPORT_FUNCTION();
  //printf("void CDVDMediaCodecInfo::ReleaseOutputBuffer(%x)\n", render);fflush(stdout);

  if (!m_valid || m_isReleased)
    return;

  // release OutputBuffer and render if indicated
  // then wait for rendered frame to become avaliable.

  if (render)
    m_frameready->Reset();

  //m_codec->releaseOutputBuffer(m_index, render);
  media_codec_release_output_buffer(m_codec, m_index, render);
  m_isReleased = true;
}

int CDVDMediaCodecInfo::GetIndex() const {
  CSingleLock lock(m_section);
  //REPORT_FUNCTION();
  return m_index;
}

int CDVDMediaCodecInfo::GetTextureID() const {
  //REPORT_FUNCTION();
  // since m_texture never changes,
  // we do not need a m_section lock here.
  return m_texture;
}

void CDVDMediaCodecInfo::GetTransformMatrix(float *textureMatrix) {
  CSingleLock lock(m_section);
  REPORT_FUNCTION();

  if (!m_valid)
    return;

  //m_surfacetexture->getTransformMatrix(textureMatrix);
  gl_consumer_get_transformation_matrix(m_surfacetexture,textureMatrix);
}

void CDVDMediaCodecInfo::UpdateTexImage() {
  CSingleLock lock(m_section);
  REPORT_FUNCTION();

  if (!m_valid)
    return;

  // updateTexImage will check and spew any prior gl errors,
  // clear them before we call updateTexImage.
  glGetError();

  // this is key, after calling releaseOutputBuffer, we must
  // wait a little for MediaCodec to render to the surface.
  // Then we can updateTexImage without delay. If we do not
  // wait, then video playback gets jerky. To optomize this,
  // we hook the SurfaceTexture OnFrameAvailable callback
  // using CJNISurfaceTextureOnFrameAvailableListener and wait
  // on a CEvent to fire. 50ms seems to be a good max fallback.
  m_frameready->WaitMSec(50);

  //m_surfacetexture->updateTexImage();
  gl_consumer_update_texture(m_surfacetexture);

  //m_timestamp = m_surfacetexture->getTimestamp();
}

CDVDVideoCodecHybris::CDVDVideoCodecHybris() : CDVDVideoCodec() {
  m_iDecodedWidth = 0;
  m_iDecodedHeight = 0;
  m_iConvertedWidth = 0;
  m_iConvertedHeight = 0;
  m_drop = false;
  m_codec = NULL;
  m_format = NULL;
  memset(&m_videoBuffer, 0, sizeof(m_videoBuffer));
  m_bVideoConvert = false;
  m_bitstream = NULL;
  m_render_sw = false;
  m_surface = NULL;
  m_textureId = 0;
}

CDVDVideoCodecHybris::~CDVDVideoCodecHybris() {
  CLog::Log(LOGNOTICE, "%s::%s::%d - LOG", CLASSNAME, __func__, __LINE__);
  Dispose();
  CLog::Log(LOGNOTICE, "%s::%s::%d - LOG", CLASSNAME, __func__, __LINE__);
}

bool CDVDVideoCodecHybris::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options) {
  int ret = 0;
  
  if (hints.software)
    return false;

  // stagefright crashes with null size. Trap this...
  if (!hints.width || !hints.height) {
    CLog::Log(LOGERROR, "%s::%s - %s\n", CLASSNAME, __func__,"null size, cannot handle");
    return false;
  }

  if(m_bitstream) {
    SAFE_DELETE(m_bitstream);
    m_bitstream = NULL;
  }

  CLog::Log(LOGDEBUG,
          "%s::%s - trying to open, codec(%d), profile(%d), level(%d)",
          CLASSNAME, __func__, hints.codec, hints.profile, hints.level);

  m_drop = false;
  m_hints = hints;

  switch(hints.codec) {
//    OMX_ON2_VIDEO_CodingMPEG2,      /**< AKA: H.262 */^M
    case AV_CODEC_ID_MPEG2VIDEO:
      m_mimeType = "video/mpeg2";
      m_name = "hyb-mpeg2";
      break;

//    OMX_ON2_VIDEO_CodingH263,       /**< H.263 */^M
    case AV_CODEC_ID_H263:
      m_name = "hyb-h263";
      m_mimeType = "video/3gpp";
      break;

//    OMX_ON2_VIDEO_CodingMPEG4,      /**< MPEG-4 */^M
    case AV_CODEC_ID_MPEG4:
      m_name = "hyb-mpeg4";
      m_mimeType = "video/mp4v-es";
      break;

//    OMX_ON2_VIDEO_CodingAVC,        /**< H.264/AVC */^M
    case AV_CODEC_ID_AVS:
    case AV_CODEC_ID_CAVS:
    case AV_CODEC_ID_H264:
      switch(hints.profile)
      {
        case FF_PROFILE_H264_HIGH_10:
        case FF_PROFILE_H264_HIGH_10_INTRA:
        case FF_PROFILE_H264_HIGH_422:
        case FF_PROFILE_H264_HIGH_422_INTRA:
        case FF_PROFILE_H264_HIGH_444_PREDICTIVE:
        case FF_PROFILE_H264_HIGH_444_INTRA:
        case FF_PROFILE_H264_CAVLC_444:
          // Hi10P not supported
          return false;
      }

      m_name = "hyb-h264";
      m_mimeType = "video/avc";
      if (m_hints.extradata) {
        m_bitstream = new CBitstreamConverter;
        if (!m_bitstream->Open(m_hints.codec, (uint8_t*)m_hints.extradata, m_hints.extrasize, true)) {
          SAFE_DELETE(m_bitstream);
        }
      }
      break;

//    OMX_ON2_VIDEO_CodingVP6,^M
//    OMX_ON2_VIDEO_CodingVP8,        /**< VP8 */^M
    case AV_CODEC_ID_VP3:
    case AV_CODEC_ID_VP6:
    case AV_CODEC_ID_VP6F:
    case AV_CODEC_ID_VP8:
      //m_mimeType = "video/x-vp6";
      //m_mimeType = "video/x-vp7";
      m_mimeType = "video/x-vnd.on2.vp8";
      m_name = "hyb-vpX";
      break;

//    OMX_ON2_VIDEO_CodingWMV,        /**< Windows Media Video (WMV1,WMV2,WMV3)*/^M
    case AV_CODEC_ID_WMV3:
      m_mimeType = "video/wmv3";
      m_name = "hyb-wmv3";
      if (m_hints.extradata) {
        m_bitstream = new CBitstreamConverter;
        if (!m_bitstream->Open(m_hints.codec, (uint8_t*)m_hints.extradata, m_hints.extrasize, true)) {
          SAFE_DELETE(m_bitstream);
        }
      }
      break;
//    OMX_ON2_VIDEO_CodingVC1 = 0x01000000, /**< Windows Media Video (WMV1,WMV2,WMV3)*/^M
    case AV_CODEC_ID_VC1:
      m_mimeType = "video/vc1";
      m_name = "hyb-vc1";
      if (m_hints.extradata) {
        m_bitstream = new CBitstreamConverter;
        if (!m_bitstream->Open(m_hints.codec, (uint8_t*)m_hints.extradata, m_hints.extrasize, true)) {
          SAFE_DELETE(m_bitstream);
        }
      }
      break;

// mac_l1: 4K HEVC crashes after 10-20 secs... so exclude for now
//    OMX_RK_VIDEO_CodingHEVC,        /**< H.265/HEVC */^M
    case AV_CODEC_ID_HEVC:
      m_mimeType = "video/hevc";
      m_name = "hyb-h265";
      // check for hevc-hvcC and convert to h265-annex-b
      if (m_hints.extradata) {
        m_bitstream = new CBitstreamConverter;
        if (!m_bitstream->Open(m_hints.codec, (uint8_t*)m_hints.extradata, m_hints.extrasize, true)) {
          SAFE_DELETE(m_bitstream);
        }
      }
      break;

// mac_l1: better safe than sorry (although they might be supported by HW): 
// following are not supported in Androids XBMC/SPMC
//    OMX_ON2_VIDEO_CodingRV,         /**< all versions of Real Video */^M
//    OMX_ON2_VIDEO_CodingMJPEG,      /**< Motion JPEG */^M
//    OMX_ON2_VIDEO_CodingFLV1,       /**< Sorenson H.263 */^M
// following are not implemented in 
// frameworks/av/media/libstagefright/codecs/rkon2dec/RkOn2Decoder.cpp
// however, could work though...
//    OMX_ON2_VIDEO_CodingDIVX3,      /**< DIVX3 */^M
//    OMX_ON2_VIDEO_CodingVP9,        /**< VP9 */^M
    default:
      CLog::Log(LOGDEBUG, "%s::%s - Unsupported hints.codec(%d)", 
                CLASSNAME, __func__, hints.codec);
      return false;
      break;
  }

  m_format = media_format_create_video_format(m_mimeType.c_str(), 
                 hints.width, hints.height, 0, 0);
  if (m_format == NULL) {
    CLog::Log(LOGERROR, "%s::%s - Failed to create format object for %s", 
              CLASSNAME, __func__, m_mimeType.c_str());
    return false;
  }

  m_codec = media_codec_create_by_codec_type(m_mimeType.c_str());
  if (m_codec == NULL) {
    CLog::Log(LOGERROR, "%s::%s - Failed to create codec for %s", 
              CLASSNAME, __func__, m_mimeType.c_str());
    media_codec_release(m_codec);
    media_codec_delegate_destroy(m_codec);
    media_format_destroy(m_codec);
    return false;
  }

  if (hints.extrasize > 0) {
    size_t size = m_hints.extrasize;
    void *src_ptr = m_hints.extradata;
    if (m_bitstream)
    {
      size = m_bitstream->GetExtraSize();
      src_ptr = m_bitstream->GetExtraData();
    }
    media_format_set_byte_buffer(m_format, "csd-0", (uint8_t*)src_ptr, size);
  }

  // setup a YUV420P DVDVideoPicture buffer.
  // first make sure all properties are reset.
  memset(&m_videoBuffer, 0x00, sizeof(DVDVideoPicture));

  m_videoBuffer.dts = DVD_NOPTS_VALUE;
  m_videoBuffer.pts = DVD_NOPTS_VALUE;
  m_videoBuffer.color_range = 0;
  m_videoBuffer.color_matrix = 4;
  m_videoBuffer.iFlags = DVP_FLAG_ALLOCATED;
  m_videoBuffer.iWidth = m_hints.width;
  m_videoBuffer.iHeight = m_hints.height;
  // these will get reset to crop values later
  m_videoBuffer.iDisplayWidth = m_hints.width;
  m_videoBuffer.iDisplayHeight = m_hints.height;

  m_render_sw = false;

  InitSurfaceTexture();

  if( m_render_sw ) {
    if (media_codec_configure(m_codec, m_format, NULL, 0) != OK) {
      CLog::Log(LOGERROR, "%s::%s - Failed to configure codec for %s", 
                CLASSNAME, __func__, m_mimeType.c_str());
      return false;
    }
  } else {
    if (media_codec_configure(m_codec, m_format, m_surface, 0) != OK) {
      CLog::Log(LOGERROR, "%s::%s - Failed to configure codec w surface for %s",
                CLASSNAME, __func__, m_mimeType.c_str());
      return false;
    }
  }

  media_codec_start(m_codec);

  return true;
}

void CDVDVideoCodecHybris::Dispose() {
  CLog::Log(LOGDEBUG, "%s::%s - Freeing memory allocated for buffers", CLASSNAME, __func__);

  while(!m_pts.empty())
    m_pts.pop();
  while(!m_dts.empty())
    m_dts.pop();

  //m_input.clear();
  //m_output.clear();
  FlushInternal();

  // clear m_videoBuffer bits
  if (m_render_sw)
  {
    if(m_videoBuffer.data[0]){free(m_videoBuffer.data[0]), m_videoBuffer.data[0] = NULL;}
    if(m_videoBuffer.data[1]){free(m_videoBuffer.data[1]), m_videoBuffer.data[1] = NULL;}
    if(m_videoBuffer.data[2]){free(m_videoBuffer.data[2]), m_videoBuffer.data[2] = NULL;}
  }
  m_videoBuffer.iFlags = 0;
  // m_videobuffer.mediacodec is unioned with m_videobuffer.data[0]
  // so be very careful when and how you touch it.
  m_videoBuffer.mediacodec = NULL;

  if(m_codec) {
    media_codec_stop(m_codec);
    media_codec_release(m_codec);
    media_codec_delegate_destroy(m_codec);
    m_codec = NULL;
  }
  if(m_format) {
    media_format_destroy(m_format);
    m_format = NULL;
  }
  ReleaseSurfaceTexture();

  m_iDecodedWidth = 0;
  m_iDecodedHeight = 0;
  m_iConvertedWidth = 0;
  m_iConvertedHeight = 0;
  m_drop = false;
  m_bVideoConvert = false;

  if(m_bitstream) {
      SAFE_DELETE(m_bitstream);
      m_bitstream = NULL;
  }
}

int CDVDVideoCodecHybris::Decode(BYTE* pData, int iSize, double dts, double pts) {
  // Handle input, add demuxer packet to input queue, we must accept it or
  // it will be discarded as DVDPlayerVideo has no concept of "try again".
  // we must return VC_BUFFER or VC_PICTURE, default to VC_BUFFER.
  int rtn = VC_BUFFER;

  MediaCodecBufferInfo bufferInfo;
  if (m_hints.ptsinvalid)
    pts = DVD_NOPTS_VALUE;

  // must check for an output picture 1st,
  // otherwise, mediacodec can stall on some devices.
  if (GetOutputPicture() > 0)
    rtn |= VC_PICTURE;

  if (pData)
  {
    if (m_bitstream)
    {
      m_bitstream->Convert(pData, iSize);
      iSize = m_bitstream->GetConvertSize();
      pData = m_bitstream->GetConvertBuffer();
    }

    // queue demux pkt in case we cannot get an input buffer
    amc_demux demux_pkt;
    demux_pkt.dts = dts;
    demux_pkt.pts = pts;
    demux_pkt.iSize = iSize;
    demux_pkt.pData = (uint8_t*)malloc(iSize);
    memcpy(demux_pkt.pData, pData, iSize);
    m_demux.push(demux_pkt);

    // try to fetch an input buffer
    int64_t timeout_us = 5000;
    size_t index;// = m_codec->dequeueInputBuffer(timeout_us);
    int ret = media_codec_dequeue_input_buffer(m_codec, &index, timeout_us);
    if (ret == OK)
    {
      // we have an input buffer, fill it.
      int size = media_codec_get_nth_input_buffer_capacity(m_codec, index);
      // fetch the front demux packet
      amc_demux &demux_pkt = m_demux.front();
      if (demux_pkt.iSize > size)
      {
        CLog::Log(LOGERROR, "CDVDVideoCodecHybris::Decode, iSize(%d) > size(%d)", iSize, size);
        demux_pkt.iSize = size;
      }
      // fetch a pointer to the ByteBuffer backing store
      void *dst_ptr = media_codec_get_nth_input_buffer(m_codec, index);
      if (dst_ptr)
        memcpy(dst_ptr, demux_pkt.pData, demux_pkt.iSize);

      free(demux_pkt.pData);
      m_demux.pop();

      // Translate from dvdplayer dts/pts to MediaCodec pts,
      // pts WILL get re-ordered by MediaCodec if needed.
      // Do not try to pass pts as a unioned double/int64_t,
      // some android devices will diddle with presentationTimeUs
      // and you will get NaN back and DVDPlayerVideo will barf.
      int64_t presentationTimeUs = AV_NOPTS_VALUE;
      if (demux_pkt.pts != DVD_NOPTS_VALUE)
        presentationTimeUs = demux_pkt.pts;
      else if (demux_pkt.dts != DVD_NOPTS_VALUE)
        presentationTimeUs = demux_pkt.dts;
/*
      CLog::Log(LOGDEBUG, "CDVDVideoCodecHybris:: "
          "pts(%f), ipts(%lld), iSize(%d), GetDataSize(%d), loop_cnt(%d)",
           presentationTimeUs, pts_dtoi(presentationTimeUs), iSize, 
           GetDataSize(), loop_cnt);
*/
      //m_codec->queueInputBuffer(index, offset, demux_pkt.iSize, presentationTimeUs, flags);
      bufferInfo.index = index;
      bufferInfo.offset = 0;
      bufferInfo.size = demux_pkt.iSize;
      bufferInfo.presentation_time_us = 0;//presentationTimeUs;
      bufferInfo.flags = 0;

      if(media_codec_queue_input_buffer(m_codec, &bufferInfo) != OK) {
        CLog::Log(LOGERROR, "CDVDVideoCodecHybris::Decode, failed to queue input buffer!");
      }
    }
  }

  return rtn;
}

void CDVDVideoCodecHybris::Reset() {
  // dump any pending demux packets
  while (!m_demux.empty())
  {
    amc_demux &demux_pkt = m_demux.front();
    free(demux_pkt.pData);
    m_demux.pop();
  }

  if (m_codec)
  {
    // flush all outputbuffers inflight, they will
    // become invalid on m_codec->flush and generate
    // a spew of java exceptions if used
    FlushInternal();

    // now we can flush the actual MediaCodec object
    if(media_codec_flush(m_codec) != OK) {
        CLog::Log(LOGERROR, "CDVDVideoCodecHybris::Reset(), media_codec_flush failed!");
    }

    // Invalidate our local DVDVideoPicture bits
    m_videoBuffer.pts = DVD_NOPTS_VALUE;
    if (!m_render_sw)
      m_videoBuffer.mediacodec = NULL;
  }
}

bool CDVDVideoCodecHybris::GetPicture(DVDVideoPicture* pDvdVideoPicture) {
  *pDvdVideoPicture = m_videoBuffer;
  m_videoBuffer.pts = DVD_NOPTS_VALUE;
  if (!m_render_sw)
    m_videoBuffer.mediacodec = NULL;

  return true;
}

bool CDVDVideoCodecHybris::ClearPicture(DVDVideoPicture* pDvdVideoPicture) {
  if (pDvdVideoPicture->format == RENDER_FMT_MEDIACODEC)
    SAFE_RELEASE(pDvdVideoPicture->mediacodec);
  memset(pDvdVideoPicture, 0x00, sizeof(DVDVideoPicture));
  return true;
  //return CDVDVideoCodec::ClearPicture(pDvdVideoPicture);
}

void CDVDVideoCodecHybris::SetDropState(bool bDrop) {
  m_drop = bDrop;
  if(m_drop)
    m_videoBuffer.iFlags |=  DVP_FLAG_DROPPED;
  else
    m_videoBuffer.iFlags &= ~DVP_FLAG_DROPPED;
}

int CDVDVideoCodecHybris::GetDataSize(void)
{
  // just ignore internal buffering contribution.
  return 0;
}

double CDVDVideoCodecHybris::GetTimeSize(void)
{
  // just ignore internal buffering contribution.
  return 0.0;
}

unsigned CDVDVideoCodecHybris::GetAllowedReferences()
{
  return 3;
}

void CDVDVideoCodecHybris::FlushInternal()
{
  // invalidate any existing inflight buffers and create
  // new ones to match the number of output buffers
  if (m_render_sw)
    return;

  for (size_t i = 0; i < m_inflight.size(); i++)
    m_inflight[i]->Validate(false);
  m_inflight.clear();
/*
  for (size_t i = 0; i < m_output.size(); i++)  // Deprecated as of API 21
  {
    m_inflight.push_back(
      new CDVDMediaCodecInfo(i, m_textureId, m_codec, m_surfaceTexture, m_frameAvailable)
    );
  }
*/
}

int CDVDVideoCodecHybris::GetOutputPicture(void) {
  int rtn = 0;
  int ret = 0;
  int64_t timeout_us = 5000;
  int index;// = m_codec->dequeueOutputBuffer(bufferInfo, timeout_us);
  MediaCodecBufferInfo bufferInfo;
  ret = media_codec_dequeue_output_buffer(m_codec, &bufferInfo, timeout_us);
  index = bufferInfo.index;
  if (ret == OK)
  {
    if (m_drop)
    {
      media_codec_release_output_buffer(m_codec, index, 0);
      return 0;
    }

    // some devices will return a valid index
    // before signaling INFO_OUTPUT_BUFFERS_CHANGED which
    // is used to setup m_output, D'uh. setup m_output here.
#if 0
    if (m_output.empty())
    {
      m_output = m_codec->getOutputBuffers();
      FlushInternal();
    }
#endif
    int flags = bufferInfo.flags;
#if 0
    if (flags & CJNIMediaCodec::BUFFER_FLAG_SYNC_FRAME)
      CLog::Log(LOGDEBUG, "CDVDVideoCodecHybris:: BUFFER_FLAG_SYNC_FRAME");

    if (flags & CJNIMediaCodec::BUFFER_FLAG_CODEC_CONFIG)
      CLog::Log(LOGDEBUG, "CDVDVideoCodecHybris:: BUFFER_FLAG_CODEC_CONFIG");
#endif
    if (flags & 4 /*CJNIMediaCodec::BUFFER_FLAG_END_OF_STREAM*/)
    {
      CLog::Log(LOGDEBUG, "CDVDVideoCodecHybris:: BUFFER_FLAG_END_OF_STREAM");
      media_codec_release_output_buffer(m_codec, index, 0);
      return 0;
    }

    if (!m_render_sw)
    {
      size_t i = 0;
      for (; i < m_inflight.size(); ++i)
      {
        if (m_inflight[i]->GetIndex() == index)
          break;
      }
      if (i == m_inflight.size())
        m_inflight.push_back(
          new CDVDMediaCodecInfo(index, m_textureId, m_codec, m_surfaceTexture, m_frameAvailable)
        );
      m_videoBuffer.mediacodec = m_inflight[i]->Retain();
      m_videoBuffer.mediacodec->Validate(true);
    }
    else
    {
      int size = bufferInfo.size;
      int offset = bufferInfo.offset;

      if (size && media_codec_get_nth_output_buffer_capacity(m_codec, index))
      {
        uint8_t *src_ptr = (uint8_t*)media_codec_get_nth_output_buffer(m_codec, index);
        src_ptr += offset;

        int loop_end = 0;
        if (m_videoBuffer.format == RENDER_FMT_NV12)
          loop_end = 2;
        else if (m_videoBuffer.format == RENDER_FMT_YUV420P)
          loop_end = 3;

        for (int i = 0; i < loop_end; i++)
        {
          uint8_t *src = src_ptr + m_src_offset[i];
          int src_stride = m_src_stride[i];
          uint8_t *dst = m_videoBuffer.data[i];
          int dst_stride = m_videoBuffer.iLineSize[i];

          int height = m_videoBuffer.iHeight;
          if (i > 0)
            height = (m_videoBuffer.iHeight + 1) / 2;

          for (int j = 0; j < height; j++, src += src_stride, dst += dst_stride)
            memcpy(dst, src, dst_stride);
        }
      }
      media_codec_release_output_buffer(m_codec, index, 0);
    }

    int64_t pts = bufferInfo.presentation_time_us;
    m_videoBuffer.dts = DVD_NOPTS_VALUE;
//    m_videoBuffer.pts = DVD_NOPTS_VALUE;
  //  if (pts != AV_NOPTS_VALUE)
    //  m_videoBuffer.pts = pts;
#if 0
    if (pts != AV_NOPTS_VALUE)
      m_videoBuffer.pts = pts;
#endif
/*
    CLog::Log(LOGDEBUG, "CDVDVideoCodecHybris::GetOutputPicture "
        "index(%d), pts(%f)", index, m_videoBuffer.pts);
*/
    rtn = 1;
  }
  else if (ret == -3 /* == CJNIMediaCodec::INFO_OUTPUT_BUFFERS_CHANGED*/)
  {
//    m_output = m_codec->getOutputBuffers();
  }
  else if (ret == -2 /* CJNIMediaCodec::INFO_OUTPUT_FORMAT_CHANGED*/)
  {
    OutputFormatChanged();
  }
  else if (ret == -1/* CJNIMediaCodec::INFO_TRY_AGAIN_LATER*/)
  {
    // normal dequeueOutputBuffer timeout, ignore it.
    rtn = -1;
  }
  else
  {
    // we should never get here
    CLog::Log(LOGERROR, "CDVDVideoCodecHybris::GetOutputPicture unknown index(%d)", index);
  }

  return rtn;
}

void CDVDVideoCodecHybris::OutputFormatChanged(void)
{
  MediaFormat f = media_codec_get_output_format(m_codec);

  int width = media_format_get_width(f);
  int height = media_format_get_height(f);
  int stride = media_format_get_stride(f);
  int slice_height = media_format_get_slice_height(f);
  int color_format= media_format_get_color_format(f);
  int crop_left = media_format_get_crop_left(f);
  int crop_top = media_format_get_crop_top(f);
  int crop_right = media_format_get_crop_right(f);
  int crop_bottom = media_format_get_crop_bottom(f);

  CLog::Log(LOGNOTICE, "CDVDVideoCodecHybris:: "
    "width(%d), height(%d), stride(%d), slice-height(%d), color-format(%d)",
    width, height, stride, slice_height, color_format);
  CLog::Log(LOGNOTICE, "CDVDVideoCodecHybris:: "
    "crop-left(%d), crop-top(%d), crop-right(%d), crop-bottom(%d)",
    crop_left, crop_top, crop_right, crop_bottom);

  if (!m_render_sw)
  {
    CLog::Log(LOGDEBUG, "CDVDVideoCodecHybris:: Direct Surface Rendering");
    m_videoBuffer.format = RENDER_FMT_MEDIACODEC;
  }
  else
  {
    //  Android device quirks and fixes
    if (stride <= width)
      stride = width;
    if (!crop_right)
      crop_right = width-1;
    if (!crop_bottom)
      crop_bottom = height-1;

    // default picture format to none
    for (int i = 0; i < 4; i++)
      m_src_offset[i] = m_src_stride[i] = 0;
    // delete any existing buffers
    for (int i = 0; i < 4; i++)
      free(m_videoBuffer.data[i]);
  
    // setup picture format and data offset vectors
    if (color_format == HAL_PIXEL_FORMAT_YCrCb_NV12 /* RK dedicated: 32 */)
    {
      CLog::Log(LOGDEBUG, 
                "CDVDVideoCodecHybris::HAL_PIXEL_FORMAT_YCrCb_NV12 = 0x20");

      // RK quirks
      if( m_hints.codec == AV_CODEC_ID_HEVC ) {
        stride = (((width + 255)&(~255))|256);
        slice_height = (height + 15)&(~15);
      } else {
        stride = (width + 15)&(~15);
        slice_height = (height + 15)&(~15);
      }

      // Y plane
      m_src_stride[0] = stride;
      m_src_offset[0] = crop_top * stride;
      m_src_offset[0]+= crop_left;

      // UV plane
      m_src_stride[1] = stride;
      // skip over the Y plane
      m_src_offset[1] = slice_height * stride;
      m_src_offset[1]+= crop_top * stride;
      m_src_offset[1]+= crop_left;

      m_videoBuffer.iLineSize[0] = width; // Y
      m_videoBuffer.iLineSize[1] = width; // UV
      m_videoBuffer.iLineSize[2] = 0;
      m_videoBuffer.iLineSize[3] = 0;

      unsigned int iPixels = width * height;
      unsigned int iChromaPixels = iPixels;
      m_videoBuffer.data[0] = (uint8_t*)malloc(16 + iPixels);
      m_videoBuffer.data[1] = (uint8_t*)malloc(16 + iChromaPixels);
      m_videoBuffer.data[2] = NULL;
      m_videoBuffer.data[3] = NULL;
      m_videoBuffer.format = RENDER_FMT_NV12;
    }
    else
    {
      CLog::Log(LOGERROR, "CDVDVideoCodecHybris:: Fixme unknown color_format(%d)", color_format);
      return;
    }
  }
/*
  if (width)
    m_videoBuffer.iWidth  = width;
  if (height)
    m_videoBuffer.iHeight = height;

  // picture display width/height include the cropping.
  m_videoBuffer.iDisplayWidth  = crop_right  + 1 - crop_left;
  m_videoBuffer.iDisplayHeight = crop_bottom + 1 - crop_top;
  if (m_hints.aspect > 1.0 && !m_hints.forced_aspect)
  {
    m_videoBuffer.iDisplayWidth  = ((int)lrint(m_videoBuffer.iHeight * m_hints.aspect)) & -3;
    if (m_videoBuffer.iDisplayWidth > m_videoBuffer.iWidth)
    {
      m_videoBuffer.iDisplayWidth  = m_videoBuffer.iWidth;
      m_videoBuffer.iDisplayHeight = ((int)lrint(m_videoBuffer.iWidth / m_hints.aspect)) & -3;
    }
  }
*/
  m_videoBuffer.iDisplayWidth = width;
  m_videoBuffer.iDisplayHeight = height;
}


static void on_client_died_cb(void *context)
{
  REPORT_FUNCTION();
      CLog::Log(LOGERROR, "%s::%s - %s", 
              CLASSNAME, __func__, 
        "decodingservice client died" );
}

void CDVDVideoCodecHybris::CallbackInitSurfaceTexture(void *userdata) {
  REPORT_FUNCTION();
  CDVDVideoCodecHybris *ctx = static_cast<CDVDVideoCodecHybris*>(userdata);
  ctx->InitSurfaceTexture();
}

void CDVDVideoCodecHybris::InitSurfaceTexture(void) {
  REPORT_FUNCTION();
  // We MUST create the GLES texture on the main thread
  // to match where the valid GLES context is located.
  // It would be nice to move this out of here, we would need
  // to create/fetch/create from g_RenderMananger. But g_RenderMananger
  // does not know we are using MediaCodec until Configure and we
  // we need m_surfaceTexture valid before then. Chicken, meet Egg.
  if (g_application.IsCurrentThread()) {
    // localize GLuint so we do not spew gles includes in our header
    GLuint texture_id;

    glGenTextures(1, &texture_id);
    glBindTexture(  GL_TEXTURE_EXTERNAL_OES, texture_id);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(  GL_TEXTURE_EXTERNAL_OES, 0);
    m_textureId = texture_id;
    CLog::Log(LOGNOTICE, "%s::%s::%d - m_textureId = %x", CLASSNAME, __func__, __LINE__,(unsigned int)m_textureId);

    //m_surfaceTexture = std::shared_ptr<CJNISurfaceTexture>(new CJNISurfaceTexture(m_textureId));
    decoding_service_init();
    decoding_service_set_client_death_cb(&on_client_died_cb, 123, static_cast<void*>(this));
    DSSessionWrapperHybris decoding_session = decoding_service_create_session(123);
    CLog::Log(LOGNOTICE, "%s::%s::%d - decoding_session = %x", CLASSNAME, __func__, __LINE__,(unsigned int)decoding_session);

    IGBCWrapperHybris igbc_wrapper = 
                              decoding_service_get_igraphicbufferconsumer();
    CLog::Log(LOGNOTICE, "%s::%s::%d - igbc_wrapper = %x", CLASSNAME, __func__, __LINE__,(unsigned int)igbc_wrapper);
    m_surfaceTexture = gl_consumer_create_by_id_with_igbc(m_textureId,
                              igbc_wrapper );
    CLog::Log(LOGNOTICE, "%s::%s::%d - m_surfaceTexture = %x", CLASSNAME, __func__, __LINE__,(unsigned int)m_surfaceTexture);
    // hook the surfaceTexture OnFrameAvailable callback
    m_frameAvailable = std::shared_ptr<CDVDMediaCodecOnFrameAvailable>(new CDVDMediaCodecOnFrameAvailable(m_surfaceTexture));
    CLog::Log(LOGNOTICE, "%s::%s::%d - m_frameAvailable = %x", CLASSNAME, __func__, __LINE__,(unsigned int)&(*m_frameAvailable));
    IGBPWrapperHybris igbp = decoding_service_get_igraphicbufferproducer();
    CLog::Log(LOGNOTICE, "%s::%s::%d - igbp = %x", CLASSNAME, __func__, __LINE__,(unsigned int)igbp);
    m_surface = surface_texture_client_create_by_igbp(igbp);
    CLog::Log(LOGNOTICE, "%s::%s::%d - m_surface = %x", CLASSNAME, __func__, __LINE__,(unsigned int)m_surface);
    surface_texture_client_set_hardware_rendering (m_surface, true);
  } else {
    ThreadMessageCallback callbackData;
    callbackData.callback = &CallbackInitSurfaceTexture;
    callbackData.userptr  = (void*)this;

    ThreadMessage msg;
    msg.dwMessage = TMSG_CALLBACK;
    msg.lpVoid = (void*)&callbackData;

    // wait for it.
    CApplicationMessenger::Get().SendMessage(msg, true);
  }
  return;
}

void CDVDVideoCodecHybris::ReleaseSurfaceTexture(void) {
  REPORT_FUNCTION();
  // it is safe to delete here even though these items
  // were created in the main thread instance
  //SAFE_DELETE(m_surface);
  //surface_texture_client_destroy(m_surface); mac_l1: crashes!
  //m_frameAvailable.reset();
  //m_surfaceTexture.reset();
  //mac_l1: doesnt have an ubuntu libhybrisized alternative (yet), dont care

  if (m_textureId > 0) {
    GLuint texture_id = m_textureId;
    glDeleteTextures(1, &texture_id);
    m_textureId = 0;
  }
}
