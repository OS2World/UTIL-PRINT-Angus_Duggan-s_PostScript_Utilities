/* psutil.c
 * AJCD 29/1/91
 * utilities for PS programs
 */

/*
 *  AJCD 6/4/93
 *    Changed to using ftell() and fseek() only (no length calculations)
 *  Hunter Goatley    31-MAY-1993 23:33
 *    Fixed VMS support.
 *  Hunter Goatley     2-MAR-1993 14:41
 *    Added VMS support.
 */
#define LOCAL
#include "psutil.h"
#include "patchlev.h"

#ifdef VMS
#include <file.h>
#else
#include <fcntl.h>
#endif
#include <string.h>

#define iscomment(x,y) (strncmp(x,y,strlen(y)) == 0)

extern void argerror();

static char buffer[BUFSIZ];
static long bytes = 0;
static long pagescmt = 0;
static long headerpos = 0;
static long endsetup = 0;
static long beginprocset = 0;		/* start of pstops procset */
static long endprocset = 0;
static int outputpage = 0;
static int maxpages = 100;
static long *pageptr;

/* list of paper sizes supported */
static struct papersize papersizes[] = {
   { "a3", 842, 1191 },		/* 29.7cm * 42cm */
   { "a4", 595, 842 },		/* 21cm * 29.7cm */
   { "a5", 421, 595 },		/* 14.85cm * 21cm */
   { "b5", 516, 729 },		/* 18.2cm * 25.72cm */
   { "A3", 842, 1191 },		/* 29.7cm * 42cm */
   { "A4", 595, 842 },		/* 21cm * 29.7cm */
   { "A5", 421, 595 },		/* 14.85cm * 21cm */
   { "B5", 516, 729 },		/* 18.2cm * 25.72cm */
   { "letter", 612, 792 },	/* 8.5in * 11in */
   { "legal", 612, 1008 },	/* 8.5in * 14in */
   { "ledger", 1224, 792 },	/* 17in * 11in */
   { "tabloid", 792, 1224 },	/* 11in * 17in */
   { "statement", 396, 612 },	/* 5.5in * 8.5in */
   { "executive", 540, 720 },	/* 7.6in * 10in */
   { "folio", 612, 936 },	/* 8.5in * 13in */
   { "quarto", 610, 780 },	/* 8.5in * 10.83in */
   { "10x14", 720, 1008 },	/* 10in * 14in */
   { NULL, 0, 0 }
};

/* return pointer to paper size struct or NULL */
struct papersize* findpaper(name)
     char *name;
{
   struct papersize *pp;
   for (pp = papersizes; pp->name; pp++) {
      if (strcmp(pp->name, name) == 0) {
	 return pp;
      }
   }
   return NULL;
}

/* make a file seekable; trick stolen from Chris Torek's libdvi */
FILE *seekable(fp)
     FILE *fp;
{
   int fd, tf, n, w;
   char *tmpdir, *p;

   fd = fileno(fp);
   if (lseek(fd, 0L, 1) >= 0 && !isatty(fd))
      return (fp);

#ifdef MSDOS
   fprintf(stderr, "%s: input is not seekable\n", prog);
   fflush(stderr);
   exit(1);
#else
   if ((tmpdir = getenv("TMPDIR")) == NULL)
      tmpdir = TMPDIR;
   (void) sprintf(buffer, "%s/#%d", tmpdir, getpid());
   if ((tf = open(buffer, O_RDWR | O_CREAT | O_EXCL, 0666)) == -1)
      return (NULL);
   (void) unlink(buffer);

   while ((n = read(fd, p = buffer, BUFSIZ)) > 0) {
      do {
	 if ((w = write(tf, p, n)) < 0) {
	    (void) close(tf);
	    (void) fclose(fp);
	    return (NULL);
	 }
	 p += w;
      } while ((n -= w) > 0);
   }
   if (n < 0) {
      (void) close(tf);
      (void) fclose(fp);
      return (NULL);
   }

   /* discard the input file, and rewind and open the temporary */
   (void) fclose(fp);
   (void) lseek(tf, 0L, 0);
   if ((fp = fdopen(tf, "r")) == NULL) {
      (void) close(tf);
   }
   return (fp);
#endif
}


