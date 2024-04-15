#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>

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
    const filesystem::path inputPath(fileName);
    const string expectedFileExtension = ".vm";

    vector<string> filesToParse;
    string outputFileName;

    if (filesystem::is_regular_file(inputPath))
    {
        if (inputPath.extension().string() != expectedFileExtension)
        {
            std::cerr << "Expected " << expectedFileExtension << " file." << std::endl;
            exit(3);
        }

        cout << inputPath.stem().string() + ".asm";
        outputFileName = inputPath.stem().string() + ".asm";
        filesToParse.push_back(fileName);
    }
    else if (filesystem::is_directory(inputPath))
    {
        for (const auto& entry : filesystem::directory_iterator(inputPath))
        {
            if (entry.is_regular_file())
            {
                if (entry.path().extension().string() == expectedFileExtension)
                {
                    cout << entry.path().string() << endl;
                    filesToParse.push_back(entry.path().string());
                }
            }
        }

        string directory = "";
        for (auto it : inputPath)
        {
            if (it.string() != "")
            {
                directory = it.string();
            }
        }
        filesystem::path finalPath = inputPath;
        finalPath /= (directory + ".asm");
        outputFileName = finalPath.string();
    }
    else
    {
        std::cerr << "Expected " << expectedFileExtension << " file or directory." << std::endl;
        exit(3);
    }

    if (filesToParse.size() == 0)
    {
        std::cerr << "No files found!" << std::endl;
        exit(3);
    }

    CodeWriter codeWriter(outputFileName);
    for (const string& fileName : filesToParse)
    {
        Parser parser(fileName);
        codeWriter.setFileName(fileName);

        while (parser.hasMoreCommands())
        {
            parser.advance();
            CommandType commandType = parser.commandType();
            string arg1 = parser.arg1();
            int arg2 = parser.arg2();
            if (commandType == C_ARITHMETIC)
            {
                codeWriter.writeArithmetic(arg1);
            }
            else if (commandType == C_PUSH || commandType == C_POP)
            {
                codeWriter.writePushPop(commandType, arg1, arg2);
            }
            else if (commandType == C_LABEL)
            {
                codeWriter.writeLabel(arg1);
            }
            else if (commandType == C_GOTO)
            {
                codeWriter.writeGoto(arg1);
            }
            else if (commandType == C_IF)
            {
                codeWriter.writeIf(arg1);
            }
            else if (commandType == C_FUNCTION)
            {
                codeWriter.writeFunction(arg1, arg2);
            }        
            else if (commandType == C_CALL)
            {
                codeWriter.writeCall(arg1, arg2);
            }
            else if (commandType == C_RETURN)
            {
                codeWriter.writeReturn();
            }
        }
    }
    codeWriter.close();
} 