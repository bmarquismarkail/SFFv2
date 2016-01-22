#include "SFF2File.h"
#include <fstream>


int main()
{
    std::ifstream *sff_file = new std::ifstream("test.sff",std::ios::binary);
    SFF2File SFFTest(sff_file);
    sff_file->close();
    delete sff_file;
    std::ofstream *output = new std::ofstream("test.bin", std::ios::binary);
    int x = 0;
    output->write((char*)SFFTest.GetSprData(x)->GetData(), SFFTest.GetSprData(x)->GetLength());
    output->close();
    delete output;
    return 0;
}
