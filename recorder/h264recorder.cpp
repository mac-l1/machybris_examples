#include <stdio.h>
#include "mindroid/os/Thread.h"
#include "mindroid/os/Lock.h"
using namespace mindroid;

#include "Timer.h"
//using std::string;
using namespace MaliSDK;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <android/libon2/vpu_api.h>

#include "fbtools.h"

char* movieFileName = NULL;

#include "hwc_copybit.h"
CopyBit copyBit;
void yuv2rgb(uint8_t* rgb, uint8_t *yuv420sp, int width, int height);
void rgb2yuv(uint8_t* yuv420sp, uint8_t *rgb, int width, int height);

#define WINDOW_W 1920
#define WINDOW_H 1080

int windowWidth = -1;
int windowHeight = -1;
int rgbWidth = -1;
int rgbHeight = -1;
int yuvWidth = -1;
int yuvHeight = -1;

uint8_t *rgbImage = NULL;
uint8_t *rgb16Image = NULL;
uint8_t *yuvImage = NULL;

#define VPU_DEMO_LOG(...)   do { fprintf(stdout, __VA_ARGS__);fprintf(stdout, "\n");fflush(stdout);  } while (0)
 
#define BSWAP32(x) \
    ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

typedef struct VpuApiEncInput {
    EncInputStream_t stream;
    RK_U32 capability;
}VpuApiEncInput;

FILE* pOutFile = NULL;
struct VpuCodecContext *ctx = NULL;
RK_S32 nal = 0x00000001;
RK_S32 frame_count, ret, size;
EncoderOut_t    enc_out_yuv;
EncoderOut_t *enc_out = NULL;
VpuApiEncInput enc_in_strm;
VpuApiEncInput *api_enc_in = &enc_in_strm;
EncInputStream_t *enc_in =NULL;
EncParameter_t *enc_param = NULL;
RK_S64 fakeTimeUs =0;

int vpu_encode_init( void ) {
    pOutFile = fopen(movieFileName, "wb");
    if (pOutFile == NULL) {
        VPU_DEMO_LOG("can not write output file\n");
        printf("ENCODE_ERR_RET(ERROR_INVALID_PARAM);\n");fflush(stdout);
        return 0;
    }

    memset(&enc_in_strm, 0, sizeof(VpuApiEncInput));
    enc_in = &enc_in_strm.stream;
    enc_in->buf = NULL;

    memset(&enc_out_yuv, 0, sizeof(EncoderOut_t));
    enc_out = &enc_out_yuv;
    enc_out->data = (RK_U8*)malloc(yuvWidth * yuvHeight);
    if (enc_out->data == NULL) {
        printf("ENCODE_ERR_RET(ERROR_MEMORY);\n");fflush(stdout);return 0;
    }

    ret = vpu_open_context(&ctx);
    if (ret || (ctx ==NULL)) {
        printf("ENCODE_ERR_RET(ERROR_MEMORY);\n");fflush(stdout);return 0;
    }

    ctx->codecType = CODEC_ENCODER;
    ctx->videoCoding = OMX_ON2_VIDEO_CodingAVC;
    ctx->width = yuvWidth;
    ctx->height = yuvHeight;
    ctx->no_thread = 1;

    ctx->private_data = malloc(sizeof(EncParameter_t));
    memset(ctx->private_data,0,sizeof(EncParameter_t));

    enc_param = (EncParameter_t*)ctx->private_data;
    enc_param->width = yuvWidth;
    enc_param->height = yuvHeight;
    enc_param->bitRate =  1000000; // 100000;
    enc_param->framerate = 25;
    enc_param->enableCabac   = 0;
    enc_param->cabacInitIdc  = 0;
    enc_param->intraPicRate  = 30;

    if ((ret = ctx->init(ctx, NULL, 0)) !=0) {
       VPU_DEMO_LOG("init vpu api context fail, ret: 0x%X", ret);
       printf("ENCODE_ERR_RET(ERROR_INIT_VPU);\n");fflush(stdout);return 0;
    }

    VPU_DEMO_LOG("encode init ok, sps len: %d", ctx->extradata_size);
    if(pOutFile && (ctx->extradata_size >0)) {
        VPU_DEMO_LOG("dump %d bytes enc output stream to file",
            ctx->extradata_size);
        /* save sps and pps */
        fwrite(ctx->extradata, 1, ctx->extradata_size, pOutFile);
        fflush(pOutFile);
    }

    RK_U32 w_align = ((ctx->width + 15) & (~15));
    RK_U32 h_align = ((ctx->height + 15) & (~15));
    size = w_align * h_align * 3/2;
    nal = BSWAP32(nal);
   
    return 1;
}

