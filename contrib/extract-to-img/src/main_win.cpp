#include <iostream>
#include <filesystem>
#include <string>

#include "../libs/img_handler/file_struct.h"
#include "../libs/img_handler/img_handler.h"
#include "../libs/restore/restore.h"

#include "../libs/vhdx_handler/vhdx_handler.h"

std::string wideToString(const std::wstring &wstr)
{
    std::string str(wstr.length(), ' ');
    size_t convertedChars = 0;
    wcstombs(&str[0], wstr.c_str(), str.size());
    return str;
}

void handleWinRestore(std::string backupFileName)
{
    file_structs::File_Layout fileLayout;
    readBackupFileLayout(fileLayout, backupFileName);

    std::filesystem::path curPath = std::filesystem::current_path();
    std::wstring vhdxPath = curPath.wstring() + L"\\test.vhdx";

    CreateVDisk(vhdxPath, fileLayout.disks[0]._geometry.disk_size, fileLayout.disks[0]._geometry.bytes_per_sector);

    std::wstring diskPath;
    MountVDisk(vhdxPath, diskPath);
    restoreDisk(backupFileName, wideToString(diskPath), fileLayout, 0);
    UpdateDiskProperties(diskPath);
}

int main(int argc, char *argv[])
{
    std::cout << argc << std::endl;
    if (argc == 1) {
        std::cout << "No arguments provided" << std::endl;
        return 1;
    }
    std::string backupFileName = argv[1];
    handleWinRestore(backupFileName);
    std::cin.get();
}
