# $Header: /home/cjm/cvs/libdprt-jni_general/c/Makefile,v 1.2 2008-07-16 15:11:42 cjm Exp $

include	../../../Makefile.common
include	../../Makefile.common
include	../Makefile.common

LIBNAME		= $(LIBDPRT_HOME)_$(JNI_HOME)
INCDIR 		= $(LIBDPRT_JNI_SRC_HOME)/include
DOCSDIR 	= $(LIBDPRT_JNI_DOC_HOME)/cdocs
BINDIR		= $(LIBDPRT_JNI_BIN_HOME)/c/$(HOSTTYPE)
CFLAGS 		= -g $(CCHECKFLAG) -I$(INCDIR) -I$(JNIINCDIR) -I$(JNIMDINCDIR) -L$(LT_LIB_HOME)
LINTFLAGS 	= -I$(INCDIR) -I$(JNIINCDIR) -I$(JNIMDINCDIR)
DOCFLAGS 	= -static
SRCS 		= dprt_jni_general.c
HEADERS		= $(SRCS:%.c=%.h)
OBJS		= $(SRCS:%.c=$(BINDIR)/%.o)
DOCS 		= $(SRCS:%.c=$(DOCSDIR)/%.html)
LIBS		= 

top: shared docs

shared: $(LT_LIB_HOME)/$(LIBNAME).so

$(LT_LIB_HOME)/$(LIBNAME).so: $(OBJS)
	$(CC) $(CCSHAREDFLAG) $(CFLAGS) $(OBJS) -o $@ $(TIMELIB) $(LIBS) $(SOCKETLIB)

$(BINDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

static: $(LT_LIB_HOME)/$(LIBNAME).a

$(LT_LIB_HOME)/$(LIBNAME).a: $(OBJS)
	ar rcv $@ $?

docs: $(DOCS)

$(DOCS) : $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

checkout:
	$(CO) $(CO_OPTIONS) $(SRCS)
	cd $(INCDIR); $(CO) $(CO_OPTIONS) $(HEADERS);

checkin:
	-$(CI) $(CI_OPTIONS) $(SRCS)
	-(cd $(INCDIR); $(CI) $(CI_OPTIONS) $(HEADERS);)

staticdepend:
	makedepend $(MAKEDEPENDFLAGS) -p$(BINDIR)/ -- $(CFLAGS) -- $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -p$(BINDIR)/ -- $(CFLAGS) -- $(SRCS)

lint:
	$(LINT)	$(LINTFLAGS) $(SRCS)

clean:
	-$(RM) $(RM_OPTIONS) $(OBJS) $(LT_LIB_HOME)/$(LIBNAME).so $(LT_LIB_HOME)/$(LIBNAME).a 
	-$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)

tidy:
	-$(RM)  $(RM_OPTIONS) $(TIDY_OPTIONS)
	-cd $(INCDIR); $(RM) $(RM_OPTIONS) $(TIDY_OPTIONS);

backup: tidy
	-$(RM) $(RM_OPTIONS) $(OBJS)

#
# $Log: not supported by cvs2svn $
# Revision 1.1  2004/03/31 16:43:53  cjm
# Initial revision
#
#

# DO NOT DELETE

/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/stdio.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/features.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/sys/cdefs.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/gnu/stubs.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/bits/types.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/bits/wordsize.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/bits/typesizes.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/libio.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/_G_config.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/wchar.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/bits/wchar.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/gconv.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/stdlib.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/string.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/math.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/bits/huge_val.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/bits/mathdef.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/include/bits/mathcalls.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/java/j2sdk1.4.2_06/include/jni.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /usr/java/j2sdk1.4.2_06/include/linux/jni_md.h
/home/dev/bin/libdprt/jni_general/c/i386-linux/dprt_jni_general.o: /home/dev/src/libdprt/jni_general/include/dprt_jni_general.h
