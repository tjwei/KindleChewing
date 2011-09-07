#include "im.h"
//#include <iostream>
#include <ft2build.h> 
#include FT_FREETYPE_H
#include FT_CACHE_H
static FT_Library library; 
static FTC_Manager ftcManager;
static FTC_SBitCache sbitCache;
static FTC_CMapCache cmapCache;
static FTC_SBit sbit;
struct FTC_FaceIDRec_
{
	int face_index;
};
static FTC_FaceID faceID={0};
static FTC_ImageTypeRec ftcImageTypeRec={
	faceID, 24, 24, FT_LOAD_DEFAULT | FT_LOAD_RENDER}; 
int utf8_to_ucs2 (unsigned char * input, unsigned char ** end_ptr)
{
    *end_ptr = input;
    if (input[0] == 0)
        return -1;
    if (input[0] < 0x80) {
        * end_ptr = input + 1;
        return input[0];
    }
    if ((input[0] & 0xE0) == 0xE0) {
        if (input[1] == 0 || input[2] == 0)
            return -1;
        * end_ptr = input + 3;
        return
            (input[0] & 0x0F)<<12 |
            (input[1] & 0x3F)<<6  |
            (input[2] & 0x3F);
    }
    if ((input[0] & 0xC0) == 0xC0) {
        if (input[1] == 0)
            return -1;
        * end_ptr = input + 2;
        return
            (input[0] & 0x1F)<<6  |
            (input[1] & 0x3F);
    }
    return -1;
}

static int draw_text(pixmap_t *pix, int x, int basey, unsigned int uni){
	if(basey>=pix->height) return x;
	unsigned int idx=FTC_CMapCache_Lookup(cmapCache, faceID, 0, uni);
	if(idx==0) return x+12;
	FTC_SBitCache_Lookup(sbitCache, &ftcImageTypeRec, idx,&sbit,NULL); 
	int left=sbit->left, top=sbit->top;
        int dx=x+left, dy=basey-top;
	if(dx>=pix->width || dy<0) return x;
	unsigned char *pos=PIXMAP_POS(pix,dx,dy);
	unsigned char *spos=sbit->buffer;
	for(int py=0;py<sbit->height;py++, pos+=pix->width, spos+=sbit->pitch)
		memcpy(pos,spos,sbit->width);
	return x+sbit->xadvance;
//	return x+(left+sbit->width>14 ? 26 : 13);
}
int draw_str(pixmap_t *pix, int x,int basey,char *str, int len, char **nptr){
	unsigned char *ptr=(unsigned char *)str;
	int i;
	for(i=len; i!=0 && *ptr;i--){
		int uni=utf8_to_ucs2(ptr, &ptr);
		if(uni==-1) break;
		x=draw_text(pix, x,basey,uni);	
	}
	if(nptr!=NULL) *nptr=(char *)ptr;
	return x;
	
}
static FT_Error ftcFaceRequester(FTC_FaceID fid, FT_Library lib, FT_Pointer data,FT_Face *aface)
{
	return FT_New_Face(lib,
			 "/usr/java/lib/fonts/Mono_Regular.ttf",
			0, aface);
}
int init_ttf(){
	int	error;
	error = FT_Init_FreeType( &library ); 
	if ( error ) {printf("FT_Init error\n");return 0;}
	error=FTC_Manager_New(library, 1 , 1, 24*24*200, ftcFaceRequester, 
						 NULL ,&ftcManager  );
	if (error) {
		perror("unable to open the font cache manager");
		return 0;
	};
	error=FTC_SBitCache_New(ftcManager, &sbitCache);
	if (error) {
		perror("unable to create new sbitcache");
		return 0;
	}
	error=FTC_CMapCache_New(ftcManager, &cmapCache);
	if (error) {
		perror("unable to create new cmapcache");
		return 0;
	}
	return 1;
}
