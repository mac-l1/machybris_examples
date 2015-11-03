#ifndef _RGA_DEFINE_H_
#define _RGA_DEFINE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define RGA_BLIT_SYNC	0x5017
#define RGA_BLIT_ASYNC  0x5018
#define RGA_FLUSH       0x5019
#define RGA_GET_RESULT  0x501a
#define RGA_GET_VERSION 0x501b

/* RGA process mode enum */
enum _RGA_PROCESS_MODE {
    MODE_BITBLIT              = 0x0,
    MODE_COLOR_PALETTE        = 0x1,
    MODE_COLOR_FILL           = 0x2,
    MODE_LINE_POINT_DRAW	  = 0x3,
    MODE_BLUR_SHARP_FILTER    = 0x4,
    MODE_PRESCALE             = 0x5,
    MODE_UPDATE_PALETTE_TABLE = 0x6,
    MODE_UPDATE_PATTEN_BUFF	  = 0x7,
};

/* RGA rotate mode */
enum 
{
    ROTATE_DISABLE           = 0x0,     /* no rotate */
    ROTATE_ENABLE            = 0x1,     /* rotate    */
    ROTATE_MIRROR_X          = 0x2,     /* x_mirror  */
    ROTATE_MIRROR_Y          = 0x3,     /* y_mirror  */
};

typedef struct RANGE
{
    unsigned short min;
    unsigned short max;
}
RANGE;

typedef struct POINT
{
    unsigned short x;
    unsigned short y;
}
POINT;

typedef struct RECT
{
    unsigned short xmin;
    unsigned short xmax; // width - 1
    unsigned short ymin; 
    unsigned short ymax; // height - 1 
} RECT;

typedef struct RGB
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char res;
}RGB;

typedef struct MMU
{
    unsigned char mmu_en;
    unsigned int base_addr;
    unsigned int mmu_flag;     /* [0] mmu enable [1] src_flush [2] dst_flush [3] CMD_flush [4~5] page size*/
} MMU;

typedef struct COLOR_FILL
{
    short gr_x_a;
    short gr_y_a;
    short gr_x_b;
    short gr_y_b;
    short gr_x_g;
    short gr_y_g;
    short gr_x_r;
    short gr_y_r;

    //u8  cp_gr_saturation;
} COLOR_FILL;

typedef struct FADING
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char res;
} FADING;

typedef struct line_draw_t
{
    POINT start_point;              /* LineDraw_start_point                */
    POINT end_point;                /* LineDraw_end_point                  */
    unsigned int   color;               /* LineDraw_color                      */
    unsigned int   flag;                /* (enum) LineDrawing mode sel         */
    unsigned int   line_width;          /* range 1~16 */
} line_draw_t;

struct rga_req { 
    unsigned char render_mode;				/* (enum) process mode sel */
    
    rga_img_info_t src;                   	/* src image info */
    rga_img_info_t dst;                   	/* dst image info */
    rga_img_info_t pat;             		/* patten image info */

    unsigned int rop_mask_addr;         	/* rop4 mask addr */
    unsigned int LUT_addr;              	/* LUT addr */
    
    RECT clip;                      		/* dst clip window default value is dst_vir */
                                    		/* value from [0, w-1] / [0, h-1]*/
        
    int sina;                   			/* dst angle  default value 0  16.16 scan from table */
    int cosa;                   			/* dst angle  default value 0  16.16 scan from table */        

    unsigned short alpha_rop_flag;			/* alpha rop process flag           */
											/* ([0] = 1 alpha_rop_enable)       */
											/* ([1] = 1 rop enable)             */                                                                                                                
											/* ([2] = 1 fading_enable)          */
											/* ([3] = 1 PD_enable)              */
											/* ([4] = 1 alpha cal_mode_sel)     */
											/* ([5] = 1 dither_enable)          */
											/* ([6] = 1 gradient fill mode sel) */
											/* ([7] = 1 AA_enable)              */

    unsigned char  scale_mode;				/* 0 nearst / 1 bilnear / 2 bicubic */                             
                            
    unsigned int color_key_max;				/* color key max */
    unsigned int color_key_min;				/* color key min */     

    unsigned int fg_color;					/* foreground color */
    unsigned int bg_color;					/* background color */

    COLOR_FILL gr_color;					/* color fill use gradient */
    
    line_draw_t line_draw_info;
    
    FADING fading;
                              
    unsigned char PD_mode;                /* porter duff alpha mode sel */
    
    unsigned char alpha_global_value;     /* global alpha value */
     
    unsigned short rop_code;              /* rop2/3/4 code  scan from rop code table*/
    
    unsigned char bsfilter_flag;          /* [2] 0 blur 1 sharp / [1:0] filter_type*/
    
    unsigned char palette_mode;           /* (enum) color palatte  0/1bpp, 1/2bpp 2/4bpp 3/8bpp*/

    unsigned char yuv2rgb_mode;           /* (enum) BT.601 MPEG / BT.601 JPEG / BT.709  */ 
    
    unsigned char endian_mode;            /* 0/big endian 1/little endian*/

    unsigned char rotate_mode;            /* (enum) rotate mode  */
										/* 0x0,     no rotate  */
										/* 0x1,     rotate     */
										/* 0x2,     x_mirror   */
										/* 0x3,     y_mirror   */

    unsigned char color_fill_mode;		/* 0 solid color / 1 patten color */
                                    
    MMU mmu_info;						/* mmu information */

    unsigned char  alpha_rop_mode;		/* ([0~1] alpha mode)       */
										/* ([2~3] rop   mode)       */
										/* ([4]   zero  mode en)    */
										/* ([5]   dst   alpha mode) */

    unsigned char  src_trans_mode;

    unsigned char CMD_fin_int_enable;                        

    /* completion is reported through a callback */
	void (*complete)(int retval);
};

#ifdef __cplusplus
}
#endif

#endif 
