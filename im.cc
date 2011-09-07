#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>

#include <linux/input.h>
#include <libgen.h>
#include <algorithm>
#include <sys/file.h>
#include <poll.h>
#include <iostream>
#include <sstream>
#include <string>
#include "im.h"
extern "C" {
#include "include/chewingio.h"
#include "include/chewing.h"
}
static fbscreen_t *fb=NULL;
static pixmap_t *saved_pixmap=NULL;
static pixmap_t *im_pix=NULL;
using namespace std;
/*
 * keyboard and fiveway input event file name pointers, if given in a command line
 */
static int fd_kbd = -1 ;
static int fd_fw = -1 ;

static int do_daemonize = 0 ;
static int running_as_daemon = 0 ;
static int keepgoing=1;
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define LOCKFILE_NAME  "/var/run/im.pid"
#define DIAG_PRINT printf

static void daemonize( const char *lockfile ) ;
static int input_captured = 0 ;

static bool open_event_files(void)
{
	fd_kbd = open("/dev/input/event0", O_RDWR | O_NONBLOCK , 0);
	fd_fw = open("/dev/input/event1", O_RDWR | O_NONBLOCK, 0);
	return fd_kbd!=-1 && fd_fw!=-1;
}

static void close_event_files()
{
	if (fd_kbd != -1) close(fd_kbd) ;
	if (fd_fw != -1) close(fd_fw);
	fd_fw=fd_kbd = -1;
}

static void capture_input(void)
{
	int on = 1 ;
	if (fd_kbd != -1 && ioctl(fd_kbd, EVIOCGRAB, on)) perror("Capture kbd input:");
	if (fd_fw != -1 && ioctl(fd_fw, EVIOCGRAB, on)) perror("Capture fw input:");
	input_captured = 1 ;
}

static void release_input(void)
{
	int off = 0 ;
	if (fd_kbd != -1 && ioctl(fd_kbd, EVIOCGRAB, off)) perror("Release kbd input:");
	if (fd_fw != -1 && ioctl(fd_fw, EVIOCGRAB, off)) perror("Release fw input:");
	input_captured = 0 ;
}

