#include <map>
#include <vector>
#include <fstream>
#include <string>

#include "../img_handler/img_handler.h"

// Maps block index to a backup file 
typedef std::shared_ptr<std::fstream> BackupFilePtr;
typedef std::unique_ptr<file_structs::Partition::Partition_Layout> PartitionLayoutPtr;
typedef uint32_t BlockIndex;

struct BackupSetBlockIndexElement
{
    DataBlockIndexElement block;
    BackupFilePtr file;
};

struct PartitionBackupSet
{
    std::vector<std::string> filePaths;
    std::vector<PartitionLayoutPtr> partitionLayouts;
    std::vector<BackupSetBlockIndexElement> backupSetBlockIndex;
    std::vector<BackupFilePtr> backupFilePtrs;
};

void BuildPartitionBackupSet(PartitionBackupSet& backupSet, file_structs::Partition::Partition_Layout& partitionLayout, int diskIndex);

void CloseBackupFiles(PartitionBackupSet& backupSet);