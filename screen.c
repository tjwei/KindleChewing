/*
 * (C) 2010 Andy
 *
 * fbscreen interface routines
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "pixop.h"
#include "screen.h"

typedef unsigned char u8 ;

#include "linux/einkfb.h"

fbscreen_t *fb_open(void)
{
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	static fbscreen_t fb ; // XXX non reentrant

	memset(&fb, 0, sizeof(fb)) ;

	/* Open the file for reading and writing */
	fb.fd = open("/dev/fb0", O_RDWR);
	if (fb.fd < 0) {
		printf("Error: cannot open framebuffer device.\n");
		return NULL;
	}
	/* Get fixed screen information */
	if (ioctl(fb.fd, FBIOGET_FSCREENINFO, &finfo)) /* non fatal */
		printf("Error reading fixed screen information.\n");

	/* Get variable screen information */
	if (ioctl(fb.fd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("Error reading variable screen information.\n");
		/* assume we are on K2, if so */
		vinfo.xres=824;
		vinfo.yres=1200;
		vinfo.bits_per_pixel=4 ;
	}
	/* Figure out the size of the screen in bytes */
	fb.screensize = (vinfo.xres * vinfo.yres * vinfo.bits_per_pixel) / 8;

	/* Map the device to memory.
	 * We need MAP_SHARED or updates will not be sent.
	 */
	fb.pixmap.surface = mmap(NULL, fb.screensize,
		    PROT_READ | PROT_WRITE, MAP_SHARED, fb.fd, 0);
	if (fb.pixmap.surface == ((void*) -1)) {
		close(fb.fd) ;
		printf("Error: failed to mmap framebuffer\n");
		return NULL;
	}
	fb.pixmap.width = vinfo.xres ;
	fb.pixmap.height = vinfo.yres ;
	fb.pixmap.bpp = vinfo.bits_per_pixel;
	return &fb;
}

void fb_close(fbscreen_t *fb)
{
	if (!fb)
		return;
	if ((fb->fd != -1) && (fb->pixmap.surface) && (fb->screensize != 0)) {
		munmap(fb->pixmap.surface, fb->screensize);
		close(fb->fd);
	}	
	fb->fd = -1 ;
	fb->screensize = 0 ;
	if (fb->pixmap.surface)
		memset(&fb->pixmap, 0, sizeof(pixmap_t)) ;
	fb->pixmap.surface = NULL;
}


void fb_update_area(fbscreen_t *fb,
	int mode, int x0, int y0, int w, int h, void *pbuf)
{
	update_area_t ua;
	int ret;

	c_truncate(&x0, &w, fb->pixmap.width);
	c_truncate(&y0, &h, fb->pixmap.height);
	ua.x1 = x0 ;
	ua.y1 = y0 ;
	ua.x2 = x0 + w ;
	ua.y2 = y0 + h ;
	ua.which_fx = mode ;
	ua.buffer = pbuf ;
	if (w == 0 || h == 0)
		return;

	ret = ioctl(fb->fd, FBIO_EINK_UPDATE_DISPLAY_AREA, &ua);
	if (ret) {
		fprintf(stderr, "%s @%d %d %d x %d error %d\n",
			__FUNCTION__, x0, y0, w, h, errno);
		perror("error: ");
	}
}
