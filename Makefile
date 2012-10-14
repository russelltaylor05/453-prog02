CC 	= gcc

CFLAGS  = -Wall -g -I ../include

LD 	= gcc

LDFLAGS  = -Wall -g

PUBFILES =  README  hungrymain.c  libPLN.a  libsnakes.a  lwp.h\
	    numbersmain.c  snakemain.c  snakes.h

TARGET =  pn-cs453@hornet:Given/asgn2

PROGS	= snakes nums hungry

SNAKEOBJS  = snakemain.o 

HUNGRYOBJS = hungrymain.o 

NUMOBJS    = numbersmain.o

OBJS	= $(SNAKEOBJS) $(HUNGRYOBJS) $(NUMOBJS) 

SRCS	= snakemain.c numbersmain.c


EXTRACLEAN = core $(PROGS)

all: 	$(PROGS)

allclean: clean
	@rm -f $(EXTRACLEAN)

clean:	
	rm -f  *.o $(OBJS) *~ TAGS nums hungry snakes

snakes: snakemain.o libPLN.a libsnakes.a liblwp.a  
	$(LD) $(LDFLAGS) -o snakes snakemain.o -L. -lncurses -lsnakes -llwp -lPLN 

hungry: hungrymain.o libPLN.a libsnakes.a liblwp.a
	$(LD) $(LDFLAGS) -o hungry hungrymain.o -L. -lncurses -lsnakes -llwp -lPLN 

nums: numbersmain.o libPLN.a liblwp.a
	$(LD) $(LDFLAGS) -o nums numbersmain.o -L. -llwp -lPLN

hungrymain.o: lwp.h snakes.h

snakemain.o: lwp.h snakes.h

numbermain.o: lwp.h

liblwp.a: lwp.o
	ar rcs liblwp.a lwp.o

lwp.o: lwp.c
	$(LD) $(LDFLAGS) -c lwp.c

pub:
	scp $(PUBFILES) $(TARGET)

