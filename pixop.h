/*
 * (C) 2010 Andy
 *
 * routines to use the eink screen
 */

#ifndef _PIXOP_H_
#define _PIXOP_H_
#include <string.h>
typedef struct pixmap_str {
	int	width;
	int	height;
	int	bpp;	/* bits per pixel */
	unsigned char	*surface;	// 1 byte = 2 pixels
} pixmap_t;
static inline void c_truncate(int *ofs, int *len, int bound)
{
        if (*ofs < 0) { /* decrease size by offset */
                *len += *ofs;
                *ofs = 0;
        }
	if (*ofs > bound) {
		*len = 0;
		*ofs = bound;
	}
	if (*len <= 0)
		*len = 0;
        if (*ofs + *len > bound)
                *len = bound - *ofs;
}


static inline unsigned char *PIXMAP_POS(pixmap_t *pix,int x,int y) {
	if(pix->bpp==8)
		return pix->surface+y*pix->width+x;
	else if (pix->bpp==4)
		return pix->surface+((y*pix->width+x)>>1);
	else
		return pix->surface+((y*pix->width+x)*pix->bpp/8);
		
}

static inline pixmap_t * pix_alloc(int w, int h, int bpp)
{
	int size = ((bpp*w+7)/8*h) + sizeof(pixmap_t) ;
	pixmap_t * p = (pixmap_t *)calloc(1, size);
	if (p) {
		p->width = w ;
		p->height = h ;
		p->bpp=bpp;
		p->surface = (unsigned char *)(p + 1);
	}

	return p ;
}

static inline void pix_free(pixmap_t* p)
{
	free(p);	
}

#endif
