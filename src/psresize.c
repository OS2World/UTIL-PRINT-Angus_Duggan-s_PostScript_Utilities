/* psresize.c
 * AJCD 29/11/93
 * alter pagesize of document
 *
 * Usage:
 *      psresize [-q] [-w<dim>] [-h<dim>] [-ppaper] [-W<dim>] [-H<dim>]
 *            [-Ppaper] [in [out]]
 *              -w<dim> sets the output paper width
 *              -h<dim> sets the output paper height
 *              -ppaper sets the output paper size (width and height) by name
 *              -W<dim> sets the input paper width
 *              -H<dim> sets the input paper height
 *              -Ppaper sets the input paper size (width and height) by name
 */

#include "psutil.h"
#include "psspec.h"
#include "patchlev.h"

void usage()
{
   fprintf(stderr, "%s release %d patchlevel %d\n", prog, RELEASE, PATCHLEVEL);
   fprintf(stderr, "Usage: %s [-q] [-wwidth] [-hheight] [-ppaper] [-Wwidth] [-Hheight] [-Ppaper] [infile [outfile]]\n",
	   prog);
   fflush(stderr);
   exit(1);
}

void argerror()
{
   fprintf(stderr, "%s: bad dimension\n", prog);
   fflush(stderr);
   exit(1);
}

#define min(x,y) ((x) > (y) ? (y) : (x))
#define max(x,y) ((x) > (y) ? (x) : (y))

main(argc, argv)
     int argc;
     char *argv[];
{
   double scale, rscale;			/* page scale */
   double waste, rwaste;			/* amount wasted */
   double vshift, hshift;			/* page centring shifts */
   int rotate;
   double inwidth = -1;
   double inheight = -1;
   struct papersize *paper;
   struct pagespec *specs;

#ifdef PAPER
   if (paper = findpaper(PAPER)) {
      inwidth = width = (double)paper->width;
      inheight = height = (double)paper->height;
   }
#endif

   vshift = hshift = 0;
   rotate = 0;

   infile = stdin;
   outfile = stdout;
   verbose = 1;
   for (prog = *argv++; --argc; argv++) {
      if (argv[0][0] == '-') {
	 switch (argv[0][1]) {
	 case 'q':	/* quiet */
	    verbose = 0;
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
	 case 'W':	/* input page width */
	    inwidth = singledimen(*argv+2);
	    break;
	 case 'H':	/* input page height */
	    inheight = singledimen(*argv+2);
	    break;
	 case 'P':	/* input paper type */
	    if (paper = findpaper(*argv+2)) {
	       inwidth = (double)paper->width;
	       inheight = (double)paper->height;
	    } else {
	       fprintf(stderr, "%s: paper size '%s' not recognised\n",
		       prog, *argv+2);
	       fflush(stderr);
	       exit(1);
	    }
	    break;
	 case 'v':	/* version */
	 default:
	    usage();
	 }
      } else if (infile == stdin) {
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
   if ((infile=seekable(infile))==NULL) {
      fprintf(stderr, "%s: can't seek input\n", prog);
      fflush(stderr);
      exit(1);
   }

   if (width <= 0 || height <= 0) {
      fprintf(stderr, "%s: output page width and height must be set\n", prog);
      fflush(stderr);
      exit(1);
   }

   if (inwidth <= 0 || inheight <= 0) {
      fprintf(stderr, "%s: input page width and height must be set\n", prog);
      fflush(stderr);
      exit(1);
   }

   /* try normal orientation first */
   scale = min(width/inwidth, height/inheight);
   waste = (width-scale*inwidth)*(width-scale*inwidth) +
      (height-scale*inheight)*(height-scale*inheight);
   hshift = (width - inwidth*scale)/2;
   vshift = (height - inheight*scale)/2;

   /* try rotated orientation */
   rscale = min(height/inwidth, width/inheight);
   rwaste = (height-scale*inwidth)*(height-scale*inwidth) +
      (width-scale*inheight)*(width-scale*inheight);
   if (rwaste < waste) {
      double tmp = width;
      scale = rscale;
      hshift = (width + inheight*scale)/2;
      vshift = (height - inwidth*scale)/2;
      rotate = 1;
      width = height;
      height = tmp;
   }

   width /= scale;
   height /= scale;

   /* now construct specification list and run page rearrangement procedure */
   specs = newspec();

   if (rotate) {
      specs->rotate = 90;
      specs->flags |= ROTATE;
   }
   specs->pageno = 0;
   specs->scale = scale;
   specs->flags |= SCALE;
   specs->xoff = hshift;
   specs->yoff = vshift;
   specs->flags |= OFFSET;
      
   pstops(1, 1, 0, specs, 0.0);		/* do page rearrangement */

   exit(0);
}