static void sig_handler(int x)
{
	keepgoing = 0 ;
	DIAG_PRINT("\n -- Exiting...\n") ;
}
static ChewingContext *ctx;
#define SELKEYS 10
static int selKey_define[ SELKEYS+1 ] = {'a','s','d','f','g','h','j','k','l',';',0};
inline int imbar_draw_selection(int x){
	char key[2]=" ";
	stringstream s;
	s << "選擇("<< (chewing_cand_CurrentPage(ctx)+1)<<"/" << chewing_cand_TotalPage(ctx)  <<"):";
	x=draw_str(im_pix,x,28, s);
	chewing_cand_Enumerate(ctx);
	int choices_per_page=chewing_cand_ChoicePerPage(ctx); 
	for(int i=0;chewing_cand_hasNext(ctx) && (i< choices_per_page);i++, x+=12){
		s.str("");
		s << (char) selKey_define[i] << chewing_cand_String(ctx);
		x=draw_str(im_pix,x,28, s);
	}
	return x;
}
inline int imbar_draw_buffer(int x){
	int cur=chewing_cursor_Current(ctx);
	if(cur<0){
		perror("cur error");
		return x;
	}
	char *buffer=chewing_buffer_String(ctx);
	char *bufferleft=buffer;
	int nZuin;
	char *zuin=chewing_zuin_String(ctx, &nZuin);
	x=draw_str(im_pix,x,28,buffer, cur, &bufferleft);
	int ox=x;
	if(nZuin>0)
		x=draw_str(im_pix,x,28,zuin);
	else if (*bufferleft)		
		x=draw_str(im_pix,x,28,bufferleft,1,&bufferleft);
	else
		x+=10;
	rect_xor(im_pix, ox,1,x-ox,27,0xff);
	if(*bufferleft)
		draw_str(im_pix,x,28,bufferleft);
	free(buffer);
	free(zuin);
	return x;
}
static void imbar_draw(){
	pixmap_t *pix=&fb->pixmap;
	rect_fill(im_pix, 1, im_pix->height-1, 0);
	draw_str(im_pix,0,28,  chewing_get_ChiEngMode(ctx) ? (char *) "中" : (char *) "英");
	draw_str(im_pix,24,28,  chewing_get_ShapeMode(ctx) ? (char *) "全" : (char *) "半");
	char kbs[6]="xxxxx";
	strncpy(kbs,&chewing_get_KBString(ctx )[3],5);	
	draw_str(im_pix,48,28,  kbs);
	rect_or(im_pix,0,0,116,36, 0x7f);
	if(chewing_cand_TotalChoice(ctx)>0)
		imbar_draw_selection(120);
	else
		imbar_draw_buffer(120);
}
static void imbar_updatefb(){
	pixmap_t *pix=&fb->pixmap;
	int itop=pix->height - IMBAR_HEIGHT;
	unsigned char *dpos=pix->surface+(pix->width/2)*itop;
	unsigned char *spos=im_pix->surface;
	for(int n=im_pix->width*im_pix->height/2;n>0;n--)
		*(dpos++)=(*(spos++)&0xf0)|(*(spos++)>>4);	   				
	fb_update_area(fb, UMODE_PARTIAL, 0, itop, pix->width, IMBAR_HEIGHT,NULL);
}
void handle_normal(bool alt, bool shift, int keycode){
	if(alt && shift && keycode==KEY_SPACE){
		fb=fb_open();
		if(!fb) {
			perror("unable to open framebuffer\n");
			return;
		}
		pixmap_t *pix=&fb->pixmap;
		saved_pixmap=pix_alloc(pix->width, 36, 4);
		im_pix=pix_alloc(pix->width, 36, 8);
                memcpy(saved_pixmap->surface, PIXMAP_POS(pix,0,pix->height-36), 
					     18*saved_pixmap->width);
		rect_fill(im_pix,0,1,0xff);
		imbar_draw();
		imbar_updatefb();
		capture_input();
		// simulate release alt-shift-space
		sendkey(0,32,32,9);
		sendkey(0,18,65535,1);
		sendkey(0,16,65535,0);
	}
}
void handle_im(bool alt, bool shift, int keycode){
	static char keymap[55]="\0\0""1234567890-=\0"
                               "\tqwertyuiop[]\n"
                               "\0""asdfghjkl;'`\0\\"
                               "zxcvbnm,./";
	char keychar='\x0';
	if(keycode<54){
		keychar=keymap[keycode];
		if(shift && isalpha(keychar))
			keychar=toupper(keychar);
	}
	switch(keycode){
		case KEY_ENTER: chewing_handle_Enter(ctx);break;
		case KEY_SPACE:
			if(alt && shift && keycode==KEY_SPACE){
				pixmap_t *pix=&fb->pixmap;
                		memcpy(PIXMAP_POS(pix,0,pix->height-36), saved_pixmap->surface, 
					     18*saved_pixmap->width);
				fb_update_area(fb, UMODE_PARTIAL, 0, pix->height-36, pix->width, 36, NULL);
				pix_free(saved_pixmap);
				pix_free(im_pix);
				fb_close(fb);
				fb=NULL;
				release_input();
				return;
			}			
			else if(shift){
				//chewing_handle_ShiftSpace(ctx);
				 chewing_set_ShapeMode(ctx,!chewing_get_ShapeMode(ctx));
			}
			else if(alt){
				//chewing_handle_ShiftSpace(ctx);
				 chewing_set_ShapeMode(ctx,!chewing_get_ShapeMode(ctx));
			}
			else
				chewing_handle_Space(ctx);
			break;
		case KEY_DELETE: 
			chewing_handle_Del(ctx);break;
		case KEY_BACKSPACE: 
			if(chewing_buffer_Len(ctx)==0 && chewing_get_phoneSeqLen(ctx)==0){
				sendkey(1, 8, 8, 0);
				sendutf8("\x08");
				sendkey(0, 8, 8, 0);
			}
			chewing_handle_Backspace(ctx);break;
		case KEY_LEFT: chewing_handle_Left(ctx);break;
		case KEY_RIGHT: chewing_handle_Right(ctx);break;
		case 122/*Up*/: chewing_handle_Up(ctx);break;
		case 123/*Down*/: chewing_handle_Down(ctx);break;
		case 90/*Aa*/: chewing_handle_Capslock(ctx);break;
		case 94/*Sym*/: 			
			//switch between KB_HSU and KB_HANYU_PINGYIN
			chewing_set_KBType(ctx, chewing_get_KBType(ctx)^8);
			break;
		default:
		if(keychar)
			chewing_handle_Default(ctx, keychar);
		else return;
	}
	char *s;
	pixmap_t *pix=&fb->pixmap;
	if(chewing_commit_Check(ctx)){
			s=chewing_commit_String(ctx);
			sendutf8(s);			
			free(s);			
	}
	imbar_draw();
	imbar_updatefb();
}

