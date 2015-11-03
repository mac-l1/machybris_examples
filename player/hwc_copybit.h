#ifndef __RGA_H__
#define __RGA_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*
//          Alpha    Red     Green   Blue  
{  4, 32, {{32,24,   8, 0,  16, 8,  24,16 }}, GGL_RGBA },   // RK_FORMAT_RGBA_8888    
{  4, 24, {{ 0, 0,   8, 0,  16, 8,  24,16 }}, GGL_RGB  },   // RK_FORMAT_RGBX_8888    
{  3, 24, {{ 0, 0,   8, 0,  16, 8,  24,16 }}, GGL_RGB  },   // RK_FORMAT_RGB_888
{  4, 32, {{32,24,  24,16,  16, 8,   8, 0 }}, GGL_BGRA },   // RK_FORMAT_BGRA_8888
{  2, 16, {{ 0, 0,  16,11,  11, 5,   5, 0 }}, GGL_RGB  },   // RK_FORMAT_RGB_565        
{  2, 16, {{ 1, 0,  16,11,  11, 6,   6, 1 }}, GGL_RGBA },   // RK_FORMAT_RGBA_5551    
{  2, 16, {{ 4, 0,  16,12,  12, 8,   8, 4 }}, GGL_RGBA },   // RK_FORMAT_RGBA_4444
{  3, 24, {{ 0, 0,  24,16,  16, 8,   8, 0 }}, GGL_BGR  },   // RK_FORMAT_BGB_888

*/
enum _RGA_FORMAT {
	RK_FORMAT_RGBA_8888    = 0x0,
    RK_FORMAT_RGBX_8888    = 0x1,
    RK_FORMAT_RGB_888      = 0x2,
    RK_FORMAT_BGRA_8888    = 0x3,
    RK_FORMAT_RGB_565      = 0x4,
    RK_FORMAT_RGBA_5551    = 0x5,
    RK_FORMAT_RGBA_4444    = 0x6,
    RK_FORMAT_BGR_888      = 0x7,
    
    RK_FORMAT_YCbCr_422_SP = 0x8,    
    RK_FORMAT_YCbCr_422_P  = 0x9,    
    RK_FORMAT_YCbCr_420_SP = 0xa,    
    RK_FORMAT_YCbCr_420_P  = 0xb,

    RK_FORMAT_YCrCb_422_SP = 0xc,    
    RK_FORMAT_YCrCb_422_P  = 0xd,    
    RK_FORMAT_YCrCb_420_SP = 0xe,    
    RK_FORMAT_YCrCb_420_P  = 0xf,
    
    RK_FORMAT_BPP1         = 0x10,
    RK_FORMAT_BPP2         = 0x11,
    RK_FORMAT_BPP4         = 0x12,
    RK_FORMAT_BPP8         = 0x13,
    RK_FORMAT_UNKNOWN       = 0x100, 
};

#define FLAG_SCALE_SHIFT		0
#define FLAG_SCALE_MASK			0x0F
enum _RGA_SCALE_MODE {
	RK_NEAREST		= 0 << FLAG_SCALE_SHIFT,
	RK_BILNEAR		= 1 << FLAG_SCALE_SHIFT,
	RK_BICUBIC		= 2 << FLAG_SCALE_SHIFT
};

#define FLAG_YUV2RGB_SHIFT	4
#define FLAG_YUV2RGB_MASK	(0x0F << FLAG_YUV2RGB_SHIFT)
enum _RGA_YUV2RGBMODE
{
    RK_BT_601_MPEG            = 0x0 << FLAG_YUV2RGB_SHIFT,     /* BT.601 MPEG */
    RK_BT_601_JPEG            = 0x1 << FLAG_YUV2RGB_SHIFT,     /* BT.601 JPEG */
    RK_BT_709		          = 0x2 << FLAG_YUV2RGB_SHIFT,     /* BT.709      */
};

#define FLAG_ROTATION_SHIFT	8
#define FLAG_ROTATION_MASK	(0xFF << FLAG_ROTATION_SHIFT)
enum _RGA_ROTATION {
	RK_ROTATE_0		= 0 << FLAG_ROTATION_SHIFT,	//0	  degree
	RK_ROTATE_90	= 1 << FLAG_ROTATION_SHIFT,	//90  degree
	RK_ROTATE_180	= 2 << FLAG_ROTATION_SHIFT,	//180 degree
	RK_ROTATE_270	= 3 << FLAG_ROTATION_SHIFT,	//270 degree
};

#define FLAG_MMU_SHIFT	16
#define FLAG_MMU_MASK	(3 << FLAG_MMU_SHIFT)
enum _RGA_MMU {
	RK_MMU_DISABLE	= 0 << FLAG_MMU_SHIFT,
	RK_MMU_SRC_ENABLE	= 1 << FLAG_MMU_SHIFT,
	RK_MMU_DST_ENABLE = 2 << FLAG_MMU_SHIFT
};

#define FLAG_SYNC_SHIFT	18
#define FLAG_SYNC_MASK	(1 << FLAG_SYNC_SHIFT)
enum _RGA_SYNC {
	RK_SYNC_MODE = 0,
	RK_ASYNC_MODE = 1 << FLAG_SYNC_SHIFT
};

typedef struct _rga_img_info_t
{
    unsigned int yrgb_addr;      /* yrgb    mem addr         */
    unsigned int uv_addr;        /* cb/cr   mem addr         */
    unsigned int v_addr;         /* cr      mem addr         */
    unsigned int format;         //definition by RK_FORMAT
    
    unsigned short act_w;
    unsigned short act_h;
    unsigned short x_offset;
    unsigned short y_offset;
    
    unsigned short vir_w;
    unsigned short vir_h;
    
    unsigned short endian_mode; //for BPP
    unsigned short alpha_swap;
} rga_img_info_t;

class CopyBit {
public:
	CopyBit();
	~CopyBit();
	// flag:
	// bit[0-3]		SCALE	mode
	// bit[4-7]		YUV2RGB mode
	// bit[8-15]	Rotation degree
	// bit[16]		MMU mode
	// bit[17]		SYNC mode
	int draw(rga_img_info_t *src, rga_img_info_t *dst, unsigned int flag);		
																	
private:
	int fd;
};

#ifdef __cplusplus
}
#endif

#endif //__RGA_H__
