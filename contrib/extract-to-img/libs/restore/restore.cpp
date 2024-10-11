#include "backup_set.h"

#include <fstream>
#include <iostream>

std::unique_ptr<unsigned char[]> readDataBlock(std::fstream& backupFile, file_structs::File_Layout backupFileLayout, DataBlockIndexElement& block)
{
    std::unique_ptr<unsigned char[]> readBuffer = std::make_unique<unsigned char[]>(block.block_length);
    setFilePointer(backupFile, block.file_position, std::ios::beg);
    readFile(backupFile, readBuffer.get(), block.block_length);
    return readBuffer;
}

void restoreDisk(std::string backupFilePath, std::string targetDiskPath, file_structs::File_Layout& backupFileLayout, int diskIndex){
    std::fstream diskFile = openFile(targetDiskPath);

    file_structs::Disk::Disk_Layout disk = backupFileLayout.disks[diskIndex];

    writeToFile(diskFile, disk.track0.data(), (uint32_t)disk.track0.size());
  
    for (auto& partition : backupFileLayout.disks[0].partitions)
    {
        PartitionBackupSet backupSet;
        BuildPartitionBackupSet(backupSet, partition, diskIndex);
        std::cout << "Backupset created" << std::endl;

        setFilePointer(diskFile, partition._geometry.start + partition._geometry.boot_sector_offset, std::ios::beg);

        // Restore reserved sectors, for FAT32
        if (partition._file_system.reserved_sectors_byte_length > 0) {
            auto totalBytesToWrite = partition._file_system.reserved_sectors_byte_length;
            uint32_t bytesWritten = 0;
            int index = 0;
            for (auto& reservedSectorBlock : partition.reserved_sectors) {
                auto blockData = readDataBlock(*backupSet.backupFilePtrs.back().get(), backupFileLayout, reservedSectorBlock);
                if (blockData != nullptr) {
                    uint32_t bytesToWrite = std::min(reservedSectorBlock.block_length, totalBytesToWrite - bytesWritten);
                    writeToFile(diskFile, blockData.get(), bytesToWrite);
                }
            }
            std::cout << "Restored reserved sectors" << std::endl;
        }

        int blockIndex = 0;
        auto lcn0Start = partition._geometry.start + (partition._file_system.lcn0_offset - partition._file_system.start);
        for (auto& backupSetBlock : backupSet.backupSetBlockIndex)
        {
            auto blockData = readDataBlock(*(backupSetBlock.file), backupFileLayout, backupSetBlock.block);

            if (blockData != nullptr && backupSetBlock.block.block_length != 0)
            {
                std::streamoff offset = lcn0Start + (static_cast<uint64_t>(partition._header.block_size) * blockIndex);
                setFilePointer(diskFile, offset, std::ios::beg);
                writeToFile(diskFile, blockData.get(), backupSetBlock.block.block_length);
            }
            blockIndex++;
            
        }
        CloseBackupFiles(backupSet);
    }
    std::cout << "Restored all blocks" << std::endl;
    closeFile(diskFile);
}