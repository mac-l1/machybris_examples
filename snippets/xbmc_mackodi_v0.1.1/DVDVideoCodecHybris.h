#pragma once

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

#include "DVDVideoCodec.h"
#include "DVDResource.h"
#include "DVDStreamInfo.h"
#include "utils/BitstreamConverter.h"
#include <string>
#include <queue>
#include <list>
#include "guilib/GraphicContext.h"

#include <hybris/media/media_compatibility_layer.h>
#include <hybris/media/media_codec_layer.h>
#include <hybris/media/surface_texture_client_hybris.h>

#include <vector>
#include <memory>
#include "threads/Thread.h"
#include "threads/SingleLock.h"

//compressed frame size. 1080p mpeg4 10Mb/s can be un to 786k in size, so this is to make sure frame fits into buffer
#define STREAM_BUFFER_SIZE (786432 * 8 * 4) /* mac_l1: 80Mps = 8, 2K = 2 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

typedef struct amc_demux {
  uint8_t *pData;
  int iSize;
  double dts;
  double pts;
} amc_demux;

//class CJNISurface;
//class CJNISurfaceTexture;
//class CJNIMediaCodec;
//class CJNIMediaFormat;
class CDVDMediaCodecOnFrameAvailable;
//class CJNIByteBuffer;
class CBitstreamConverter;

class CDVDMediaCodecInfo
{
public:
  CDVDMediaCodecInfo( int index,
                      unsigned int texture,
                      MediaCodecDelegate codec,
                      GLConsumerWrapperHybris surfacetexture,
//                    std::shared_ptr<CJNISurfaceTexture> &surfacetexture,
//                    CDVDMediaCodecOnFrameAvailable* frameready);
                      std::shared_ptr<CDVDMediaCodecOnFrameAvailable> &frameready);

  // reference counting
  CDVDMediaCodecInfo* Retain();
  long                Release();

  // meat and potatos
  void                Validate(bool state);
  // MediaCodec related
  void                ReleaseOutputBuffer(bool render);
  // SurfaceTexture released
  int                 GetIndex() const;
  int                 GetTextureID() const;
  void                GetTransformMatrix(float *textureMatrix);
  void                UpdateTexImage();

private:
  // private because we are reference counted
  virtual            ~CDVDMediaCodecInfo();

  long                m_refs;
  bool                m_valid;
  bool                m_isReleased;
  int                 m_index;
  unsigned int        m_texture;
  int64_t             m_timestamp;
  CCriticalSection    m_section;
  // shared_ptr bits, shared between
  // CDVDVideoCodecAndroidMediaCodec and LinuxRenderGLES.
  MediaCodecDelegate m_codec;
//std::shared_ptr<CJNISurfaceTexture> m_surfacetexture;
  GLConsumerWrapperHybris m_surfacetexture;
//std::shared_ptr<SurfaceTextureClientHybris> m_surfacetexture;
  std::shared_ptr<CDVDMediaCodecOnFrameAvailable> m_frameready;
};


class CDVDVideoCodecHybris : public CDVDVideoCodec
{
public:
  CDVDVideoCodecHybris();
  virtual ~CDVDVideoCodecHybris();
  virtual bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options);
  virtual void Dispose();
  virtual int Decode(BYTE* pData, int iSize, double dts, double pts);
  virtual void Reset();
  bool GetPictureCommon(DVDVideoPicture* pDvdVideoPicture);
  virtual bool GetPicture(DVDVideoPicture* pDvdVideoPicture);
  virtual bool ClearPicture(DVDVideoPicture* pDvdVideoPicture);
  virtual void SetDropState(bool bDrop);
  virtual const char* GetName() { return m_name.c_str(); }; // m_name is never changed after open
  virtual int     GetDataSize(void);
  virtual double  GetTimeSize(void);
  virtual unsigned GetAllowedReferences();

protected:
  void            FlushInternal(void);
  int GetOutputPicture(void);
  void OutputFormatChanged(void);

  std::string m_name;
  unsigned int m_iDecodedWidth;
  unsigned int m_iDecodedHeight;
  unsigned int m_iConvertedWidth;
  unsigned int m_iConvertedHeight;

  CBitstreamConverter *m_bitstream;

  std::priority_queue<double> m_pts;
  std::priority_queue<double> m_dts;

  bool m_drop;
  bool m_bVideoConvert;

  std::string m_mimeType;
  MediaCodecDelegate m_codec;
  MediaFormat m_format;
  CDVDStreamInfo m_hints;

  std::queue<amc_demux> m_demux;
  std::vector<CDVDMediaCodecInfo*> m_inflight;

  bool m_opened;

  DVDVideoPicture m_videoBuffer;
  bool            m_render_sw;
  int m_src_offset[4];
  int m_src_stride[4];

  //bool OpenDevices();

  // surface handling functions
  static void     CallbackInitSurfaceTexture(void*);
  void            InitSurfaceTexture(void);
  void            ReleaseSurfaceTexture(void);

  SurfaceTextureClientHybris    m_surface;
  unsigned int    m_textureId;
  // we need these as shared_ptr because CDVDVideoCodecAndroidMediaCodec
  // will get deleted before CLinuxRendererGLES is shut down and
  // CLinuxRendererGLES refs them via CDVDMediaCodecInfo.
  GLConsumerWrapperHybris m_surfaceTexture;
  //CDVDMediaCodecOnFrameAvailable* m_frameAvailable;
  std::shared_ptr<CDVDMediaCodecOnFrameAvailable> m_frameAvailable;
};

