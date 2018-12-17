# Makefile
# CS 332 Project 3
# Sherri Goings - last modified 12/15/2008

src=sched.c simOS.c proc.c
obj=sched.o simOS.o proc.o
inc=sched.h simOS.h proc.h
misc=Makefile
target=simOS
cflags=-Wall -g -O0
lflags=-lpthread

all: $(target)

$(target) : $(obj) $(misc)
	gcc $(cflags) $(lflags) -o $(target) $(obj)

%.o : %.c $(misc) $(inc)
	gcc $(cflags) -c -o $@ $<

clean:
	rm -f $(obj) $(target)
