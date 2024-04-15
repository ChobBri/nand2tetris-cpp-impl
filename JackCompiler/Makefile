all: JackCompiler

JackCompiler: JackCompiler.o compilationEngine.o VMWriter.o jackTokenizer.o symbolTable.o
	g++ -o JackCompiler JackCompiler.o compilationEngine.o VMWriter.o jackTokenizer.o symbolTable.o

VMtranslator.o: JackCompiler.cc
	g++ -c JackCompiler.cc

compilationEngine.o: compilationEngine.cc compilationEngine.h
	g++ -c compilationEngine.cc

VMWriter.o: VMWriter.cc VMWriter.h
	g++ -c VMWriter.cc

jackTokenizer.o: jackTokenizer.cc jackTokenizer.h
	g++ -c jackTokenizer.cc

symbolTable.o: symbolTable.cc symbolTable.h
	g++ -c symbolTable.cc
