#include <iostream>
#include <string>
#include <algorithm>
#include "parser.h"
#include "code.h"

int main(int argc, char *argv[])
{
    using namespace std;

    if (argc < 2)
    {
        std::cerr << "Provide file as argument." << std::endl;
        exit(1);
    }

    string fileName = string(argv[1]);
    string expectedFileExtension = ".asm";

    if (!equal(fileName.rbegin(), fileName.rbegin() + expectedFileExtension.size(), expectedFileExtension.rbegin()))
    {
        std::cerr << "Expected .asm file." << std::endl;
        exit(3);
    }

    Parser parser(fileName);
    Code code;

    ofstream outfile;
    outfile.open("Prog.hack", std::ios::trunc | std::ios::binary);
    while (parser.hasMoreCommands())
    {
        parser.advance();
        Command_Type commandType = parser.commandType();

        uint16_t linebits = 0x0;
        if (commandType == A_COMMAND)
        {
            string symbol = parser.symbol();
            linebits = 0xFFFF;
            uint16_t num = stoi(symbol);
            linebits &= num;
        }
        else if (commandType == C_COMMAND)
        {
            string dest = parser.dest();
            string comp = parser.comp();
            string jump = parser.jump();

            uint16_t destbit = code.dest(dest);
            uint16_t compbit = code.comp(comp);
            uint16_t jumpbit = code.jump(jump);

            linebits |= 0b111 << 13;
            linebits |= compbit << 6;
            linebits |= destbit << 3;
            linebits |= jumpbit;
        }
        else if (commandType == L_COMMAND)
        {
            // ngl, no idea what the L_COMMAND is for.
            continue;
        }
        char bytes[2];

        string linebitstr = "";

        for (int i = 15; i >= 0; i--)
        {
            linebitstr += ((linebits >> i) & 0x1) ? "1" : "0";
        }
        outfile << linebitstr << endl;
    }

    outfile.close();
} 