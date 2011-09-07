#ifndef __IM_H__
#define __IM_H__
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
extern "C" {
	#include "screen.h"
	int utf8_to_ucs2 (unsigned char * input, unsigned char ** end_ptr);
	void sendutf8(char *utf8str); //via dbus
	void sendkey(int pressed, int keycode, int keychar, int modifier); //via dbus
	int init_ttf();
}
#define IMBAR_HEIGHT 36
int draw_str(pixmap_t *,int,int,char *, int len=-1, char **ptr=NULL);
inline int draw_str(pixmap_t *pix,int x,int y,const char *s, int len=-1, char **ptr=NULL){
	return draw_str(pix,x,y,(char *)s,len, ptr);
}
inline int draw_str(pixmap_t *pix,int x,int y, std::string &s, int len=-1, char **ptr=NULL){
	return draw_str(pix,x,y,s.c_str(),len, ptr);
}
inline int draw_str(pixmap_t *pix,int x,int y, std::stringstream &s, int len=-1, char **ptr=NULL){
	return draw_str(pix,x,y,s.str().c_str(),len, ptr);
}
#define RECT_OP_VARS(TYPE) pixmap_t *pix, int x, int y,int w,int h, TYPE v
#define LOOP_RECT_DO(op) \
	unsigned char *pos0=PIXMAP_POS(pix, x, y);\
	for(int py=0;py<h;py++){ \
	   unsigned char *pos=pos0; \
	   for(int px=0;px<w;px++) (*pos++) op v;\
	   pos0+=pix->width;}
#define uchar unsigned char
inline void rect_xor (RECT_OP_VARS(uchar)) {LOOP_RECT_DO(^=)}
inline void rect_or (RECT_OP_VARS(uchar)) {LOOP_RECT_DO(|=)}
inline void rect_and (RECT_OP_VARS(uchar)) {LOOP_RECT_DO(&=)}
inline void rect_add (RECT_OP_VARS(uchar)) {LOOP_RECT_DO(+=)}
inline void rect_fill (RECT_OP_VARS(uchar)) {LOOP_RECT_DO(=)}
inline void rect_lshift (RECT_OP_VARS(int)) {LOOP_RECT_DO(>>=)}
inline void rect_rshift (RECT_OP_VARS(int)) {LOOP_RECT_DO(<<=)}
inline void rect_mul (RECT_OP_VARS(float)) {LOOP_RECT_DO(*=)}
#define RECT_OP_VARS_FULLWIDTH(TYPE) pixmap_t *pix,  int y, int h, TYPE v
inline void rect_fill(RECT_OP_VARS_FULLWIDTH(uchar)) {memset(PIXMAP_POS(pix,0,y),v,pix->width*h);}
#endif
