#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>

#include "parser.h"
#include "codeWriter.h"

int main(int argc, char *argv[])
{
    using namespace std;

    if (argc < 2)
    {
        std::cerr << "Provide file as argument." << std::endl;
        exit(1);
    }

    const string fileName = string(argv[1]);
    const filesystem::path directory(fileName);

    const string expectedFileExtension = ".vm";

    if (!equal(fileName.rbegin(), fileName.rbegin() + expectedFileExtension.size(), expectedFileExtension.rbegin()))
    {
        std::cerr << "Expected " << expectedFileExtension << " file." << std::endl;
        exit(3);
    }

    Parser parser(fileName);
    string outputFileName = fileName.substr(0, fileName.find_last_of(".vm") - 2) + ".asm";
    cout << outputFileName << endl;
    CodeWriter codeWriter(outputFileName);
    while (parser.hasMoreCommands())
    {
        parser.advance();
        CommandType commandType = parser.commandType();
        string arg1 = parser.arg1();
        int arg2 = parser.arg2();
        cout << arg1 << endl;
        if (commandType == C_ARITHMETIC)
        {
            codeWriter.writeArithmetic(arg1);
        }
        else
        {
            codeWriter.writePushPop(commandType, arg1, arg2);
        }
    }
    codeWriter.close();
} 