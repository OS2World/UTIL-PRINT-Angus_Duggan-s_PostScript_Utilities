/* psspec.h
 * AJCD 5/6/93
 * routines for page rearrangement specs
 */

#ifndef LOCAL
#define LOCAL extern
#endif

/* pagespec flags */
#define ADD_NEXT (0x01)
#define ROTATE   (0x02)
#define SCALE    (0x04)
#define OFFSET   (0x08)
#define GSAVE    (ROTATE|SCALE|OFFSET)

struct pagespec {
   int reversed, pageno, flags, rotate;
   double xoff, yoff, scale;
   struct pagespec *next;
};

LOCAL double width, height;

LOCAL struct pagespec *newspec();
LOCAL int parseint();
LOCAL double parsedouble();
LOCAL double parsedimen();
LOCAL double singledimen();
LOCAL void pstops();

extern double atof();
