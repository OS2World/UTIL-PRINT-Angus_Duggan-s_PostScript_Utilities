#
#  VMS MMS build file for PS utilities
#
#  Hunter Goatley, 16-MAR-1993 14:47
#
CFLAGS = /NOLIST/OBJECT=$(MMS$TARGET)
LFLAGS = /NOTRACE/EXEC=$(MMS$TARGET)

.IFDEF __ALPHA__               #If building with Alpha cross-compilers, OBJ
OPTIONS_FILE =                  #... is defined already
LINKOPT =
CFLAGS = /NOLIST/OBJECT=$(MMS$TARGET)/STANDARD=VAXC
.ELSE
OBJ = .OBJ
EXE = .EXE
OPTIONS_FILE = ,VAXC.OPT
LINKOPT = $(options_file)/OPTIONS
.ENDIF

# epsffit fits an epsf file to a given bounding box
# psbook rearranges pages into signatures
# psselect selects page ranges
# pstops performs general page rearrangement and merging
# psnup puts multiple logical pages on one physical page

all : psbook$(exe), psselect$(exe), pstops$(exe), epsffit$(exe), psnup$(exe), psresize$(exe)
        @ write sys$output "PSUTILS build complete"

epsffit$(exe) : epsffit$(obj)$(options_file)
        $(LINK)$(LFLAGS) $(MMS$SOURCE)$(linkopt)

psnup$(exe) :   psnup$(obj), psutil$(obj), psspec$(obj)
        $(LINK)$(LFLAGS) psnup$(obj),psutil$(obj),psspec$(obj)$(linkopt)

psresize$(exe) :   psresize$(obj), psutil$(obj), psspec$(obj)
        $(LINK)$(LFLAGS) psresize$(obj),psutil$(obj),psspec$(obj)$(linkopt)

psbook$(exe) :  psbook$(obj), psutil$(obj)$(options_file)
        $(LINK)$(LFLAGS) psbook$(obj),psutil$(obj)$(linkopt)

psselect$(exe) :        psselect$(obj), psutil$(obj)
        $(LINK)$(LFLAGS) psselect$(obj),psutil$(obj)$(linkopt)

pstops$(exe) :  pstops$(obj), psutil$(obj), psspec$(obj)
        $(LINK)$(LFLAGS) pstops$(obj),psutil$(obj),psspec$(obj)$(linkopt)

psnup$(obj)  : psnup.c, psutil.h patchlev.h psspec.h

psresize$(obj)  : psresize.c, psutil.h patchlev.h psspec.h

psbook$(obj) : psbook.c, psutil.h patchlev.h

pstops$(obj) : pstops.c, psutil.h patchlev.h psspec.h

psutil$(obj) : psutil.c, psutil.h patchlev.h

psselect$(obj) : psselect.c, psutil.h patchlev.h

psspec$(obj) : psspec.c, psutil.h patchlev.h psspec.h

clean :
        delete/log *$(obj);*,psnup$(exe);*,psresize$(exe);*,psbook$(exe);*,-
                psselect$(exe);*,pstops$(exe);*,epsffit$(exe);*


