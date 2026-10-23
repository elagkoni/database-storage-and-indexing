# compile with gcc
CC=g++

# Set compiler options here.
CXXFLAGS = -g -Wall -Wextra -std=c++11

# set the name of the executable file to compile here
PROGRAM = ysbd

OBJS = src/main.o src/abstractfile.o src/abstracthashtable.o src/hashtable.o src/hashtableinfo.o \
    src/secondary.o src/secondaryinfo.o src/hashstatistics.o src/heapfile.o src/heapfileinfo.o src/utilities.o src/tests.o

ifeq ($(MODE), TESTING)
	CXXFLAGS += -D TESTING
endif

CXXFLAGS += -D MAKEFILE

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM) lib/BF_64.a -no-pie

clean:
	rm -f $(PROGRAM) $(OBJS)

run: $(PROGRAM)
	./$(PROGRAM)