int vpu_encode_frame( void ) {
        if (enc_in && (enc_in->size ==0)) {
            if (enc_in->buf == NULL) {
                enc_in->buf = (RK_U8*)(malloc)(size);
                if (enc_in->buf == NULL) {
                    printf("ENCODE_ERR_RET(ERROR_MEMORY);\n");fflush(stdout);
                    return 0;
                }
                api_enc_in->capability = size;
            }

            if (api_enc_in->capability <((RK_U32)size)) {
                enc_in->buf = (RK_U8*)(realloc)((void*)(enc_in->buf), size);
                if (enc_in->buf == NULL) {
                    printf("ENCODE_ERR_RET(ERROR_MEMORY);\n");fflush(stdout);
                    return 0;
                }
                api_enc_in->capability = size;
            }

            memcpy(enc_in->buf, yuvImage, size );
            {
                enc_in->size = size;
                enc_in->timeUs = fakeTimeUs;
                fakeTimeUs += 40000;
            }

            //VPU_DEMO_LOG("read one frame, size: %d, timeUs: %lld ",
            //    enc_in->size, enc_in->timeUs  );
        }

        if ((ret = ctx->encode(ctx, enc_in, enc_out)) !=0) {
            printf("ENCODE_ERR_RET(ERROR_VPU_ENCODE);\n");fflush(stdout);
            return 0;
        } else {
            printf("\rvpu encode frame [%d], out len: %d, "
                   "\ttimeUs: %lld", frame_count,
                enc_out->size, enc_in->timeUs ); fflush(stdout);

            /*
             ** encoder output stream is raw bitstream, you need to add nal
             ** head by yourself.
            */
            if ((enc_out->size) && (enc_out->data)) {
                if(pOutFile) {
                    //VPU_DEMO_LOG("dump %d bytes enc output stream to file",
                    //    enc_out->size);
                    fwrite((uint8_t*)&nal, 1, 4, pOutFile);
                    fwrite(enc_out->data, 1, enc_out->size, pOutFile);
                    fflush(pOutFile);
                }

                enc_out->size = 0;
            }
        }
        //usleep(30);

    return 1;
}

void vpu_encode_exit( void ) {
    if (enc_in && enc_in->buf) {
        free(enc_in->buf);
        enc_in->buf = NULL;
    }
    if (enc_out && (enc_out->data)) {
        free(enc_out->data);
        enc_out->data = NULL;
    }
    if (ctx) {
        if (ctx->private_data) {
            free(ctx->private_data);
            ctx->private_data = NULL;
        }
        //vpu_close_context(&ctx);
        ctx = NULL;
    }
    if (pOutFile) {
        fclose(pOutFile);
        pOutFile = NULL;
    }
    return;
}

void rga_blit(uint8_t* pdst, int dst_format, uint8_t* psrc, int src_format, 
              int width, int height) {
    struct _rga_img_info_t src, dst;
    memset(&src, 0, sizeof(struct _rga_img_info_t));
    memset(&dst, 0, sizeof(struct _rga_img_info_t));
   
    src.yrgb_addr = (int)psrc;
    src.vir_w = (width + 15)&(~15);
    src.vir_h = (height + 15)&(~15);
    src.format = src_format;
    src.act_w = width;
    src.act_h = height;
    src.x_offset = 0;
    src.y_offset = 0;

    dst.yrgb_addr = (uint32_t)pdst;
    dst.vir_w = (width + 31)&(~31);
    dst.vir_h = (height + 15)&(~15);
    dst.format = dst_format;
    dst.act_w = width;
    dst.act_h = height;
    dst.x_offset = 0;
    dst.y_offset = 0;

    int ret = copyBit.draw(&src, &dst, RK_MMU_SRC_ENABLE | RK_MMU_DST_ENABLE | RK_BT_601_MPEG | RK_RGB2YUV);
    return;
}

