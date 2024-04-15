all: Tetris

Tetris: JackCompiler.exe VMtranslator.exe hackAssembler.exe
	./JackCompiler/JackCompiler.exe Tetris
	./VMtranslator/VMtranslator.exe Tetris
	./hackAssembler/hackAssembler.exe Tetris/Tetris.asm

JackCompiler.exe:
	mingw32-make.exe -C JackCompiler/

VMtranslator.exe:
	mingw32-make.exe -C VMtranslator/

hackAssembler.exe:
	mingw32-make.exe -C hackAssembler/
