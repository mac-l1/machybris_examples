/*
 * some generic framebuffer device stuff
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/fb.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <poll.h>
#include <stdint.h>


#include "fbtools.h"

/* framebuffer */
char                       *fbdev = NULL;
char                       *fbmode  = NULL;
int                        fd, switch_last, debug;

unsigned short red[256],  green[256],  blue[256];
struct fb_cmap cmap  = { 0, 256, red,  green,  blue };

static float fbgamma = 1;


/* -------------------------------------------------------------------- */
/* exported stuff                                                       */

struct fb_fix_screeninfo   fb_fix;
struct fb_var_screeninfo   fb_var;
unsigned char             *fb_mem;
int			   fb_mem_offset = 0;
int                        fb_switch_state = FB_ACTIVE;

/* -------------------------------------------------------------------- */
/* internal variables                                                   */
    char thread_name[64] = "VSYNC-thread";
    const char* vsync_timestamp_fb0 = "/sys/class/graphics/fb0/vsync";
    uint64_t cur_timestamp=0, last_timestamp = 0;
    ssize_t len = -1;
    int fd_timestamp = -1;
    struct pollfd fds[1];

    #define MAX_DATA 64
    static char vdata[MAX_DATA];


static int                       fb;
#if 0
static int                       bpp,black,white;
#endif

static int                       orig_vt_no = 0;
static struct vt_mode            vt_mode;

static int                       kd_mode;
static struct vt_mode            vt_omode;
static struct termios            term;
static struct fb_var_screeninfo  fb_ovar;
static unsigned short            ored[256], ogreen[256], oblue[256];
static struct fb_cmap            ocmap = { 0, 256, ored, ogreen, oblue };

/* -------------------------------------------------------------------- */
/* devices                                                              */

struct DEVS {
    char *fb0;
    char *fbnr;
};

struct DEVS devs_default = {
    fb0:   "/dev/fb0",
    fbnr:  "/dev/fb%d",
};
struct DEVS devs_devfs = {
    fb0:   "/dev/fb/0",
    fbnr:  "/dev/fb/%d",
};
struct DEVS *devices;

static void dev_init(void)
{
    struct stat dummy;

    if (NULL != devices)
	return;
    if (0 == stat("/dev/.devfsd",&dummy))
	devices = &devs_devfs;
    else
	devices = &devs_default;
}

extern int debug;

/* -------------------------------------------------------------------- */
/* initialisation & cleanup                                             */

void
fb_memset (void *addr, int c, size_t len)
{
    memset(addr, c, len);
}

static int
fb_setmode(char *name)
{
    FILE *fp;
    char line[80],label[32],value[16];
    int  geometry=0, timings=0;
    
    /* load current values */
    if (-1 == ioctl(fb,FBIOGET_VSCREENINFO,&fb_var)) {
	perror("ioctl FBIOGET_VSCREENINFO");
	exit(1);
    }
    
    if (NULL == name)
	return -1;
    if (NULL == (fp = fopen("/etc/fb.modes","r")))
	return -1;
    while (NULL != fgets(line,79,fp)) {
	if (1 == sscanf(line, "mode \"%31[^\"]\"",label) &&
	    0 == strcmp(label,name)) {
	    /* fill in new values */
	    fb_var.sync  = 0;
	    fb_var.vmode = 0;
	    while (NULL != fgets(line,79,fp) &&
		   NULL == strstr(line,"endmode")) {
		if (5 == sscanf(line," geometry %d %d %d %d %d",
				&fb_var.xres,&fb_var.yres,
				&fb_var.xres_virtual,&fb_var.yres_virtual,
				&fb_var.bits_per_pixel))
		    geometry = 1;
		if (7 == sscanf(line," timings %d %d %d %d %d %d %d",
				&fb_var.pixclock,
				&fb_var.left_margin,  &fb_var.right_margin,
				&fb_var.upper_margin, &fb_var.lower_margin,
				&fb_var.hsync_len,    &fb_var.vsync_len))
		    timings = 1;
		if (1 == sscanf(line, " hsync %15s",value) &&
		    0 == strcasecmp(value,"high"))
		    fb_var.sync |= FB_SYNC_HOR_HIGH_ACT;
		if (1 == sscanf(line, " vsync %15s",value) &&
		    0 == strcasecmp(value,"high"))
		    fb_var.sync |= FB_SYNC_VERT_HIGH_ACT;
		if (1 == sscanf(line, " csync %15s",value) &&
		    0 == strcasecmp(value,"high"))
		    fb_var.sync |= FB_SYNC_COMP_HIGH_ACT;
		if (1 == sscanf(line, " extsync %15s",value) &&
		    0 == strcasecmp(value,"true"))
		    fb_var.sync |= FB_SYNC_EXT;
		if (1 == sscanf(line, " laced %15s",value) &&
		    0 == strcasecmp(value,"true"))
		    fb_var.vmode |= FB_VMODE_INTERLACED;
		if (1 == sscanf(line, " double %15s",value) &&
		    0 == strcasecmp(value,"true"))
		    fb_var.vmode |= FB_VMODE_DOUBLE;
	    }
	    /* ok ? */
	    if (!geometry || !timings)
		return -1;
	    /* set */
	    fb_var.xoffset = 0;
	    fb_var.yoffset = 0;
	    if (-1 == ioctl(fb,FBIOPUT_VSCREENINFO,&fb_var))
		perror("ioctl FBIOPUT_VSCREENINFO");
	    /* look what we have now ... */
	    if (-1 == ioctl(fb,FBIOGET_VSCREENINFO,&fb_var)) {
		perror("ioctl FBIOGET_VSCREENINFO");
		exit(1);
	    }
	    return 0;
	}
    }
    return -1;
}

