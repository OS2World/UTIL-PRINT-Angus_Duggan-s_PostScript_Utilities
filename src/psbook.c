/* psbook.c
 * AJCD 27/1/91
 * rearrange pages in conforming PS file for printing in signatures
 *
 * Usage:
 *       psbook [-q] [-s<signature>] [infile [outfile]]
 */

#include "psutil.h"
#include "patchlev.h"

void usage()
{
   fprintf(stderr, "%s release %d patchlevel %d\n", prog, RELEASE, PATCHLEVEL);
   fprintf(stderr, "Usage: %s [-q] [-s<signature>] [infile [outfile]]\n",
	   prog);
   fprintf(stderr, "       <signature> must be positive and divisible by 4\n");
   fflush(stderr);
   exit(1);
}


main(argc, argv)
     int argc;
     char *argv[];
{
   int signature = 0;
   int currentpg, maxpage;

   infile = stdin;
   outfile = stdout;
   verbose = 1;
   for (prog = *argv++; --argc; argv++) {
      if (argv[0][0] == '-') {
	 switch (argv[0][1]) {
	 case 's':	/* signature size */
	    signature = atoi(*argv+2);
	    if (signature < 1 || signature % 4) usage();
	    break;
	 case 'q':	/* quiet */
	    verbose = 0;
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
   scanpages();

   maxpage = pages+(4-pages%4)%4;

   if (!signature)
      signature = maxpage;

   /* rearrange pages */
   writeheader(maxpage);
   writeprolog();
   writesetup();
   for (currentpg = 0; currentpg < maxpage; currentpg++) {
      int actualpg = currentpg - currentpg%signature;
      switch(currentpg%4) {
      case 0:
      case 3:
	 actualpg += signature-1-(currentpg%signature)/2;
	 break;
      case 1:
      case 2:
	 actualpg += (currentpg%signature)/2;
	 break;
      }
      if (actualpg < pages)
	 writepage(actualpg);
      else
	 writeemptypage();
   }
   writetrailer();

   exit(0);
}
