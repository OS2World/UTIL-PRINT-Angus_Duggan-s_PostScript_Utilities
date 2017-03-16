/* pstops.c
 * AJCD 27/1/91
 * rearrange pages in conforming PS file for printing in signatures
 *
 * Usage:
 *       pstops [-q] [-b] [-d] [-w<dim>] [-h<dim>] [-ppaper] <pagespecs> [infile [outfile]]
 */

#include "psutil.h"
#include "psspec.h"
#include "patchlev.h"

void usage()
{
   fprintf(stderr, "%s release %d patchlevel %d\n", prog, RELEASE, PATCHLEVEL);
   fprintf(stderr, "Usage: %s [-q] [-b] [-wwidth] [-hheight] [-dlwidth] [-ppaper] <pagespecs> [infile [outfile]]\n",
	   prog);
   fflush(stderr);
   exit(1);
}

void argerror()
{
   fprintf(stderr, "%s: page specification error:\n", prog);
   fprintf(stderr, "  <pagespecs> = [modulo:]<spec>\n");
   fprintf(stderr, "  <spec>      = [-]pageno[@scale][L|R|U][(xoff,yoff)][,spec|+spec]\n");
   fprintf(stderr, "                modulo>=1, 0<=pageno<modulo\n");
   fflush(stderr);
   exit(1);
}

static int modulo = 1;
static int pagesperspec = 1;

struct pagespec *parsespecs(str)
     char *str;
{
   char *t;
   struct pagespec *head, *tail;
   int other = 0;
   int num = -1;

   head = tail = newspec();
   while (*str) {
      if (isdigit(*str)) {
	 num = parseint(&str);
      } else {
	 switch (*str++) {
	 case ':':
	    if (other || head != tail || num < 1) argerror();
	    modulo = num;
	    num = -1;
	    break;
	 case '-':
	    tail->reversed = !tail->reversed;
	    break;
	 case '@':
	    if (num < 0) argerror();
	    tail->scale *= parsedouble(&str);
	    tail->flags |= SCALE;
	    break;
	 case 'l': case 'L':
	    tail->rotate += 90;
	    tail->flags |= ROTATE;
	    break;
	 case 'r': case 'R':
	    tail->rotate -= 90;
	    tail->flags |= ROTATE;
	    break;
	 case 'u': case 'U':
	    tail->rotate += 180;
	    tail->flags |= ROTATE;
	    break;
	 case '(':
	    tail->xoff += parsedimen(&str);
	    if (*str++ != ',') argerror();
	    tail->yoff += parsedimen(&str);
	    if (*str++ != ')') argerror();
	    tail->flags |= OFFSET;
	    break;
	 case '+':
	    tail->flags |= ADD_NEXT;
	 case ',':
	    if (num < 0 || num >= modulo) argerror();
	    if ((tail->flags & ADD_NEXT) == 0)
	       pagesperspec++;
	    tail->pageno = num;
	    tail->next = newspec();
	    tail = tail->next;
	    num = -1;
	    break;
	 default:
	    argerror();
	 }
	 other = 1;
      }
   }
   if (num >= modulo)
      argerror();
   else if (num >= 0)
      tail->pageno = num;
   return (head);
}

main(argc, argv)
     int argc;
     char *argv[];
{
   struct pagespec *specs = NULL;
   int nobinding = 0;
   double draw = 0;
   struct papersize *paper;

#ifdef PAPER
   if (paper = findpaper(PAPER)) {
      width = (double)paper->width;
      height = (double)paper->height;
   }
#endif

   infile = stdin;
   outfile = stdout;
   verbose = 1;
   for (prog = *argv++; --argc; argv++) {
      if (argv[0][0] == '-') {
	 switch (argv[0][1]) {
	 case 'q':	/* quiet */
	    verbose = 0;
	    break;
	 case 'd':	/* draw borders */
	    if (argv[0][2])
	       draw = singledimen(*argv+2);
	    else
	       draw = 1;
	    break;
	 case 'b':	/* no bind operator */
	    nobinding = 1;
	    break;
	 case 'w':	/* page width */
	    width = singledimen(*argv+2);
	    break;
	 case 'h':	/* page height */
	    height = singledimen(*argv+2);
	    break;
	 case 'p':	/* paper type */
	    if (paper = findpaper(*argv+2)) {
	       width = (double)paper->width;
	       height = (double)paper->height;
	    } else {
	       fprintf(stderr, "%s: paper size '%s' not recognised\n",
		       prog, *argv+2);
	       fflush(stderr);
	       exit(1);
	    }
	    break;
	 case 'v':	/* version */
	    usage();
	 default:
	    if (specs == NULL)
	       specs = parsespecs(*argv);
	    else
	       usage();
	 }
      } else if (specs == NULL)
	 specs = parsespecs(*argv);
      else if (infile == stdin) {
	 if ((infile = fopen(*argv, "r")) == NULL) {
	    fprintf(stderr, "%s: can't open input file %s\n", prog, *argv);
	    fflush(stderr);
	    exit(1);
	 }
      } else if (outfile == stdout) {
	 if ((outfile = fopen(*argv, "w")) == NULL) {
	    fprintf(stderr, "%s: can't open output file %s\n", prog, *argv);
	    fflush(stderr);
	    exit(1);
	 }
      } else usage();
   }
   if (specs == NULL)
      usage();
   if ((infile=seekable(infile))==NULL) {
      fprintf(stderr, "%s: can't seek input\n", prog);
      fflush(stderr);
      exit(1);
   }

   pstops(modulo, pagesperspec, nobinding, specs, draw);

   exit(0);
}
