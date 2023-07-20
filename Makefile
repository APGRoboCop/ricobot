# Ricobot Makefile for GNU/Linux

OBJ = bot.o bot_client.o dll.o engine.o h_export.o util.o

CCOPT = -DNDEBUG -mtune=generic -march=i686 -O2 -m32 \
		-mmmx -msse -msse2 -mfpmath=sse -funsafe-math-optimizations \
		-Wno-attributes -Wno-write-strings -static-libgcc

#CCDEBUG = -ggdb3 -D_DEBUG

SDKTOP = ../hlsdk-2.3-p4/multiplayer
METAMODDIR = ../metamod-p

CFLAGS = $(CCOPT) -I$(SDKTOP)/common -I$(SDKTOP)/ricochet/dlls -I$(SDKTOP)/ricochet/pm_shared -I$(SDKTOP)/engine -I$(METAMODDIR)/metamod
#CFLAGS = $(CCDEBUG) -w -I$(METAMODDIR) -I$(SDKTOP)/common -I$(SDKTOP)/engine -I$(SDKTOP)/pm_shared -I$(SDKTOP)/dlls

BASEFLAGS = -Dstricmp=strcasecmp -Dstrcmpi=strcasecmp
CPPFLAGS = ${BASEFLAGS} ${CFLAGS}

ricobot_mm.so:	${OBJ}
	gcc -fPIC -shared -o $@ ${OBJ} -ldl -lm

clean:
	-rm -f *.o
	-rm -f *.so

%.o:	%.cpp
	gcc ${CPPFLAGS} -c $< -o $@