int main(int argc, char **av)
{
	int rc ;
	if (do_daemonize)
	{
		daemonize(LOCKFILE_NAME) ;
		running_as_daemon = 1 ;
	}
	init_ttf();
	chewing_Init("/mnt/us/chewing/data/", "/mnt/us/chewing/user/");
	ctx=chewing_new();
	chewing_set_KBType(ctx, chewing_KBStr2Num((char *)"KB_HSU"));
	chewing_set_candPerPage(ctx,9);
	chewing_set_maxChiSymbolLen(ctx, 16);
	chewing_set_addPhraseDirection(ctx,1);
	chewing_set_selKey(ctx, selKey_define, 10);
	chewing_set_spaceAsSelection(ctx, 1);
	
again:
	signal(SIGINT, sig_handler) ;
	signal(SIGTERM, sig_handler) ;
	signal(SIGHUP, sig_handler) ;
	keepgoing = 1 ;
	if(!open_event_files()) {
			perror("unable to open input events\n");
			return 1;
	}

	#define MAX_KEYS ((size_t) 2)
	struct input_event kbbuf[MAX_KEYS];
	struct input_event fwbuf[MAX_KEYS];
	fd_set rxfds ;
	struct timeval tv ;
	int maxfd ;
	bool alt=false;
	bool shift=false;
	keepgoing = 1 ;
	
	while (keepgoing) {
		maxfd = 0 ;
		FD_ZERO(&rxfds) ;
		FD_SET(fd_kbd, &rxfds) ;
		FD_SET(fd_fw, &rxfds);
		maxfd = max(fd_kbd, fd_fw);
		tv.tv_sec = 1 ;
		tv.tv_usec = 0 ;
		rc = select(maxfd + 1, &rxfds, NULL, NULL, &tv) ;
		if (rc > 0) {
			if (FD_ISSET(fd_kbd, &rxfds)) {
				int numBytes = read(fd_kbd, &kbbuf[0], sizeof(struct input_event) * MAX_KEYS);			
				for (int i = 0; i < min(MAX_KEYS, numBytes/sizeof(struct input_event)); i++) {
					switch(kbbuf[i].code){
						case KEY_LEFTALT:
							alt=kbbuf[i].value;
						break;
						case KEY_LEFTSHIFT:
							shift=kbbuf[i].value;						
						break;
					}
					if(kbbuf[i].value==1) {
					if(input_captured==1)
						handle_im(alt,shift,kbbuf[i].code);
					else if(input_captured==0)
						handle_normal(alt,shift,kbbuf[i].code);
					}
				}
			}
			if (FD_ISSET(fd_fw, &rxfds)){
				int numBytes = read(fd_fw, &fwbuf[0], sizeof(struct input_event) * MAX_KEYS);
				for (int i = 0; i < min(MAX_KEYS, numBytes/sizeof(struct input_event)); i++){
					if(fwbuf[i].value==1){
					if(input_captured==1)
						handle_im(alt,shift,fwbuf[i].code);
					else if (input_captured==0)
						handle_normal(alt,shift,fwbuf[i].code);
					}
				}
			}
		}
	}
	release_input() ;
	close_event_files() ;
	chewing_delete(ctx);
	chewing_Terminate();
	return rc ;
}