/* copy input file from current position upto new position to output file */
static int fcopy(upto)
     long upto;
{
   long here = ftell(infile);
   while (here < upto) {
      if ((fgets(buffer, BUFSIZ, infile) == NULL) ||
	  (fputs(buffer, outfile) == EOF))
	 return(0);
      here = ftell(infile);
      bytes += strlen(buffer);
   }
   return (1);
}

/* build array of pointers to start/end of pages */
void scanpages()
{
   register char *comment = buffer+2;
   register int nesting = 0;
   register long int record;

   if ((pageptr = (long *)malloc(sizeof(long)*maxpages)) == NULL) {
      fprintf(stderr, "%s: out of memory\n", prog);
      fflush(stderr);
      exit(1);
   }
   pages = 0;
   fseek(infile, 0L, 0);
   while (record = ftell(infile), fgets(buffer, BUFSIZ, infile) != NULL)
      if (*buffer == '%') {
	 if (buffer[1] == '%') {
	    if (nesting == 0 && iscomment(comment, "Page:")) {
	       if (pages >= maxpages-1) {
		  maxpages *= 2;
		  if ((pageptr = (long *)realloc((char *)pageptr,
					     sizeof(long)*maxpages)) == NULL) {
		     fprintf(stderr, "%s: out of memory\n", prog);
		     fflush(stderr);
		     exit(1);
		  }
	       }
	       pageptr[pages++] = record;
	    } else if (headerpos == 0 && iscomment(comment, "Pages:"))
	       pagescmt = record;
	    else if (headerpos == 0 && iscomment(comment, "EndComments"))
	       headerpos = ftell(infile);
	    else if (iscomment(comment, "BeginDocument") ||
		     iscomment(comment, "BeginBinary") ||
		     iscomment(comment, "BeginFile"))
	       nesting++;
	    else if (iscomment(comment, "EndDocument") ||
		     iscomment(comment, "EndBinary") ||
		     iscomment(comment, "EndFile"))
	       nesting--;
	    else if (nesting == 0 && iscomment(comment, "EndSetup"))
	       endsetup = record;
	    else if (nesting == 0 &&
		       iscomment(comment, "BeginProcSet: PStoPS"))
	       beginprocset = record;
	    else if (beginprocset && !endprocset &&
		     iscomment(comment, "EndProcSet"))
	       endprocset = ftell(infile);
	    else if (nesting == 0 && iscomment(comment, "Trailer")) {
	       fseek(infile, record, 0);
	       break;
	    }
	 } else if (headerpos == 0 && buffer[1] != '!')
	    headerpos = record;
      } else if (headerpos == 0)
	 headerpos = record;
   pageptr[pages] = ftell(infile);
   if (endsetup == 0)
      endsetup = pageptr[0];
}

/* seek a particular page */
void seekpage(p)
     int p;
{
   fseek(infile, pageptr[p], 0);
   if (fgets(buffer, BUFSIZ, infile) != NULL &&
       iscomment(buffer, "%%Page:")) {
      char *start, *end;
      for (start = buffer+7; isspace(*start); start++);
      if (*start == '(') {
	 int paren = 1;
	 for (end = start+1; paren > 0; end++)
	    switch (*end) {
	    case '\0':
	       fprintf(stderr,
		       "%s: Bad page label while seeking page %d\n", prog, p);
	       fflush(stderr);
	       exit(1);
	    case '(':
	       paren++;
	       break;
	    case ')':
	       paren--;
	       break;
	    }
      } else
	 for (end = start; !isspace(*end); end++);
      strncpy(pagelabel, start, end-start);
      pagelabel[end-start] = '\0';
      pageno = atoi(end);
   } else {
      fprintf(stderr, "%s: I/O error seeking page %d\n", prog, p);
      fflush(stderr);
      exit(1);
   }
}

/* Output routines. These all update the global variable bytes with the number
 * of bytes written */
