/* psnup.c
 * AJCD 4/6/93
 * put multiple pages onto one physical sheet of paper
 *
 * Usage:
 *      psnup [-q] [-w<dim>] [-h<dim>] [-ppaper] [-b<dim>] [-m<dim>]
 *            [-l] [-c] [-f] [-sscale] [-d<wid>] [-nup] [in [out]]
 *              -w<dim> sets the paper width
 *              -h<dim> sets the paper height
 *              -ppaper sets the paper size (width and height) by name
 *              -m<dim> sets the margin around the paper
 *              -b<dim> sets the border around each page
 *              -sscale alters the scale at which the pages are displayed
 *              -l      used if pages are in landscape orientation (rot left)
 *              -r      used if pages are in seascape orientation (rot right)
 * 		-c	for column-major layout
 *		-f	for flipped (wider than tall) pages
 * 		-d<wid>	to draw the page boundaries
 */

#include "psutil.h"
#include "psspec.h"
#include "patchlev.h"

void usage()
{
   fprintf(stderr, "%s release %d patchlevel %d\n", prog, RELEASE, PATCHLEVEL);
   fprintf(stderr, "Usage: %s [-q] [-wwidth] [-hheight] [-ppaper] [-l] [-r] [-c] [-f] [-mmargin] [-bborder] [-dlwidth] [-sscale] [-nup] [infile [outfile]]\n",
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

/* return next larger exact divisor of number, or 0 if none. There is probably
 * a much more efficient method of doing this, but the numbers involved are
 * small, so it's not a big loss. */
int nextdiv(n, m)
     int n, m;
{
   while (++n <= m) {
      if (m%n == 0)
	 return (n);
   }
   return (0);
}

main(argc, argv)
     int argc;
     char *argv[];
{
   int horiz, vert, rotate, column, flip, leftright, topbottom;
   int nup = 1;
   double draw = 0;				/* draw page borders */
   double scale;				/* page scale */
   double uscale = 0;				/* user supplied scale */
   double ppwid, pphgt;				/* paper dimensions */
   double margin, border;			/* paper & page margins */
   double vshift, hshift;			/* page centring shifts */
   double tolerance = 100000;			/* layout tolerance */
   struct papersize *paper;

#ifdef PAPER
   if (paper = findpaper(PAPER)) {
      width = (double)paper->width;
      height = (double)paper->height;
   }
#endif

   margin = border = vshift = hshift = column = flip = 0;
   leftright = topbottom = 1;

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
	 case 'l':	/* landscape (rotated left) */
	    column = !column;
	    topbottom = !topbottom;
	    break;
	 case 'r':	/* seascape (rotated right) */
	    column = !column;
	    leftright = !leftright;
	    break;
	 case 'f':	/* flipped */
	    flip = 1;
	    break;
	 case 'c':	/* column major layout */
	    column = !column;
	    break;
	 case 'w':	/* page width */
	    width = singledimen(*argv+2);
	    break;
	 case 'h':	/* page height */
	    height = singledimen(*argv+2);
	    break;
	 case 'm':	/* margins around whole page */
	    margin = singledimen(*argv+2);
	    break;
	 case 'b':	/* border around individual pages */
	    border = singledimen(*argv+2);
	    break;
	 case 't':	/* layout tolerance */
	    tolerance = atof(*argv+2);
	    break;
	 case 's':	/* override scale */
	    uscale = atof(*argv+2);
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
	 case 'n':	/* n-up, for compatibility with other psnups */
	    if (argc >= 2) {
	       argv++;
	       argc--;
	       if ((nup = atoi(*argv)) < 1) {
		  fprintf(stderr, "%s: -n %d too small\n", prog, nup);
		  fflush(stderr);
		  exit(1);
	       }
	    } else {
	       fprintf(stderr, "%s: argument expected for -n\n", prog);
	       fflush(stderr);
	       exit(1);
	    }
	    break;
	 case '1':
	 case '2':
	 case '3':
	 case '4':
	 case '5':
	 case '6':
	 case '7':
	 case '8':
	 case '9':
	    nup = atoi(*argv+1);
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
      fprintf(stderr, "%s: page width and height must be set\n", prog);
      fflush(stderr);
      exit(1);
   }

   /* subtract paper margins from height & width */
   ppwid = width - margin*2;
   pphgt = height - margin*2;

   if (ppwid <= 0 || pphgt <= 0) {
      fprintf(stderr, "%s: paper margins are too large\n", prog);
      fflush(stderr);
      exit(1);
   }

   /* Finding the best layout is an optimisation problem. We try all of the
    * combinations of width*height in both normal and rotated form, and
    * minimise the wasted space. */
   {
      double best = tolerance;
      int hor;
      for (hor = 1; hor; hor = nextdiv(hor, nup)) {
	 int ver = nup/hor;
	 /* try normal orientation first */
	 double scl = min(pphgt/(height*ver), ppwid/(width*hor));
	 double optim = (ppwid-scl*width*hor)*(ppwid-scl*width*hor) +
	    (pphgt-scl*height*ver)*(pphgt-scl*height*ver);
	 if (optim < best) {
	    best = optim;
	    /* recalculate scale to allow for internal borders */
	    scale = min((pphgt-2*border*ver)/(height*ver),
			(ppwid-2*border*hor)/(width*hor));
	    hshift = (ppwid/hor - width*scale)/2;
	    vshift = (pphgt/ver - height*scale)/2;
	    horiz = hor; vert = ver;
	    rotate = flip;
	 }
	 /* try rotated orientation */
	 scl = min(pphgt/(width*hor), ppwid/(height*ver));
	 optim = (pphgt-scl*width*hor)*(pphgt-scl*width*hor) +
	    (ppwid-scl*height*ver)*(ppwid-scl*height*ver);
	 if (optim < best) {
	    best = optim;
	    /* recalculate scale to allow for internal borders */
	    scale = min((pphgt-2*border*hor)/(width*hor),
			(ppwid-2*border*ver)/(height*ver));
	    hshift = (ppwid/ver - height*scale)/2;
	    vshift = (pphgt/hor - width*scale)/2;
	    horiz = ver; vert = hor;
	    rotate = !flip;
	 }
      }

      /* fail if nothing better than worst tolerance was found */
      if (best == tolerance) {
	 fprintf(stderr, "%s: can't find acceptable layout for %d-up\n",
		 prog, nup);
	 fflush(stderr);
	 exit(1);
      }
   }

   if (flip) {	/* swap width & height for clipping */
      double tmp = width;
      width = height;
      height = tmp;
   }

   if (rotate) {	/* rotate leftright and topbottom orders */
      int tmp = topbottom;
      topbottom = !leftright;
      leftright = tmp;
      column = !column;
   }

   /* now construct specification list and run page rearrangement procedure */
   {
      int page = 0;
      struct pagespec *specs, *tail;

      tail = specs = newspec();

      while (page < nup) {
	 int up, across;		/* page index */

	 if (column) {
	    if (leftright)		/* left to right */
	       across = page/vert;
	    else			/* right to left */
	       across = horiz-1-page/vert;
	    if (topbottom)		/* top to bottom */
	       up = vert-1-page%vert;
	    else			/* bottom to top */
	       up = page%vert;
	 } else {
	    if (leftright)		/* left to right */
	       across = page%horiz;
	    else			/* right to left */
	       across = horiz-1-page%horiz;
	    if (topbottom)		/* top to bottom */
	       up = vert-1-page/horiz;
	    else			/* bottom to top */
	       up = page/horiz;
	 }
	 if (rotate) {
	    tail->xoff = margin + (across+1)*ppwid/horiz - hshift;
	    tail->rotate = 90;
	    tail->flags |= ROTATE;
	 } else {
	    tail->xoff = margin + across*ppwid/horiz + hshift;
	 }
	 tail->pageno = page;
	 if (uscale > 0)
	    tail->scale = uscale;
	 else
	    tail->scale = scale;
	 tail->flags |= SCALE;
	 tail->yoff = margin + up*pphgt/vert + vshift;
	 tail->flags |= OFFSET;
	 if (++page < nup) {
	    tail->flags |= ADD_NEXT;
	    tail->next = newspec();
	    tail = tail->next;
	 }
      }
      
      pstops(nup, 1, 0, specs, draw);		/* do page rearrangement */
   }

   exit(0);
}

