#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <sstream>

#include "jackTokenizer.h"
#include "compilationEngine.h"

using namespace std;

void writeToXml(ofstream& outputStream, const string& tokenType, const string& token);
int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        std::cerr << "Provide file as argument." << std::endl;
        exit(1);
    }

    const string fileName = string(argv[1]);
    const filesystem::path inputPath(fileName);
    const string expectedFileExtension = ".jack";

    vector<string> filesToParse;
    string outputFileName;

    if (filesystem::is_regular_file(inputPath))
    {
        if (inputPath.extension().string() != expectedFileExtension)
        {
            std::cerr << "Expected " << expectedFileExtension << " file." << std::endl;
            exit(3);
        }

        cout << inputPath.stem().string() + ".xml" << endl;
        outputFileName = inputPath.stem().string() + ".xml";
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
        finalPath /= (directory + ".xml");
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

    for (const string& fileName : filesToParse)
    {
        filesystem::path filePath(fileName);
        filesystem::path newPath = filePath.parent_path();
        newPath /= filePath.stem();
        string outputFileName = newPath.string() + ".xml";
        CompilationEngine compilationEngine(fileName, outputFileName);
        compilationEngine.compile();
    }
}

void writeToXml(ofstream& outputStream, const string& tokenType, const string& token)
{
    stringstream strStream;
    strStream << "<" << tokenType << "> " << token << " </" << tokenType << ">\n";
    outputStream << strStream.str();
}