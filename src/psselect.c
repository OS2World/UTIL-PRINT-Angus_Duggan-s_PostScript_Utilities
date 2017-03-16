/* psselect.c
 * AJCD 27/1/91
 * rearrange pages in conforming PS file for printing in signatures
 *
 * Usage:
 *       psselect [-q] [-e] [-o] [-r] [-p<pages>] [infile [outfile]]
 */

#include "psutil.h"
#include "patchlev.h"

void usage()
{
   fprintf(stderr, "%s release %d patchlevel %d\n", prog, RELEASE, PATCHLEVEL);
   fprintf(stderr,
	   "Usage: %s [-q] [-e] [-o] [-r] [-p<pages>] [infile [outfile]]\n",
	   prog);
   fflush(stderr);
   exit(1);
}

struct pgrange {
   int first, last;
   struct pgrange *next;
};

typedef struct pgrange range;

range * makerange(beg, end, next)
     int beg, end;
     range *next;
{
   range *new;
   if ((new = (range *)malloc(sizeof(range))) == NULL) {
      fprintf(stderr, "%s: out of memory\n", prog);
      fflush(stderr);
      exit(1);
   }
   new->first = beg;
   new->last = end;
   new->next = next;
   return (new);
}


range * addrange(str, rp)
     char *str;
     range *rp;
{
   int first=0;
   int sign;
   sign = (*str == '_' && ++str) ? -1 : 1;
   if (isdigit(*str)) {
      first = sign*atoi(str);
      while (isdigit(*str)) str++;
   }
   switch (*str) {
   case '\0':
      if (first)
	 return (makerange(first, first, rp));
      break;
   case ',':
      if (first)
	 return (addrange(str+1, makerange(first, first, rp)));
      break;
   case '-':
   case ':':
      str++;
      sign = (*str == '_' && ++str) ? -1 : 1;
      if (isdigit(*str)) {
	 int last = sign*atoi(str);
	 while (isdigit(*str)) str++;
	 if (!first)
	    first = 1;
	 if (last >= first) 
	    switch (*str) {
	    case '\0':
	       return (makerange(first, last, rp));
	    case ',':
	       return (addrange(str+1, makerange(first, last, rp)));
	    }
      } else if (*str == '\0')
	 return (makerange(first, -1, rp));
      else if (*str == ',')
	 return (addrange(str+1, makerange(first, -1, rp)));
   }
   fprintf(stderr, "%s: invalid page range\n", prog);
   fflush(stderr);
   exit(1);
}


main(argc, argv)
     int argc;
     char *argv[];
{
   int currentpg, maxpage = 0;
   int even = 0, odd = 0, reverse = 0;
   int pass, all;
   range *pagerange = NULL;

   infile = stdin;
   outfile = stdout;
   verbose = 1;
   for (prog = *argv++; --argc; argv++) {
      if (argv[0][0] == '-') {
	 switch (argv[0][1]) {
	 case 'e':	/* even pages */
	    even = 1;
	    break;
	 case 'o':	/* odd pages */
	    odd = 1;
	    break;
	 case 'r':	/* reverse */
	    reverse = 1;
	    break;
	 case 'p':	/* page spec */
	    pagerange = addrange(*argv+2, pagerange);
	    break;
	 case 'q':	/* quiet */
	    verbose = 0;
	    break;
	 case 'v':	/* version */
	 default:
	    usage();
	 }
      } else if (pagerange == NULL && !reverse && !even && !odd) {
	 pagerange = addrange(*argv, NULL);
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

   /* reverse page list if not reversing pages (list constructed bottom up) */
   if (!reverse) {
      range *revlist = NULL;
      range *next = NULL;
      while (pagerange) {
	 next = pagerange->next;
	 pagerange->next = revlist;
	 revlist = pagerange;
	 pagerange = next;
      }
      pagerange = revlist;
   }

   /* select all pages or all in range if odd or even not set */
   all = !(odd || even);

   /* count pages on first pass, select pages on second pass */
   for (pass = 0; pass < 2; pass++) {
      if (pass) {                           /* write header on second pass */
	 writeheader(maxpage);
	 writeprolog();
	 writesetup();
      }
      if (pagerange) {
	 range *r;
	 for (r = pagerange; r; r = r->next) {
	    if (pagerange->first < 0) {
	       pagerange->first += pages + 1;
	       if (pagerange->first < 0)
		  pagerange->first = 0;
	    }
	    if (pagerange->last < 0) {
	       pagerange->last += pages + 1;
	       if (pagerange->last < 0)
		  pagerange->last = 0;
	    }
	    if (reverse) {
	       for (currentpg = r->last; currentpg >= r->first; currentpg--) {
		  if (currentpg <= pages &&
		      ((currentpg&1) ? (odd || all) : (even || all))) {
		     if (pass)
			writepage(currentpg-1);
		     else
			maxpage++;
		  }
	       }
	    } else {
	       for (currentpg = r->first; currentpg <= r->last; currentpg++) {
		  if (currentpg <= pages &&
		      ((currentpg&1) ? (odd || all) : (even || all))) {
		     if (pass)
			writepage(currentpg-1);
		     else
			maxpage++;
		  }
	       }
	    }
	 }
      } else if (reverse) {
	 for (currentpg = pages; currentpg > 0; currentpg--)
	    if ((currentpg&1) ? (odd || all) : (even || all)) {
	       if (pass)
		  writepage(currentpg-1);
	       else
		  maxpage++;
	    }
      } else {
	 for (currentpg = 1; currentpg <= pages; currentpg++)
	    if ((currentpg&1) ? (odd || all) : (even || all)) {
	       if (pass)
		  writepage(currentpg-1);
	       else
		  maxpage++;
	    }
      }
   }
   writetrailer();

   exit(0);
}
