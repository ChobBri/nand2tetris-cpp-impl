all: VMtranslator

VMtranslator: VMtranslator.o parser.o codeWriter.o
	g++ -o VMtranslator VMtranslator.o parser.o codeWriter.o

parser.o: parser.cc parser.h
	g++ -c parser.cc

codeWriter.o: codeWriter.cc codeWriter.h
	g++ -c codeWriter.cc

VMtranslator.o: VMtranslator.cc
	g++ -c VMtranslator.cc
