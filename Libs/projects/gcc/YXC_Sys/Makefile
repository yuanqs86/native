# Makefile

prefix=/usr/local
LIBDIR=$(DESTDIR)${prefix}/lib
SHLIBDIR=$(DESTDIR)${prefix}/bin
INCDIR=$(DESTDIR)${prefix}/include
ARCH=X86
CC=gcc
CXX=g++
CFLAGS=-O3 -ffast-math -m32 -I../../../include -I../../../src \
	-march=i486 -mfpmath=sse -msse -fomit-frame-pointer
DEPMM=-MM -g0
DEPMT=-MT
LD=gcc -o 
AR=ar rc 
RANLIB=ranlib
STRIP=strip
AS=yasm
ASFLAGS= -O2 -f win32 -DPREFIX -DHAVE_ALIGNED_STACK=1 -DHIGH_BIT_DEPTH=0 -DBIT_DEPTH=8 -I/usr/local/include
RC=windres
RCFLAGS=--target=pe-i386  -I. -o 
EXE=.exe
HAVE_GETOPT_LONG=1
DEVNULL=NUL
PROF_GEN_CC=-fprofile-generate
PROF_GEN_LD=-fprofile-generate
PROF_USE_CC=-fprofile-use
PROF_USE_LD=-fprofile-use
LDFLAGSCLI = 
LIBEJ_SYS=libYXC_Sys.a
SONAME=libYXC_Sys.so
IMPLIBNAME=libYXC_Sys.so.a
SOFLAGS=-shared
SRCPATH=../../../src/YXC_Sys

all: default
default: lib-shared

SRCS = $(SRCPATH)/OS/YXC_Sys.cpp 			\
	$(SRCPATH)/OS/YXC_FilePath.cpp 			\
	$(SRCPATH)/OS/YXC_OSUtil.cpp			\
	$(SRCPATH)/OS/YXC_SysCExtend.cpp			\
	$(SRCPATH)/OS/YXC_Locker.cpp			\
	$(SRCPATH)/OS/_YXC_CondVar.cpp			\
	$(SRCPATH)/OS/YXC_CondVar.cpp			\
	$(SRCPATH)/OS/YXC_File.cpp			\
	$(SRCPATH)/OS/_YXC_FileBase.cpp			\
	$(SRCPATH)/OS/_YXC_OSFile.cpp			\
	$(SRCPATH)/OS/_YXC_StdCFile.cpp			\
	$(SRCPATH)/OS/YXC_ThreadControllor.cpp		\
	$(SRCPATH)/MM/YXC_LocalMM.cpp			\
	$(SRCPATH)/MM/YXC_MMBase.cpp			\
	$(SRCPATH)/MM/_YXC_MMModelFixed.cpp		\
	$(SRCPATH)/MM/_YXC_MMModelFlat.cpp		\
	$(SRCPATH)/MM/_YXC_PNCMP.cpp			\
	$(SRCPATH)/MM/YXC_PNCMP.cpp			\
	$(SRCPATH)/MM/_YXC_PNCMPConsumer.cpp		\
	$(SRCPATH)/MM/_YXC_PNCMPBlockQueue.cpp		\
	$(SRCPATH)/Utils/_YXC_ConsoleLogger.cpp		\
	$(SRCPATH)/Utils/_YXC_FileLogger.cpp		\
	$(SRCPATH)/Utils/YXC_IMemoryStream.cpp		\
	$(SRCPATH)/Utils/YXC_OMemoryStream.cpp		\
	$(SRCPATH)/Utils/YXC_Logger.cpp			\
	$(SRCPATH)/Utils/_YXC_LoggerBase.cpp		\
	$(SRCPATH)/Utils/YXC_MemoryStream.cpp		\
	$(SRCPATH)/Utils/YXC_Profile.cpp			\
	$(SRCPATH)/Utils/_YXC_ProfileImpl.cpp		\
	$(SRCPATH)/Utils/YXC_StrUtil.cpp			\
	$(SRCPATH)/Utils/_YXC_TextConverter.cpp		\
	$(SRCPATH)/Utils/_YXC_TextFile.cpp		\
	$(SRCPATH)/Utils/YXC_TextFile.cpp		\
	$(SRCPATH)/Net/YXC_NCMClient.cpp			\
	$(SRCPATH)/Net/YXC_NCMSyncClient.cpp		\
	$(SRCPATH)/Net/_YXC_NetCommon.cpp		\
	$(SRCPATH)/Net/YXC_NetGCSocket.cpp		\
	$(SRCPATH)/Net/YXC_NetMarshal.cpp		\
	$(SRCPATH)/Net/YXC_NetSModelClient.cpp		\
	$(SRCPATH)/Net/YXC_HttpNSMClient.cpp		\
	$(SRCPATH)/Net/YXC_HttpReqBody.cpp		\
	$(SRCPATH)/Net/YXC_HttpResBody.cpp		\
	$(SRCPATH)/Net/_YXC_NetSModelClient.cpp		\
	$(SRCPATH)/Net/_YXC_NetSModelClientMgr.cpp	\
	$(SRCPATH)/Net/_YXC_NetSModelClientThrMgr.cpp	\
	$(SRCPATH)/Net/_YXC_NetSModelServer.cpp		\
	$(SRCPATH)/Net/YXC_NetSModelServer.cpp		\
	$(SRCPATH)/Net/_YXC_NetSModelServerGroup.cpp	\
	$(SRCPATH)/Net/YXC_NSMClientBase.cpp		\
	$(SRCPATH)/Net/YXC_NSMGroupBase.cpp		\
	$(SRCPATH)/Net/YXC_NSMServerBase.cpp		\
	$(SRCPATH)/Net/YXC_NSUtility.cpp			\
	$(SRCPATH)/Net/YXC_P2PClient.cpp			\
	$(SRCPATH)/Net/YXC_P2PServer.cpp			\
	$(SRCPATH)/Net/YXC_NPBase.cpp			\
	$(SRCPATH)/Net/YXC_NetSocket.cpp

		
SRCSO = 

OBJS = 
OBJSO =

OBJS   += $(SRCS:%.cpp=%.o)
OBJSO  += $(SRCSO:%.cpp=%.o)

.PHONY: all default clean

lib-static: $(LIBEJ_SYS)
lib-shared: $(SONAME)

$(LIBEJ_SYS): $(OBJS)
	rm -f $(LIBEJ_SYS)
	$(AR) $@ $(OBJS)
	$(if $(RANLIB), $(RANLIB) $@)

$(SONAME): $(OBJS) $(OBJSO)
	$(LD) $@ $(OBJS) $(OBJASM) $(OBJSO) $(SOFLAGS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	rm -f $(OBJS) $(OBJSO) $(SONAME) *.a *.lib *.exp *.pdb
