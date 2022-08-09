all: rdiff tests

rdiff: Main.o BlockSignature.o Signature.o Delta.o Patch.o WriterDelta.o
	g++ -lm Main.o BlockSignature.o Signature.o Delta.o Patch.o WriterDelta.o -o rdiff

tests: Tests.o BlockSignature.o Signature.o Delta.o Patch.o WriterDelta.o
	g++ -lm Tests.o BlockSignature.o Signature.o Delta.o Patch.o WriterDelta.o -o tests

Main.o: Main.cpp Mdfour.h Rabinkarp.h Rollsum.h Weaksum.h Strongsum.h BlockSignature.h Signature.h Delta.h Patch.h FileIO.h WriterDelta.h
	g++ -Wall -c Main.cpp -o Main.o

Tests.o: Tests.cpp Mdfour.h Rabinkarp.h Rollsum.h Weaksum.h Strongsum.h BlockSignature.h Signature.h Delta.h Patch.h FileIO.h WriterDelta.h
	g++ -Wall -c Tests.cpp -o Tests.o

BlockSignature.o: BlockSignature.cpp BlockSignature.h
	g++ -Wall -c BlockSignature.cpp -o BlockSignature.o

Signature.o: Signature.cpp Signature.h
	g++ -Wall -c Signature.cpp -o Signature.o

Delta.o: Delta.cpp Delta.h
	g++ -Wall -c Delta.cpp -o Delta.o

Patch.o: Patch.cpp Patch.h
	g++ -Wall -c Patch.cpp -o Patch.o

WriterDelta.o: WriterDelta.cpp WriterDelta.h
	g++ -Wall -c WriterDelta.cpp -o WriterDelta.o

clean:
	rm -f *.o tests rdiff

