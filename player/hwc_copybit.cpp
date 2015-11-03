#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
//#include <cutils/log.h>
#include "hwc_copybit.h"
#include "rga_define.h"
#include "rga_angle.h"

#undef TARGET_RK32
#define TARGET_RK32

#define ALOGE(...) { fprintf(stdout, __VA_ARGS__);fprintf(stdout,"\n"); }
#define ALOGD(...) { fprintf(stdout, __VA_ARGS__);fprintf(stdout,"\n"); }

int CopyBit::draw(rga_img_info_t *src, rga_img_info_t *dst, unsigned int flag)
{
	struct rga_req  Rga_Request;
	unsigned int rotation = 0;
	int ret = 0;
	
	if(fd < 0) {
		ALOGE("%s: rga is not opened.\n", __FUNCTION__);
		return -1;
	}
	
	if(src == NULL || dst == NULL) {
		ALOGE("%s: parameter ALOGEor", __FUNCTION__);
		return -1;
	}
	
	memset(&Rga_Request, 0x0, sizeof(Rga_Request));
	Rga_Request.render_mode = MODE_BITBLIT;
	Rga_Request.src = *src;
	Rga_Request.dst = *dst;
#ifdef TARGET_RK32
	Rga_Request.src.uv_addr =  Rga_Request.src.yrgb_addr;
	Rga_Request.src.yrgb_addr = 0;
	Rga_Request.src.v_addr = 0;
	Rga_Request.dst.uv_addr = Rga_Request.dst.yrgb_addr;
	Rga_Request.dst.yrgb_addr = 0;
	Rga_Request.dst.v_addr = 0;
#endif
	Rga_Request.clip.xmin = 0;
    Rga_Request.clip.xmax = dst->vir_w - 1;
    Rga_Request.clip.ymin = 0;
    Rga_Request.clip.ymax = dst->vir_h - 1;
    
    if(flag & FLAG_MMU_MASK) {
    	Rga_Request.mmu_info.mmu_en    = 1;
    	Rga_Request.mmu_info.mmu_flag  = ((2 & 0x3) << 4) | 1;
#ifdef TARGET_RK32
		Rga_Request.mmu_info.mmu_flag |= (1 << 31);
		if(flag & RK_MMU_SRC_ENABLE){
			Rga_Request.mmu_info.mmu_flag  |=  (1<<8); /* [8] src [10] dst */
		}
		if(flag & RK_MMU_DST_ENABLE){
			Rga_Request.mmu_info.mmu_flag  |=  (1<<10);
		}
#endif
	}

    Rga_Request.scale_mode = (flag & FLAG_SCALE_MASK) >> FLAG_SCALE_SHIFT;
	
    if(flag & FLAG_YUV2RGB_MASK) {
    	Rga_Request.yuv2rgb_mode = (flag & FLAG_YUV2RGB_MASK) >> FLAG_YUV2RGB_SHIFT;
    }
	
	if(flag & FLAG_ROTATION_MASK) {
		rotation = (flag & FLAG_ROTATION_MASK) >> FLAG_ROTATION_SHIFT;
		switch(rotation) {
			case RK_ROTATE_90:
				rotation = 90;
				Rga_Request.dst.x_offset += dst->act_h - 1;
				break;
			case RK_ROTATE_180:
				rotation = 180;
				Rga_Request.dst.x_offset += dst->act_h - 1;
				Rga_Request.dst.y_offset += dst->act_w - 1;
				break;
			case RK_ROTATE_270:
				rotation = 270;
				Rga_Request.dst.y_offset += dst->act_w - 1;
				break;
			default:
				rotation = 0;
				break;
		}
	}
	if(rotation)
		Rga_Request.rotate_mode = ROTATE_ENABLE;
	else
		Rga_Request.rotate_mode = 0;
	Rga_Request.cosa = cosa_table[rotation];
	Rga_Request.sina = sina_table[rotation];
	
    //ALOGE("scale_mode %d yuv2rgb_mode %d rotate_mode %d\n", Rga_Request.scale_mode, Rga_Request.yuv2rgb_mode, Rga_Request.rotate_mode);
    if(flag & FLAG_SYNC_MASK)
    	ret = ioctl(fd, RGA_BLIT_ASYNC, &Rga_Request);
    else
    	ret = ioctl(fd, RGA_BLIT_SYNC, &Rga_Request);
    if(ret != 0) {
		ALOGE("%s:  rga operation error\n", __FUNCTION__);
	 	ALOGE("src info: yrgb_addr=%x, uv_addr=%x,v_addr=%x,"
	         "vir_w=%d,vir_h=%d,format=%d,"
	         "act_x_y_w_h [%d,%d,%d,%d] ",
				Rga_Request.src.yrgb_addr, Rga_Request.src.uv_addr ,Rga_Request.src.v_addr,
				Rga_Request.src.vir_w ,Rga_Request.src.vir_h ,Rga_Request.src.format ,
				Rga_Request.src.x_offset ,
				Rga_Request.src.y_offset,
				Rga_Request.src.act_w ,
				Rga_Request.src.act_h

	        );

	 	ALOGE("dst info: yrgb_addr=%x, uv_addr=%x,v_addr=%x,"
	         "vir_w=%d,vir_h=%d,format=%d,"
	         "clip[%d,%d,%d,%d], "
	         "act_x_y_w_h [%d,%d,%d,%d] ",

				Rga_Request.dst.yrgb_addr, Rga_Request.dst.uv_addr ,Rga_Request.dst.v_addr,
				Rga_Request.dst.vir_w ,Rga_Request.dst.vir_h ,Rga_Request.dst.format,
				Rga_Request.clip.xmin,
				Rga_Request.clip.xmax,
				Rga_Request.clip.ymin,
				Rga_Request.clip.ymax,
				Rga_Request.dst.x_offset ,
				Rga_Request.dst.y_offset,
				Rga_Request.dst.act_w ,
				Rga_Request.dst.act_h

	        );
		return -1;
	}
	return 0;
}

CopyBit::CopyBit(void)
{
	if(!access("/dev/rga", R_OK | W_OK)) {
		fd = open("/dev/rga", O_RDWR, 0);
		if(fd < 0)
                {
			ALOGE("open rga device error");
                }
		else
                {
			ALOGD("open rga device");
                }
	}
	else
		fd = -1;
}



CopyBit::~CopyBit(void)
{
	if(fd >= 0)
        {
		close(fd);
		ALOGD("close rga device");
        }
}
