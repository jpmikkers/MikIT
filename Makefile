
CPPFLAGS = -I. -I include -O2 -DMIKIT_LINUX

OBJS = mikitux.o \
	mmod_it0.o mmod_it1.o mmod_it2.o \
	mmod_xm0.o mmod_xm1.o mmod_xm2.o \
	mmod_s30.o mmod_s31.o mmod_s32.o \
	mmod_md0.o mmod_md1.o mmod_md2.o \
	mmodule.o mdriver.o minput.o \
	mdrv_uss.o mdrv_scn.o mdrv_fmx.o

all:	mikit

mikit:	$(OBJS)
	g++ -o $@ $(OBJS)

clean:
	rm -f *.o mikit
