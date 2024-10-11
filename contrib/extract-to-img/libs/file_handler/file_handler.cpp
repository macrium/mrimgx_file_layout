#include <fstream>
#include <iostream>

void readFile(std::fstream& file, void* buffer, std::streamsize bytesToRead)
{
    file.read(static_cast<char*>(buffer), bytesToRead);
    if (file.fail())
    {
        if (file.eof()) {std::cout << "End of file reached" << std::endl;}
        throw std::runtime_error("Failed to read file.");
    }
}

void setFilePointer(std::fstream& file, std::streamoff offset, std::ios_base::seekdir base)
{
    file.seekg(offset, base);
    if (file.fail())
    {
        std::cout << "Pointer failed to set" << std::endl;
        throw std::runtime_error("Failed to set file pointer.");
    }
}

std::fstream openFile(std::string fileName)
{
    std::fstream file = std::fstream();
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    if (file.fail())
    {
        std::cout << "Failed to open file: " + fileName << std::endl;
        throw std::runtime_error("Failed to open file.");
    }
    return file;
}

void closeFile(std::fstream& file)
{
    file.close();
    if (file.fail())
    {
        std::cout << "Failed to close file" << std::endl;
        throw std::runtime_error("Failed to close file.");
    }
}

void writeToFile(std::fstream& file, void* buffer, std::streamsize bytesToWrite)
{
    file.write(static_cast<char*>(buffer), bytesToWrite);
    if (file.fail())
    {
        if (file.eof()) {std::cout << "End of file reached" << std::endl;}
        std::cout << "Failed to write to file: " << std::endl;
        throw std::runtime_error("Failed to write to file.");
    }
}