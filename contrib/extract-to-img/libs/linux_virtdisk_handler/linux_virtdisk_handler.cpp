#include "linux_virtdisk_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

void RunCommandWithOutput(std::string command, std::string &output)
{
    FILE *fp = popen(command.c_str(), "r");
    char c = fgetc(fp);
    do
    {
        output += c;
        c = fgetc(fp);
    } while (c != '\n');
    pclose(fp);
}

void CreateIMG(std::string imgPath, unsigned long long size, unsigned long sectorSize)
{
    uint32_t count = size / sectorSize;
    system(("sudo dd if=/dev/zero of=" + imgPath + " bs=" + std::to_string(sectorSize) + " count=" + std::to_string(count)).c_str());
    system(("sudo chmod 777 " + imgPath).c_str());
}

void MountIMG(std::string imgPath, std::string &loopFilePath)
{
    RunCommandWithOutput("sudo losetup -f", loopFilePath);
    system(("sudo losetup -P " + loopFilePath + " " + imgPath).c_str());
}

void UnmountIMG(std::string loopFilePath)
{
    system(("sudo losetup -d" + loopFilePath).c_str());
}