void yuv2rgb(uint8_t* rgb, uint8_t *yuv420sp, int width, int height) {
    return rga_blit(rgb,RK_FORMAT_RGBA_8888,yuv420sp,RK_FORMAT_YCbCr_420_SP,
                    width,height);
}

void rgb2yuv(uint8_t *yuv420sp, uint8_t* rgb, int width, int height) {
    return rga_blit(yuv420sp,RK_FORMAT_YCbCr_420_SP,rgb,RK_FORMAT_RGBA_8888,
                    width,height);
}

void rgb2rgb16(uint8_t *rgb16, uint8_t* rgb, int width, int height) {
    return rga_blit(rgb16,RK_FORMAT_RGB_565,rgb,RK_FORMAT_RGBA_8888,
                    width,height);
}

void rgb162yuv(uint8_t *yuv420sp, uint8_t* rgb, int width, int height) {
    return rga_blit(yuv420sp,RK_FORMAT_YCbCr_420_SP,rgb,RK_FORMAT_RGB_565,
                    width,height);
}
 
void yuv2rgb16(uint8_t *dst, uint8_t* src, int width, int height) {
    return rga_blit(dst,RK_FORMAT_RGB_565,src,RK_FORMAT_YCbCr_420_SP,
                    width,height);
}
 
void rgb162rgb(uint8_t *dst, uint8_t* src, int width, int height) {
    return rga_blit(dst,RK_FORMAT_RGBA_8888,src,RK_FORMAT_RGB_565,
                    width,height);
}

int write_yuv( uint8_t* data, int width,  int height, int frame_count ) {
  char filename[256];
  sprintf( filename, "capture.%d.nv21", frame_count );
  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) return 0;
  printf("write %d frame(%dx%d) data to %s\n", frame_count, width, height, filename );
  int wAlign16 = (width+ 15) & (~15);
  int hAlign16 = (height + 15) & (~15);
  int frameSize = wAlign16*hAlign16*3/2;
  fwrite((uint8_t*)data, 1, frameSize, fp);
  (void) fclose(fp);
  return 1;
}

int write_ppm( int* data, int width,  int height, int frame_count ) {
  char filename[256];
  sprintf( filename, "capture.%d.ppm", frame_count );
  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) return 0;
  printf("write %d frame(%dx%d) data to %s\n", frame_count, width, height, filename );

  int x, y;
  (void) fprintf(fp, "P6\n%d %d\n255\n", width, height);
  for (y = 0; y < height; ++y)
  {
    for (x = 0; x < width; ++x)
    {
      char r = (data[x+(y*width)] & 0x000000ff);
      char g = (data[x+(y*width)] & 0x0000ff00) >> 8;
      char b = (data[x+(y*width)] & 0x00ff0000) >> 16;
      char a = (data[x+(y*width)] & 0xff000000) >> 24;
      putc((int)(r & 0xff),fp);
      putc((int)(g & 0xff),fp);
      putc((int)(b & 0xff),fp);
    }
  }
  (void) fclose(fp);
  return 1;
}

