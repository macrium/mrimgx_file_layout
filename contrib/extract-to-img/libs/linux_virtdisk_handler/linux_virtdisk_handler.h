#include <string>

void CreateIMG(std::string imgPath, unsigned long long size, unsigned long sectorSize);

void MountIMG(std::string imgPath, std::string &loopFilePath);

void UnmountIMG(std::string loopFilePath);