/*****************  daemonize-related stuff ********************************/

int has_console(void)
{
	return((int) (running_as_daemon == 0)) ;
}

static void child_handler(int signum)
{
    switch(signum) {
    	case SIGALRM: exit(EXIT_FAILURE); break;
    	case SIGUSR1: exit(EXIT_SUCCESS); break;
    	case SIGCHLD: exit(EXIT_FAILURE); break;
    }
}
static void daemon_handler(int signum)
{
    switch(signum)
    {
    	case SIGTERM:
    		unlink(LOCKFILE_NAME) ;
    		exit(EXIT_FAILURE);
    		break;
    }
}


static void daemonize( const char *lockfile )
{
    pid_t pid, sid, parent;
    int lfp = -1;
    struct stat statbuf ;
    char pidbuf[16] ;
    char tmpbuf[32] ;
    int n ;

    /* already a daemon */
    if ( getppid() == 1 ) return;

    if (stat(lockfile, &statbuf) == 0)
    {
        lfp = open(lockfile,O_RDWR,0640);
        if ( lfp < 0 ) {
            DIAG_PRINT("unable to open lock file %s, code=%d (%s)\n",
                    lockfile, errno, strerror(errno) );
            exit(EXIT_FAILURE);
        }
        pidbuf[0] = 0 ;
        read(lfp, &pidbuf[0], sizeof(pidbuf)-1) ;
        close(lfp) ;
        pidbuf[sizeof(pidbuf)-1] = '\0' ;
        sprintf(tmpbuf, "/proc/%s", pidbuf) ;
    }

    /* Create the lock file as the current user */
    if ( lockfile && lockfile[0] ) {
        lfp = open(lockfile,O_RDWR|O_CREAT,0640);
        if ( lfp < 0 ) {
            DIAG_PRINT("unable to create lock file %s, code=%d (%s)\n",
                    lockfile, errno, strerror(errno) );
            exit(EXIT_FAILURE);
        }
    }


    /* Trap signals that we expect to recieve */
    signal(SIGCHLD,child_handler);
    signal(SIGUSR1,child_handler);
    signal(SIGALRM,child_handler);

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        DIAG_PRINT("unable to fork daemon, code=%d (%s)\n",
                errno, strerror(errno) );

        close(lfp) ;
        unlink(lockfile) ;
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {

        /* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
           for two seconds to elapse (SIGALRM).  pause() should not return. */
        alarm(2);
        pause();

        exit(EXIT_FAILURE);
    }

    /* At this point we are executing as the child process */
    parent = getppid();

    /* Cancel certain signals */
    signal(SIGCHLD,SIG_DFL); /* A child process dies */
    signal(SIGTSTP,SIG_IGN); /* Various TTY signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);

    signal(SIGTERM, daemon_handler);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        DIAG_PRINT("unable to create a new session, code %d (%s)\n",
                errno, strerror(errno) );
        exit(EXIT_FAILURE);
    }

    n = sprintf(pidbuf, "%d", getpid()) ;
    if ((n > 0) && ((write(lfp, pidbuf, n+1)) > n))
    {
    	close(lfp) ;
    	lfp = -1 ;
    }
    else
    {
	if(fb){
		fb_close(fb);
	}
    	close(lfp) ;
    	unlink(lockfile) ;
        DIAG_PRINT("unable to write pid %s to %s -- %s\n",
                pidbuf, lockfile, strerror(errno) );
        exit(EXIT_FAILURE);
    }
}