int write_tga( int* data, int width,  int height, int frame_count ) {
    char filename[256];
    sprintf( filename, "capture.%d.tga", frame_count );
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) return 0;

    printf("write %d frame(%dx%d) data to %s\n", frame_count, width, height, filename );
    char header[ 18 ] = { 0 }; // char = byte
    header[ 2 ] = 2; // 
    header[ 12 ] = width & 0xff;
    header[ 13 ] = (width >> 8) & 0xff;
    header[ 14 ] = height & 0xff;
    header[ 15 ] = (height >> 8) & 0xff;
    header[ 16 ] = 24; // bits per pixel
    fwrite((const char*)&header, 1, sizeof(header), fp);

    int x,y;
    for (y = height -1; y >= 0; y--)
        for (x = 0; x < width; x++) {
            char r = (data[x+(y*width)] & 0x000000ff);
            char g = (data[x+(y*width)] & 0x0000ff00) >> 8;
            char b = (data[x+(y*width)] & 0x00ff0000) >> 16;
            char a = (data[x+(y*width)] & 0xff000000) >> 24;
            putc((int)(r & 0xff),fp);
            putc((int)(g & 0xff),fp);
            putc((int)(b & 0xff),fp);
        }

    static const char footer[ 26 ] =
        "\0\0\0\0" // no extension area
        "\0\0\0\0" // no developer directory
        "truevision-xfile" // yep, this is a tga file
        ".";
    fwrite((const char*)&footer, 1, sizeof(footer), fp);

    fclose(fp);
    return 1;
}

bool end = false;
int fbMain(void) {
    windowWidth = fb_var.xres;
    windowHeight = fb_var.yres;
    rgbWidth = (windowWidth + 31)&(~31);
    rgbHeight = (windowHeight + 15)&(~15);
    yuvWidth = (windowWidth + 15)&(~15);
    yuvHeight = (windowHeight + 15)&(~15);
  
    rgbImage = (uint8_t*)malloc(rgbWidth*rgbHeight*4);
    rgb16Image = (uint8_t*)malloc(rgbWidth*rgbHeight*2);
    yuvImage = (uint8_t*)malloc(yuvWidth*yuvHeight*3/2);

    int frame_count = 0;
    Timer fpsTimer;
    fpsTimer.reset();
    end = false;

    if( !vpu_encode_init() ) end = true;
    while(!end)
    {
        float FPS = fpsTimer.getFPS();
        if(fpsTimer.isTimePassed(1.0f)) {
            printf(", FPS: %.1f", FPS);
        }
        //fb_waitforsync();
        //fb_waitforsync();

        rgb2yuv(yuvImage, fb_mem, windowWidth, windowHeight);
        frame_count++;
        if( !vpu_encode_frame() ) { end = true; break; }
/* 
        if((frame_count%100)==0)
        {
             write_ppm((int*)fb_mem,windowWidth,windowHeight,frame_count);
             write_yuv(yuvImage,yuvWidth,yuvHeight,frame_count);
             yuv2rgb(rgbImage, yuvImage, windowWidth, windowHeight);
             write_tga((int*)rgbImage,rgbWidth,rgbHeight,frame_count);
        }
*/
    }

    vpu_encode_exit();
    if(!rgbImage) free(rgbImage);
    rgbImage = NULL;
    if(!yuvImage) free(yuvImage);
    yuvImage = NULL;

    return 0;
}

//===========================================================================

class CaptureThread : public Thread {
    virtual void run() {
        fbMain();
    }
};

/*
class DecoderThread : public Thread {
	virtual void run() {
		decodeMain();
	}
};
*/
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int mygetch ( void ) 
{
  int ch;
  struct termios oldt, newt;
  
  tcgetattr ( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr ( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr ( STDIN_FILENO, TCSANOW, &oldt );
  
  return ch;
}

int main(int argc, char *argv[]) {
    if(argc == 2) {
        movieFileName = argv[1]; 
    } else {
        printf("usage: %s filename\n",argv[0]); return -1;
    }

    if( !fb_init((char*)"/dev/fb0", NULL )) {
        printf("fb_init() failed\n"); return -1;
    }

    sleep(1);

    sp<CaptureThread> captureThread = new CaptureThread();
    captureThread->start();

    sleep(1);

    mygetch();
    end = true;

    captureThread->join();

    fb_cleanup();

    printf("done!\n");
    return 0;
}