int
fb_init(char *device, char *mode)
{
    unsigned long page_mask;
    int ret = 0;
    int fb_blank = 0 ? FB_BLANK_POWERDOWN : FB_BLANK_UNBLANK;
    int vsync_enable = !0; // 0 = disabled,

    dev_init();

    /* get current settings (which we have to restore) */
    if (-1 == (fb = open(device,O_RDWR /* O_WRONLY */))) {
	fprintf(stderr,"open %s: %s\n",device,strerror(errno));
	exit(1);
    }
    if (-1 == ioctl(fb,FBIOGET_VSCREENINFO,&fb_ovar)) {
	perror("ioctl FBIOGET_VSCREENINFO");
	exit(1);
    }
    if (-1 == ioctl(fb,FBIOGET_FSCREENINFO,&fb_fix)) {
	perror("ioctl FBIOGET_FSCREENINFO");
	exit(1);
    }
    if (fb_ovar.bits_per_pixel == 8 ||
	fb_fix.visual == FB_VISUAL_DIRECTCOLOR) {
	if (-1 == ioctl(fb,FBIOGETCMAP,&ocmap)) {
	    perror("ioctl FBIOGETCMAP");
	    exit(1);
	}
    }

    /* switch mode */
    fb_setmode(mode);
    
    /* checks & initialisation */
    if (-1 == ioctl(fb,FBIOGET_FSCREENINFO,&fb_fix)) {
	perror("ioctl FBIOGET_FSCREENINFO");
	exit(1);
    }
    if (fb_fix.type != FB_TYPE_PACKED_PIXELS) {
	fprintf(stderr,"can handle only packed pixel frame buffers\n");
	goto err;
    }
#if 0
    switch (fb_var.bits_per_pixel) {
    case 8:
	white = 255; black = 0; bpp = 1;
	break;
    case 15:
    case 16:
	if (fb_var.green.length == 6)
	    white = 0xffff;
	else
	    white = 0x7fff;
	black = 0; bpp = 2;
	break;
    case 24:
	white = 0xffffff; black = 0; bpp = fb_var.bits_per_pixel/8;
	break;
    case 32:
	white = 0xffffff; black = 0; bpp = fb_var.bits_per_pixel/8;
	fb_setpixels = fb_setpixels4;
	break;
    default:
	fprintf(stderr, "Oops: %i bit/pixel ???\n",
		fb_var.bits_per_pixel);
	goto err;
    }
#endif
    page_mask = getpagesize()-1;
    fb_mem_offset = (unsigned long)(fb_fix.smem_start) & page_mask;
    fb_mem = (unsigned char*)mmap(NULL,fb_fix.smem_len+fb_mem_offset,
		  PROT_READ|PROT_WRITE,MAP_SHARED,fb,0);
    if (-1L == (long)fb_mem) {
	perror("mmap");
	goto err;
    }
    /* move viewport to upper left corner */
    if (fb_var.xoffset != 0 || fb_var.yoffset != 0) {
	fb_var.xoffset = 0;
	fb_var.yoffset = 0;
	if (-1 == ioctl(fb,FBIOPAN_DISPLAY,&fb_var)) {
	    perror("ioctl FBIOPAN_DISPLAY");
	    goto err;
	}
    }

    ret = ioctl(fb, FBIOBLANK, fb_blank);
    if( ret < 0 )
	perror("ioctl FBIOBLANK");
    //printf("ioctl FBIOBLANK = %d, enable = %d\n", ret, fb_blank );

    #define RK_FBIOSET_VSYNC_ENABLE     0x4629
    ret = ioctl(fb, RK_FBIOSET_VSYNC_ENABLE, &vsync_enable);
    if (ret < 0 ) 
	perror("ioctl RK_FBIOSET_VSYNC_ENABLE");
    //printf("ioctl RK_FBIOSET_VSYNC_ENABLE = %d, enable = %d\n", ret, vsync_enable );

    #define HAL_PRIORITY_URGENT_DISPLAY (-8)
    prctl(PR_SET_NAME, (unsigned long) &thread_name, 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, HAL_PRIORITY_URGENT_DISPLAY);

    fd_timestamp = open(vsync_timestamp_fb0, O_RDONLY);
    if (fd_timestamp < 0) 
	perror("open(vsync_timestamp_fb0, O_RDONLY)");

    fds[0].fd = fd_timestamp;
    fds[0].events = POLLPRI;

    /* cls */
    //fb_memset(fb_mem+fb_mem_offset, 0, fb_fix.line_length * fb_var.yres);
    return fb;

 err:
    fb_cleanup();
    exit(1);
}

