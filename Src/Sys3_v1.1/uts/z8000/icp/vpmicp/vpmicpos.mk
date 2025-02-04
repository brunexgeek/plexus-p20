INCRT = $(ROOT)/usr/include
CFLAGS = -O -DICP -Dz8000 -DVPMSYS -I$(INCRT)
FRC =

FILES =\
	get.o\
	interp.o\
        xmt.o\
        xeom.o\
	getxbuf.o\
	rtnxbuf.o\
	rcv.o\
	put.o\
	getrbuf.o\
	rtnrbuf.o\
	proto0.o\
	proto1.o\
	proto2.o\
	proto3.o\
	fetch.o

all:	$(FILES)

proto0.o:
	/lib/cpp -DASSM -DICP -DVPMSYS -Dz8000 -I$(INCRT) -P  $< >tempfile
	as -u tempfile -o $@
	rm tempfile
	$(FRC)

proto1.o:
	/lib/cpp -DASSM -DICP -DVPMSYS -Dz8000 -I$(INCRT) -P  $< >tempfile
	as -u tempfile -o $@
	rm tempfile
	$(FRC)

proto2.o:
	/lib/cpp -DASSM -DICP -DVPMSYS -Dz8000 -I$(INCRT) -P  $< >tempfile
	as -u tempfile -o $@
	rm tempfile
	$(FRC)

proto3.o:
	/lib/cpp -DASSM -DICP -DVPMSYS -Dz8000 -I$(INCRT) -P  $< >tempfile
	as -u tempfile -o $@
	rm tempfile
	$(FRC)

get.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

interp.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

xmt.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

xeom.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

getxbuf.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

rtnxbuf.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

rcv.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

put.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

getrbuf.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

rtnrbuf.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h
	cc $(CFLAGS) -c $<
	$(FRC)

fetch.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/icp/sio.h\
	$(INCRT)/icp/icp.h\
	$(INCRT)/icp/opdef.h\
	$(INCRT)/icp/crctab.h\
	$(INCRT)/icp/atoetbl.h\
	$(INCRT)/icp/etoatbl.h
	cc $(CFLAGS) -c $<
	$(FRC)

print:
	lnum ../vpmicp/*.[sc] > /dev/lp

FRC:
