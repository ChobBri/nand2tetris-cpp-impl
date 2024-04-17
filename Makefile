all: Tetris

clean:
	rm JackCompiler/*.o
	rm JackCompiler/*.exe
	rm VMtranslator/*.o
	rm VMtranslator/*.exe
	rm HackAssembler/*.o
	rm HackAssembler/*.exe

	rm JackOS/*.vm
	rm Tetris/*.vm
	rm Tetris/*.asm

	rm *.hack

Tetris: JackCompiler.exe VMtranslator.exe HackAssembler.exe
	./JackCompiler/JackCompiler.exe Tetris
	./JackCompiler/JackCompiler.exe JackOS
	./VMtranslator/VMtranslator.exe Tetris
	./HackAssembler/HackAssembler.exe Tetris/Tetris.asm

JackCompiler.exe:
	$(MAKE) -C JackCompiler/

VMtranslator.exe:
	$(MAKE) -C VMtranslator/

HackAssembler.exe:
	$(MAKE) -C HackAssembler/


