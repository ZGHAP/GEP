CC = g++ -std=c++17 -O3 -DNDEBUG 

OBJS =  main.o gp.o

LLFLAGS = -L/usr/local/lib/  
CCFLAGS = -I/usr/local/include/ -I/.
LDFLAGS = -fpermissive

ALL:$(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(CCFLAGS) $(LLFLAGS) -o GEP

#main.o:testmain.cpp
#	$(CC) -g -c testmain.cpp $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -o  main.o

gp.o:./gp.h ./gp.cpp ./sexp.cpp ./terminal.cpp ./func.cpp ./stdfunc.cpp
	$(CC) -c *.cpp  $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -o  gp.o




clean:
	rm -rf *.o gep

clean-all:
	rm -rf *.o gep 