void writestring(s)
     char *s;
{
   fputs(s, outfile);
   bytes += strlen(s);
}

/* write page comment */
void writepageheader(label, page)
     char *label;
     int page;
{
   if (verbose) {
      sprintf(buffer, "[%d] ", page);
      message(buffer);
   }
   sprintf(buffer, "%%%%Page: %s %d\n", label, ++outputpage);
   writestring(buffer);
}

/* search for page setup */
void writepagesetup()
{
   char buffer[BUFSIZ];
   if (beginprocset) {
      for (;;) {
	 if (fgets(buffer, BUFSIZ, infile) == NULL) {
	    fprintf(stderr, "%s: I/O error reading page setup %d\n", prog,
		    outputpage);
	    fflush(stderr);
	    exit(1);
	 }
	 if (!strncmp(buffer, "PStoPSxform", 11))
	    break;
	 if (fputs(buffer, outfile) == EOF) {
	    fprintf(stderr, "%s: I/O error writing page setup %d\n", prog,
		    outputpage);
	    fflush(stderr);
	    exit(1);
	 }
	 bytes += strlen(buffer);
      }
   }
}

/* write the body of a page */
void writepagebody(p)
     int p;
{
   if (!fcopy(pageptr[p+1])) {
      fprintf(stderr, "%s: I/O error writing page %d\n", prog, outputpage);
      fflush(stderr);
      exit(1);
   }
}

/* write a whole page */
void writepage(p)
     int p;
{
   seekpage(p);
   writepageheader(pagelabel, p+1);
   writepagebody(p);
}

/* write from start of file to end of header comments */
void writeheader(p)
     int p;
{
   fseek(infile, 0L, 0);
   if (pagescmt) {
      if (!fcopy(pagescmt) || fgets(buffer, BUFSIZ, infile) == NULL) {
	 fprintf(stderr, "%s: I/O error in header\n", prog);
	 fflush(stderr);
	 exit(1);
      }
      sprintf(buffer, "%%%%Pages: %d 0\n", p);
      writestring(buffer);
   }
   if (!fcopy(headerpos)) {
      fprintf(stderr, "%s: I/O error in header\n", prog);
      fflush(stderr);
      exit(1);
   }
}

/* write prologue to end of setup section excluding PStoPS procset */
int writepartprolog()
{
   if (beginprocset && !fcopy(beginprocset)) {
      fprintf(stderr, "%s: I/O error in prologue\n", prog);
      fflush(stderr);
      exit(1);
   }
   if (endprocset)
      fseek(infile, endprocset, 0);
   writeprolog();
   return !beginprocset;
}

/* write prologue up to end of setup section */
void writeprolog()
{
   if (!fcopy(endsetup)) {
      fprintf(stderr, "%s: I/O error in prologue\n", prog);
      fflush(stderr);
      exit(1);
   }
}

/* write from end of setup to start of pages */
void writesetup()
{
   if (!fcopy(pageptr[0])) {
      fprintf(stderr, "%s: I/O error in prologue\n", prog);
      fflush(stderr);
      exit(1);
   }
}

/* write trailer */
void writetrailer()
{
   fseek(infile, pageptr[pages], 0);
   while (fgets(buffer, BUFSIZ, infile) != NULL) {
      writestring(buffer);
   }
   if (verbose) {
      sprintf(buffer, "Wrote %d pages, %ld bytes\n", outputpage, bytes);
      message(buffer);
   }
}

/* write message to stderr */
void message(s)
     char *s;
{
   static int pos = 0;
   char *nl = strchr(s, '\n');
   int len = nl ? (nl-s) : strlen(s);

   if (pos+len > 79 && (pos > 79 || len < 80)) {
      fputc('\n', stderr);
      pos = 0;
   }
   fputs(s, stderr);
   fflush(stderr);
   pos += len;
}

/* write a page with nothing on it */
void writeemptypage()
{
   if (verbose)
      message("[*] ");
   sprintf(buffer, "%%%%Page: * %d\nshowpage\n", ++outputpage);
   writestring(buffer);
}

