#include <iostream>
#include <algorithm>

#include "../file_handler/file_handler.h"
#include "backup_set.h"

bool SortByDescFileNumber(file_structs::Partition::File_History& fhistory1, file_structs::Partition::File_History& fhistory2)
{
    return fhistory1.file_number > fhistory2.file_number;
}

/*  Finds and stores the layouts and paths of the prior incremental backup files up to (and including) the latest full backup
    Full backup will be at the start of both vectors 
    This approach uses the file history in the JSON, so this only works for a setup where the file path of the previous backup files has changed */
void FindBackupFiles(PartitionBackupSet& backupSet, file_structs::Partition::Partition_Layout& partitionLayout, int diskIndex)
{
    // Sort the files in descending order so we go from most recent backup to oldest
    // If they are given in order in the JSON, a reverse is sufficient
    std::sort(partitionLayout._header.file_history.begin(), partitionLayout._header.file_history.end(), SortByDescFileNumber);
    
    for (auto& fileHistory : partitionLayout._header.file_history) {
        backupSet.filePaths.insert(backupSet.filePaths.begin(), fileHistory.file_name);
        file_structs::File_Layout fileLayout;
        readBackupFileLayout(fileLayout, fileHistory.file_name);

        for (auto& partition : fileLayout.disks[diskIndex].partitions) {
            if (partition._header.partition_number == partitionLayout._header.partition_number) {
                backupSet.partitionLayouts.insert(backupSet.partitionLayouts.begin(), std::make_unique<file_structs::Partition::Partition_Layout>(partition));
                break;
            }
        }

        if (fileLayout._header.delta_index == 0) { return; } // Stop at the full backup
    }
    
}

void FillInitialBlockFileMap(PartitionBackupSet& backupSet)
{
    BackupFilePtr filePtr = std::make_shared<std::fstream>(openFile(backupSet.filePaths[0]));
    backupSet.backupFilePtrs.push_back(filePtr);
    for (int i = 0; i < backupSet.partitionLayouts[0]->data_block_index.size(); i++) {
        BackupSetBlockIndexElement blockIndexElement;
        blockIndexElement.block = backupSet.partitionLayouts[0]->data_block_index[i];
        blockIndexElement.file = filePtr;
        backupSet.backupSetBlockIndex.push_back(blockIndexElement);
    }
}

void AddDeltaToBlockFileMap(PartitionBackupSet& backupSet)
{
    for (int i = 1; i < backupSet.partitionLayouts.size(); i++) {
        BackupFilePtr filePtr = std::make_shared<std::fstream>(openFile(backupSet.filePaths[i]));
        backupSet.backupFilePtrs.push_back(filePtr);
        for (auto& deltaBlock : backupSet.partitionLayouts[i]->delta_data_block_index) {
            backupSet.backupSetBlockIndex[deltaBlock.block_index].file = filePtr;
            backupSet.backupSetBlockIndex[deltaBlock.block_index].block = deltaBlock.data_block;
        }
    }
}

void CloseBackupFiles(PartitionBackupSet& backupSet)
{
    for(auto& backupFilePtr : backupSet.backupFilePtrs) {
        closeFile(*backupFilePtr);
    }
}

void BuildPartitionBackupSet(PartitionBackupSet& backupSet, file_structs::Partition::Partition_Layout& partitionLayout, int diskIndex)
{
    FindBackupFiles(backupSet, partitionLayout, diskIndex);
    FillInitialBlockFileMap(backupSet);
    AddDeltaToBlockFileMap(backupSet);
}