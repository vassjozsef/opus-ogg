CC = clang
CXX = clang++
CXXFLAGS=-std=c++14

converter: ogg_packer.o opus_header.o converter.o connection_log.o rtp.o
	$(CXX) $(CXXFLAGS) ogg_packer.o opus_header.o converter.o connection_log.o rtp.o -o converter

ogg_packer.o: ogg_packer.c ogg_packer.h
	$(CC) ogg_packer.c -c -o ogg_packer.o

opus_header.o: opus_header.c opus_header.h
	$(CC) opus_header.c -c -o opus_header.o

converter.o: converter.cpp
	$(CXX) $(CXXFLAGS) converter.cpp -c -o converter.o

connection_log.o: connection_log.cpp connection_log.h
	$(CXX) $(CXXFLAGS) connection_log.cpp -c -o connection_log.o

rtp.o: rtp.cpp rtp.h
	$(CXX) $(CXXFLAGS) rtp.cpp -c -o rtp.o

clean:
	rm -rf *.o converter
