all: hackAssembler

hackAssembler: hackAssembler.o parser.o code.o symbolTable.o
	g++ -o hackAssembler hackAssembler.o parser.o code.o symbolTable.o

parser.o: parser.cc parser.h
	g++ -c parser.cc

code.o: code.cc code.h
	g++ -c code.cc

symbolTable.o: symbolTable.cc symbolTable.h
	g++ -c symbolTable.cc

hackAssembler.o: hackAssembler.cc
	g++ -c hackAssembler.cc