void
fb_cleanup(void)
{
    /* restore console */
    if (-1 == ioctl(fb,FBIOPUT_VSCREENINFO,&fb_ovar))
	perror("ioctl FBIOPUT_VSCREENINFO");
    if (-1 == ioctl(fb,FBIOGET_FSCREENINFO,&fb_fix))
	perror("ioctl FBIOGET_FSCREENINFO");
    if (fb_ovar.bits_per_pixel == 8 ||
	fb_fix.visual == FB_VISUAL_DIRECTCOLOR) {
	if (-1 == ioctl(fb,FBIOPUTCMAP,&ocmap))
	    perror("ioctl FBIOPUTCMAP");
    }
    close(fb);
}

/* -------------------------------------------------------------------- */
/* handle fatal errors                                                  */

static jmp_buf fb_fatal_cleanup;

static void
fb_catch_exit_signal(int signal)
{
    siglongjmp(fb_fatal_cleanup,signal);
}

void
fb_catch_exit_signals(void)
{
    struct sigaction act,old;
    int termsig;

    memset(&act,0,sizeof(act));
    act.sa_handler = fb_catch_exit_signal;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act,&old);
    sigaction(SIGQUIT,&act,&old);
    sigaction(SIGTERM,&act,&old);

    sigaction(SIGABRT,&act,&old);
    sigaction(SIGTSTP,&act,&old);

    sigaction(SIGBUS, &act,&old);
    sigaction(SIGILL, &act,&old);
    sigaction(SIGSEGV,&act,&old);

    if (0 == (termsig = sigsetjmp(fb_fatal_cleanup,0)))
	return;

    /* cleanup */
    fb_cleanup();
    fprintf(stderr,"Oops: %s\n",sys_siglist[termsig]);
    exit(42);
}

void fb_clear_mem(void)
{
    fb_memset(fb_mem,0,fb_fix.smem_len);
}

void fb_clear_screen(void)
{
    fb_memset(fb_mem,0,fb_fix.line_length * fb_var.yres);
}

void fb_waitforsync(void)
{
    //int zero = 0;
    //ioctl(fb, FBIO_WAITFORVSYNC, &zero);
    int vsync = 0;
    do {
        int err = poll(fds, 1, -1);
        if (err > 0) {
            if (fds[0].revents & POLLPRI) {
                len = pread(fd_timestamp, vdata, MAX_DATA, 0);
                if (len < 0) {
                    if (errno != EAGAIN && errno != EINTR  &&
                        errno != EBUSY) {
	                perror(" pread(fd_timestamp, vdata, MAX_DATA, 0)");
                    }
                    continue;
                }
                const char *str = vdata;
                cur_timestamp = strtoull(str, NULL, 0);
                vsync = 1;
            }
        } else {
	    perror("ioctl FBIOPUTCMAP");
            break;
        }
    } while(!vsync); 
}

#ifdef TEST_MAIN
int main()
{
    fd = fb_init("/dev/fb0", NULL);
    //fb_catch_exit_signals();
    //fb_switch_init();
    //shadow_init();
    //shadow_set_palette(fd);
    //signal(SIGTSTP,SIG_IGN);
   
    for(;;) {
        fb_waitforsync();
        fb_memset(fb_mem,0,fb_fix.line_length * fb_var.yres);
        sleep( 1 );
        fb_waitforsync();
        fb_memset(fb_mem,128,fb_fix.line_length * fb_var.yres);
        sleep( 1 );
    } 
    return 0;
}
#endif
