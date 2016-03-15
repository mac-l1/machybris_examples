/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "RkOn2Encoder"
#include <utils/Log.h>
#include "RkOn2Encoder.h"
#include <media/stagefright/TimeSource.h>

#include "OMX_Video.h"
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

namespace android {
typedef enum
{
    AVC_BASELINE = 66,
    AVC_MAIN = 77,
    AVC_EXTENDED = 88,
    AVC_HIGH = 100,
    AVC_HIGH10 = 110,
    AVC_HIGH422 = 122,
    AVC_HIGH444 = 144
} AVCProfile;

/**
This enumeration is for levels. The value follows the level_idc in sequence
parameter set rbsp. See Annex A.
@published All
*/
typedef enum
{
    AVC_LEVEL_AUTO = 0,
    AVC_LEVEL1_B = 9,
    AVC_LEVEL1 = 10,
    AVC_LEVEL1_1 = 11,
    AVC_LEVEL1_2 = 12,
    AVC_LEVEL1_3 = 13,
    AVC_LEVEL2 = 20,
    AVC_LEVEL2_1 = 21,
    AVC_LEVEL2_2 = 22,
    AVC_LEVEL3 = 30,
    AVC_LEVEL3_1 = 31,
    AVC_LEVEL3_2 = 32,
    AVC_LEVEL4 = 40,
    AVC_LEVEL4_1 = 41,
    AVC_LEVEL4_2 = 42,
    AVC_LEVEL5 = 50,
    AVC_LEVEL5_1 = 51
} AVCLevel;

static status_t ConvertOmxAvcProfileToAvcSpecProfile(
        int32_t omxProfile, AVCProfile* pvProfile) {
    ALOGV("ConvertOmxAvcProfileToAvcSpecProfile: %d", omxProfile);
    switch (omxProfile) {
        case OMX_VIDEO_AVCProfileBaseline:
            *pvProfile = AVC_BASELINE;
            return OK;
        default:
            ALOGE("Unsupported omx profile: %d", omxProfile);
    }
    return BAD_VALUE;
}

static status_t ConvertOmxAvcLevelToAvcSpecLevel(
        int32_t omxLevel, AVCLevel *pvLevel) {
    ALOGV("ConvertOmxAvcLevelToAvcSpecLevel: %d", omxLevel);
    AVCLevel level = AVC_LEVEL5_1;
    switch (omxLevel) {
        case OMX_VIDEO_AVCLevel1:
            level = AVC_LEVEL1_B;
            break;
        case OMX_VIDEO_AVCLevel1b:
            level = AVC_LEVEL1;
            break;
        case OMX_VIDEO_AVCLevel11:
            level = AVC_LEVEL1_1;
            break;
        case OMX_VIDEO_AVCLevel12:
            level = AVC_LEVEL1_2;
            break;
        case OMX_VIDEO_AVCLevel13:
            level = AVC_LEVEL1_3;
            break;
        case OMX_VIDEO_AVCLevel2:
            level = AVC_LEVEL2;
            break;
        case OMX_VIDEO_AVCLevel21:
            level = AVC_LEVEL2_1;
            break;
        case OMX_VIDEO_AVCLevel22:
            level = AVC_LEVEL2_2;
            break;
        case OMX_VIDEO_AVCLevel3:
            level = AVC_LEVEL3;
            break;
        case OMX_VIDEO_AVCLevel31:
            level = AVC_LEVEL3_1;
            break;
        case OMX_VIDEO_AVCLevel32:
            level = AVC_LEVEL3_2;
            break;
        case OMX_VIDEO_AVCLevel4:
            level = AVC_LEVEL4;
            break;
        case OMX_VIDEO_AVCLevel41:
            level = AVC_LEVEL4_1;
            break;
        case OMX_VIDEO_AVCLevel42:
            level = AVC_LEVEL4_2;
            break;
        case OMX_VIDEO_AVCLevel5:
            level = AVC_LEVEL5;
            break;
        case OMX_VIDEO_AVCLevel51:
            level = AVC_LEVEL5_1;
            break;
        default:
            ALOGE("Unknown omx level: %d", omxLevel);
            return BAD_VALUE;
    }
    *pvLevel = level;
    return OK;
}

struct CodeMap {
   OMX_ON2_VIDEO_CODINGTYPE codec_id;
   const char *mime;
};

static const CodeMap kEncCodeMap[] = {
   { OMX_ON2_VIDEO_CodingAVC,   MEDIA_MIMETYPE_VIDEO_AVC},
   { OMX_ON2_VIDEO_CodingMJPEG, MEDIA_MIMETYPE_VIDEO_MJPEG},
   { OMX_ON2_VIDEO_CodingVP8,   MEDIA_MIMETYPE_VIDEO_VP8},
};

RkOn2Encoder::RkOn2Encoder(
        const sp<MediaSource>& source,
        const sp<MetaData>& meta)
    : mSource(source),
      mMeta(meta),
      mNumInputFrames(-1),
      mPrevTimestampUs(-1),
      mPrev_IDR_TimestampUs(-1),
      mStarted(false),
      mInputBuffer(NULL),
      mInputFrameData(NULL),
      mGroup(NULL) ,
      skipFlag(false),
      totaldealt(0) ,
      mIDR_Interval_time(2000000ll),
      mCabac_flag(0),
      mFramesIntervalSec(0),
      mLevel(0),
      mProfile(0),
      mVpuCtx(NULL),
      mCabacInitIdc(0){
    mCodecId = OMX_ON2_VIDEO_CodingUnused;
    ALOGV("Construct software RkOn2Encoder");
    wimo_flag = 0;
    const char *mime;
    mInitCheck = initCheck(meta);
    meta->findCString(kKeyMIMEType, &mime);
    int32_t kNumMapEntries = sizeof(kEncCodeMap) / sizeof(kEncCodeMap[0]);
    for(int i = 0; i < kNumMapEntries; i++){
        if(!strcmp(kEncCodeMap[i].mime, mime)){
            mCodecId = kEncCodeMap[i].codec_id;
            ALOGD("mCodecId = %d video code mine %s",mCodecId,mime);
            break;
        }
    }
    int ret = vpu_open_context(&mVpuCtx);
    if (ret || (mVpuCtx ==NULL)) {
       mInitCheck = !OK;
    }
    mFormat->setCString(kKeyMIMEType, mime);
}

RkOn2Encoder::~RkOn2Encoder() {
    ALOGV("Destruct software RkOn2Encoder");
    if (mStarted) {
        stop();
    }
    vpu_close_context(&mVpuCtx);
}

status_t RkOn2Encoder::initCheck(const sp<MetaData>& meta) {
    ALOGV("initCheck");
    CHECK(meta->findInt32(kKeyWidth, &mVideoWidth));
    CHECK(meta->findInt32(kKeyHeight, &mVideoHeight));
    CHECK(meta->findInt32(kKeyFrameRate, &mVideoFrameRate));
    CHECK(meta->findInt32(kKeyBitRate, &mVideoBitRate));
    CHECK(meta->findInt32(kKeyIFramesInterval, &mFramesIntervalSec));

    mProfile = AVC_BASELINE;
    mLevel = AVC_LEVEL3_2;

    int32_t profile,level;
    if(meta->findInt32(kKeyVideoProfile, &profile)){
        ConvertOmxAvcProfileToAvcSpecProfile(profile,(AVCProfile*)&mProfile);
    }

    if(meta->findInt32(kKeyVideoLevel, &level)){
        ConvertOmxAvcLevelToAvcSpecLevel(level,(AVCLevel*)&mLevel);
    }
    mFormat = new MetaData;
    mFormat->setInt32(kKeyWidth, mVideoWidth);
    mFormat->setInt32(kKeyHeight, mVideoHeight);
    mFormat->setInt32(kKeyBitRate, mVideoBitRate);
    mFormat->setInt32(kKeyFrameRate, mVideoFrameRate);
    mFormat->setInt32(kKeyColorFormat, mVideoColorFormat);
    mFormat->setCString(kKeyDecoderComponent, "RkOn2Encoder");
    return OK;
}

status_t RkOn2Encoder::start(MetaData *params) {
    ALOGV("start");
    if (mInitCheck != OK) {
        return mInitCheck;
    }

    if (mStarted) {
        ALOGW("Call start() when encoder already started");
        return OK;
    }


    mVpuCtx->width = mVideoWidth;
    mVpuCtx->height = mVideoHeight;
    mVpuCtx->videoCoding = mCodecId;
    mVpuCtx->codecType = CODEC_ENCODER;
    mVpuCtx->private_data = malloc(sizeof(EncParameter_t));
    memset(mVpuCtx->private_data,0,sizeof(EncParameter_t));
    EncParameter_t *EncParam = (EncParameter_t*)mVpuCtx->private_data;
    EncParam->height = mVideoHeight;
	EncParam->width = mVideoWidth;
	EncParam->bitRate = mVideoBitRate;
	EncParam->framerate = mVideoFrameRate;
    EncParam->enableCabac 	= mCabac_flag;
    EncParam->cabacInitIdc 	= mCabacInitIdc;
    EncParam->format          = 0;
    EncParam->intraPicRate      = mFramesIntervalSec*mVideoFrameRate;
    EncParam->levelIdc = mLevel;
    EncParam->profileIdc = mProfile;

    if(mVpuCtx->init(mVpuCtx,NULL,0)){
        return !OK;
    }

    mGroup = new MediaBufferGroup();
    int32_t maxSize;

	maxSize = 38135*40;

    mGroup->add_buffer(new MediaBuffer(maxSize));

    mSource->start(params);
    mNumInputFrames = -2;  // 1st two buffers contain SPS and PPS
    mStarted = true;
    mSpsPpsHeaderReceived = false;
    mReadyForNextFrame = true;
    mIsIDRFrame = 0;
    ALOGV("start out");
    return OK;
}

status_t RkOn2Encoder::stop() {
    ALOGV("stop");
    if (!mStarted) {
        ALOGW("Call stop() when encoder has not started");
        return OK;
    }
    ALOGV("stop1");

    if (mInputBuffer) {
        mInputBuffer->release();
        mInputBuffer = NULL;
    }
    ALOGV("stop2");

    if (mGroup) {
        delete mGroup;
        mGroup = NULL;
    }
	ALOGV("stop3");

    mSource->stop();
	    ALOGV("stop8");
    releaseOutputBuffers();
	    ALOGV("stop9");
    mStarted = false;

    return OK;
}

void RkOn2Encoder::releaseOutputBuffers() {
    ALOGV("releaseOutputBuffers");
    for (size_t i = 0; i < mOutputBuffers.size(); ++i) {
        MediaBuffer *buffer = mOutputBuffers.editItemAt(i);
        buffer->setObserver(NULL);
        buffer->release();
    }
    mOutputBuffers.clear();
}

sp<MetaData> RkOn2Encoder::getFormat() {
    ALOGV("getFormat");
    return mFormat;
}


status_t RkOn2Encoder::get_encoder_param(void *param)
{
	EncParameter_t* vpug = (EncParameter_t*) param;
	ALOGD("get_encoder_param ");
	if(mVpuCtx == NULL)
	{
		ALOGE("Encoder is not initalied yet");
		return UNKNOWN_ERROR;
	}
	else
        mVpuCtx->control(mVpuCtx,VPU_API_ENC_GETCFG,(void*)vpug);
	return OK;
}

void RkOn2Encoder::Set_IDR_Frame()
{
    mVpuCtx->control(mVpuCtx,VPU_API_ENC_SETIDRFRAME,NULL);
}

void RkOn2Encoder::Set_IDR_Interval(int64_t interval_time)
{
	mIDR_Interval_time = interval_time;
}

void RkOn2Encoder::Set_Flag(int flag,int cabac_flag)
{
	wimo_flag = flag;
	mCabac_flag = cabac_flag;
	if(mCabac_flag==1)
		mCabacInitIdc = 2;
	else
		mCabacInitIdc = 0;
ALOGD("AVCEncoder::Set_Flag %d mCabac_flag %d mCabacInitIdc %d",flag,mCabac_flag , mCabacInitIdc);
}

status_t RkOn2Encoder::set_encoder_param(void *param)
{
	EncParameter_t* vpug = (EncParameter_t*)param;
	if(mVpuCtx == NULL)
	{
		ALOGE("Encoder is not initalied yet");
		return UNKNOWN_ERROR;
	}
	else
	{
		if(vpug->width != mVideoWidth || vpug->height != mVideoHeight)
		{
		ALOGI("reset encoder vpug->width %d mVideoWidth %d vpug->height %d mVideoHeight %d",vpug->width , mVideoWidth,
				vpug->height ,mVideoHeight);
		}

		if(mVideoFrameRate != vpug->framerate)
		{
			mVideoFrameRate = vpug->framerate;
			mFormat->setInt32(kKeyFrameRate, mVideoFrameRate);

		}
		if(mVideoBitRate != vpug->bitRate)
		{
			mFormat->setInt32(kKeyBitRate, mVideoBitRate);
    		mVideoBitRate = vpug->bitRate;

		}
        mVpuCtx->control(mVpuCtx,VPU_API_ENC_SETCFG,(void*)&vpug);
	}
	return OK;
}
status_t RkOn2Encoder::read(
        MediaBuffer **out, const ReadOptions *options) {

	ALOGV("AVCEncoder :: read");
    //CHECK(!options);
    *out = NULL;
	ALOGV("read in");
    MediaBuffer *outputBuffer;
    mGroup->acquire_buffer(&outputBuffer);
    uint8_t *outPtr = (uint8_t *) outputBuffer->data();
    uint32_t dataLength = outputBuffer->size();

    int32_t type;

    // Combine SPS and PPS and place them in the very first output buffer
    // SPS and PPS are separated by start code 0x00000001
    // Assume that we have exactly one SPS and exactly one PPS.
    ALOGV("read in1");
    while (!mSpsPpsHeaderReceived && mNumInputFrames <= 0) {
		ALOGV("read in 2");
		if(wimo_flag)
		{
			EncParameter_t vpug;
            mVpuCtx->control(mVpuCtx,VPU_API_ENC_GETCFG,(void*)&vpug);
			vpug.rc_mode = 1;
            mVpuCtx->control(mVpuCtx,VPU_API_ENC_SETCFG,(void*)&vpug);
			ALOGD("wimo rc_mode111 %d ",vpug.rc_mode );
		}
        if (mMeta->findInt32(kKeyColorFormat, &mVideoColorFormat)) {
            ALOGI("AVCEncoder input colorFormat: %d", mVideoColorFormat);
            if (mVideoColorFormat == OMX_COLOR_FormatYUV420Planar) {
                if (mVpuCtx) {
                    H264EncPictureType encType = VPU_H264ENC_YUV420_PLANAR;
                    mVpuCtx->control(mVpuCtx,VPU_API_ENC_SETFORMAT,(void*)&encType);
                }
            }
        }

        if(mVpuCtx){
            memcpy(outPtr,mVpuCtx->extradata,mVpuCtx->extradata_size);
            dataLength = mVpuCtx->extradata_size;
        }else{
            return !OK;
        }
        mNumInputFrames = 0;
        outputBuffer->set_range(0,dataLength);
        outputBuffer->meta_data()->setInt32(kKeyIsCodecConfig, 1);
        outputBuffer->meta_data()->setInt64(kKeyTime, 0);
        mSpsPpsHeaderReceived = true;
        *out = outputBuffer;
        ALOGV("read out1");
        return OK;
    }

    // Get next input video frame
    if (mReadyForNextFrame) {
		    ALOGV("read 11");

		ALOGV("read 12");
		SystemTimeSource time;
		int64_t now = time.getRealTimeUs();
        status_t err = mSource->read(&mInputBuffer, options);
		int64_t delta = time.getRealTimeUs() - now;
		ALOGV("------>read need time %lld err %d",delta,err);
        if (err != OK) {
            ALOGV("Failed to read input video frame: %d", err);
            outputBuffer->release();
            return err;
        }
        int64_t timeUs;
        int32_t busaddress = 0;
        CHECK(mInputBuffer->meta_data()->findInt64(kKeyTime, &timeUs));
        mInputBuffer->meta_data()->findInt32(kKeyBusAdds, &busaddress);
        outputBuffer->meta_data()->setInt64(kKeyTime, timeUs);
        outputBuffer->meta_data()->setInt64(kKeyDecodingTime, timeUs);
		ALOGV("read 14");

        // When the timestamp of the current sample is the same as
        // that of the previous sample, the encoding of the sample
        // is bypassed, and the output length is set to 0.
        if (mNumInputFrames >= 1 && mPrevTimestampUs == timeUs) {
            // Frame arrives too late
            mInputBuffer->release();
            mInputBuffer = NULL;
            outputBuffer->set_range(0, 0);
            *out = outputBuffer;
			ALOGV("read 15");
            return OK;
        }

        // Don't accept out-of-order samples
        if(mPrevTimestampUs >= timeUs){
            ALOGD("mPrevTimestampUs %lld timeUs %lld");
        }

        CHECK(mPrevTimestampUs < timeUs);
        mPrevTimestampUs = timeUs;
    	int videoformat = 1;
    	int inputadd;
    	static int frame_num = 0;
    	if(mInputBuffer->meta_data()->findInt32(kKeyColorFormat, &videoformat))//;	//0yuv420sp, 1,abgr, 2 rgb565
    	{
            H264EncPictureType encType = VPU_H264ENC_YUV420_PLANAR;
            if(videoformat == OMX_COLOR_FormatYUV420Planar)
                encType = VPU_H264ENC_YUV420_PLANAR;
    		else if(videoformat == OMX_COLOR_FormatYUV420SemiPlanar)
    		    encType = VPU_H264ENC_YUV420_SEMIPLANAR;
    		else if(videoformat == OMX_COLOR_Format16bitRGB565)
    			encType = VPU_H264ENC_RGB565;
    		else if(videoformat == OMX_COLOR_Format32bitBGRA8888)
    			encType = VPU_H264ENC_RGB888;
    		else if(videoformat == OMX_COLOR_Format32bitARGB8888)
    			encType = VPU_H264ENC_BGR888;
            mVpuCtx->control(mVpuCtx,VPU_API_ENC_SETFORMAT,(void*)&encType);
    	}
		uint8_t *inputData = (uint8_t *) mInputBuffer->data();
		uint32_t inputLen = mInputBuffer->size();

		bool SyncFlag = false;
		uint32_t outPutTime = (uint32_t)mPrevTimestampUs;
		ALOGV("input len %d outbuflen %d",inputLen,dataLength);

		now = time.getRealTimeUs();
		if(wimo_flag == 1)
		{
			if(((mNumInputFrames % 5 ==0) && mNumInputFrames < 30) || (timeUs - mPrev_IDR_TimestampUs > mIDR_Interval_time))
			{
			    ALOGD("intra frame is encoded mPrev_IDR_TimestampUs %lld timeus %lld wimo_flag %d",mPrev_IDR_TimestampUs, timeUs,wimo_flag);
                mVpuCtx->control(mVpuCtx,VPU_API_ENC_SETIDRFRAME,NULL);
			}
		}
        EncInputStream_t input;
        EncoderOut_t     outPut;
        input.size = inputLen;
        if(busaddress){
            input.bufPhyAddr = busaddress;
            input.buf = NULL;
        }else{
            input.bufPhyAddr = 0;
            input.buf = inputData;
        }
        memset(&outPut,0,sizeof(EncoderOut_t));
        outPut.data = outPtr;
		int ret = mVpuCtx->encode(mVpuCtx,&input,&outPut);
        /*delta = time.getRealTimeUs() - now;
        delta = (delta/1000 - 25);
        if(delta > 0)
        {
            totaldealt += delta;
        }
		if(totaldealt > 25)
		{
            totaldealt = 0;
        }*/
        if(outPut.size > 200000) {
            ALOGV("ret %d datalen %d",ret,dataLength);
        }
		if (ret < 0 )
		{
			outputBuffer->release();
		    ALOGD("encoder error ret %d",ret);
			return ret;
		}
		if(((int32_t)outPut.size) <0)
		{
			outputBuffer->release();
			ALOGE("Avcencoder dataLength < 0 %d SyncFlag %d %2x%2x%2x%2x%2x%2x%2x%2x",dataLength,SyncFlag
				,outPtr[0],outPtr[1],outPtr[2],outPtr[3],outPtr[4],outPtr[5],outPtr[6],outPtr[7]);
			return UNKNOWN_ERROR;
		}
		ALOGV("ret %d datalen %d",ret,dataLength);

#ifdef ALL_IDR_FRAME
		mIsIDRFrame = outPut.keyFrame?1:0;
#else
		mIsIDRFrame = 0;
		if(outPut.keyFrame)
			mIsIDRFrame = 1;
#endif
		if(wimo_flag != NULL)
		{
			if(mIsIDRFrame == 1)
			{
				mPrev_IDR_TimestampUs =  timeUs;
			ALOGV("mIsIDRFrame %d timeUs %lld ",timeUs);
			}
		}
		outputBuffer->set_range(0, outPut.size);
		++mNumInputFrames;

		outputBuffer->meta_data()->setInt32(kKeyIsSyncFrame, mIsIDRFrame);
		*out = outputBuffer;
		mReadyForNextFrame = true;
        if (mInputBuffer) {
            mInputBuffer->release();
            mInputBuffer = NULL;
        }
		ALOGV("read 17");
    }
	ALOGV("read out2");
    return OK;
}

void RkOn2Encoder::signalBufferReturned(MediaBuffer *buffer) {

}

}  // namespace android
