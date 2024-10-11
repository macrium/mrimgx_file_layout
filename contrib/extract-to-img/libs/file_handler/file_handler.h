#include <fstream>

void readFile(std::fstream& file, void* buffer, std::streamsize bytesToRead);

void setFilePointer(std::fstream& file, std::streamoff offset, std::ios_base::seekdir base);

std::fstream openFile(std::string fileName);

void closeFile(std::fstream& file);

void writeToFile(std::fstream& file, void* buffer, std::streamsize bytesToWrite);