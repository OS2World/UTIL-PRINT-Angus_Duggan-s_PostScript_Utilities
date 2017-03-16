/* psutil.h
 * AJCD 29/1/91
 * utilities for PS programs
 */

#include <stdio.h>
#include <ctype.h>

#ifndef LOCAL
#define LOCAL extern
#endif

#define TMPDIR "/tmp"

/* paper size structure; configurability and proper paper resources will have
   to wait until version 2 */
struct papersize {
   char *name;		/* name of paper size */
   int width, height;	/* width, height in points */
};

LOCAL char *prog;
LOCAL int pages;
LOCAL int verbose;
LOCAL FILE *infile;
LOCAL FILE *outfile;
LOCAL char pagelabel[BUFSIZ];
LOCAL int pageno;

LOCAL struct papersize *findpaper();
LOCAL FILE *seekable();
LOCAL void writepage();
LOCAL void seekpage();
LOCAL void writepageheader();
LOCAL void writepagesetup();
LOCAL void writepagebody();
LOCAL void writeheader();
LOCAL int writepartprolog();
LOCAL void writeprolog();
LOCAL void writesetup();
LOCAL void writetrailer();
LOCAL void writeemptypage();
LOCAL void scanpages();
LOCAL void writestring();
LOCAL void message();

extern long lseek();
extern long ftell();
extern char *getenv();
#ifdef VMS
#define unlink delete
#endif

