CC = clang
CPP = clang++
CPPFLAGS=-std=c++14

%.o: %.c ogg_packer.h
	$(CC) -c -o $@ $<

converter: ogg_packer.o converter.o connection_log.o rtp.o
	$(CPP) -o $@ $^ $(CPPFLAGS)
