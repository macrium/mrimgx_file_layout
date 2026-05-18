// img_to_raw.cpp
//
// macOS / Linux equivalent of img_to_vhdx.exe.
//
// Macrium Reflect X images can be restored on Windows to a VHDX file via the
// Windows virtdisk.dll APIs (CreateVirtualDisk / AttachVirtualDisk). Those
// APIs are not available outside Windows, and Wine does not implement them.
//
// On macOS/Linux this tool writes the restored disk to a *sparse raw image
// file* instead. The resulting raw image can be inspected with userspace
// forensic tools without needing a kernel-mode mount:
//
//   * `mmls image.raw`                            - show partition table
//   * `fls -o <sector_offset> image.raw`          - list NTFS root
//   * `tsk_recover -a -o <offset> image.raw out/` - extract all files
//
// (all from the Sleuth Kit: `brew install sleuthkit` / `apt install sleuthkit`)
//
// Licensed under the MIT License (same as the rest of this repository).

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <locale>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

#include <nlohmann/json.hpp>
#include "../libs/file_reader/file_reader.h"
#include "../libs/restore/restore.h"

namespace {

std::wstring toWString(const char* s) {
    // We only deal with file-system paths here, so a byte-by-byte cast is
    // sufficient (POSIX `mbstowcs` would need a configured C locale and
    // returns (size_t)-1 for any non-ASCII byte under the default "C" locale).
    std::wstring w;
    if (!s) return w;
    for (const char* p = s; *p; ++p) {
        w.push_back(static_cast<wchar_t>(static_cast<unsigned char>(*p)));
    }
    return w;
}

void printUsage(const char* prog) {
    std::cerr <<
        "Usage: " << prog << " <input.mrimgx> [<output.raw>] [-p password] [-d disk_number] [-desc]\n"
        "\n"
        "  <input.mrimgx>  Macrium Reflect X image or file-and-folder backup\n"
        "  <output.raw>    destination raw disk image (sparse file, only allocated\n"
        "                  blocks consume physical space)\n"
        "\n"
        "  -p password     password for an encrypted backup\n"
        "  -d disk_number  disk to restore (default: first disk in the backup)\n"
        "  -desc           print backup metadata and exit (no output written)\n"
        "  -h, --help      this message\n"
        "\n"
        "After restore, inspect the raw image with Sleuth Kit:\n"
        "  mmls <output.raw>\n"
        "  tsk_recover -a -o <ntfs_start_sector> <output.raw> out_dir/\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::setlocale(LC_ALL, "");

    if (argc < 2) { printUsage(argv[0]); return 1; }

    std::string inputArg = argv[1];
    std::string outputArg;
    std::string password;
    int  diskNumber  = -1;
    bool describeOnly = false;

    int argIdx = 2;
    if (argIdx < argc && argv[argIdx][0] != '-') {
        outputArg = argv[argIdx++];
    }
    for (; argIdx < argc; ++argIdx) {
        std::string a = argv[argIdx];
        if (a == "-p" && argIdx + 1 < argc) password   = argv[++argIdx];
        else if (a == "-d" && argIdx + 1 < argc) diskNumber = std::atoi(argv[++argIdx]);
        else if (a == "-desc" || a == "--desc" || a == "-describe") describeOnly = true;
        else if (a == "-h" || a == "--help") { printUsage(argv[0]); return 0; }
        else { std::cerr << "Unknown arg: " << a << "\n"; return 1; }
    }

    if (!describeOnly && outputArg.empty()) {
        std::cerr << "Output path required (or use -desc for describe-only).\n";
        return 1;
    }

    std::wstring inputPath = toWString(inputArg.c_str());

    try {
        std::cout << "Reading backup metadata: " << inputArg << std::endl;
        file_structs::fileLayout backupFile;
        readBackupFile(inputPath, backupFile, password);

        if (describeOnly) {
            std::cout << "\n=== Backup Info ===\n";
            std::cout << "Disks: " << backupFile.disks.size() << "\n";
            for (auto& d : backupFile.disks) {
                std::cout << "  Disk " << d._header.disk_number
                          << ":  size=" << d._geometry.disk_size
                          << " (" << std::fixed << std::setprecision(2)
                          << (d._geometry.disk_size / 1024.0 / 1024.0 / 1024.0) << " GB), "
                          << "sector=" << d._geometry.bytes_per_sector << "\n"
                          << "    Partitions: " << d.partitions.size() << "\n";
            }
            return 0;
        }

        file_structs::Disk::DiskLayout diskToRestore;
        getDiskToRestoreFromDiskNumber(backupFile, diskNumber, diskToRestore);

        uint64_t diskSize   = diskToRestore._geometry.disk_size;
        uint32_t sectorSize = diskToRestore._geometry.bytes_per_sector;
        if (sectorSize == 0) sectorSize = 512;
        if (diskSize % sectorSize != 0) {
            diskSize = ((diskSize / sectorSize) + 1) * sectorSize;
        }

        std::cout << "Disk size:    " << diskSize << " bytes ("
                  << std::fixed << std::setprecision(2)
                  << (diskSize / 1024.0 / 1024.0 / 1024.0) << " GB)\n";
        std::cout << "Sector size:  " << sectorSize << " bytes\n";
        std::cout << "Output:       " << outputArg << "\n";

        // Pre-create the sparse raw file at the full disk size. The restore
        // code below opens this file via std::fstream(in|out|binary), which
        // requires the file to already exist.
        int fd = open(outputArg.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (fd < 0) {
            throw std::runtime_error(std::string("Cannot create output: ") + std::strerror(errno));
        }
        if (ftruncate(fd, static_cast<off_t>(diskSize)) < 0) {
            int e = errno;
            close(fd);
            throw std::runtime_error(std::string("ftruncate failed: ") + std::strerror(e));
        }
        close(fd);
        std::cout << "Created sparse raw file (" << (diskSize / 1024 / 1024) << " MB logical size).\n";

        auto start = std::chrono::steady_clock::now();
        ProgressCallback cb = [&start](uint64_t total, uint64_t& processed,
                                      std::chrono::steady_clock::time_point& lastUpdate) {
            auto now = std::chrono::steady_clock::now();
            auto since = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();
            if (since >= 1000 || processed == total) {
                double pct  = total > 0 ? double(processed) / double(total) * 100.0 : 0.0;
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
                double mbps  = elapsed > 0 ? (processed / 1024.0 / 1024.0) / (elapsed / 1000.0) : 0.0;
                std::cout << "\r  " << std::fixed << std::setprecision(1) << pct << "%  "
                          << (processed / 1024 / 1024) << " / " << (total / 1024 / 1024) << " MB  "
                          << std::setprecision(1) << mbps << " MB/s         " << std::flush;
                lastUpdate = now;
            }
        };

        std::cout << "Restoring blocks..." << std::endl;
        std::wstring outputPath = toWString(outputArg.c_str());
        // keepDiskId=true skips the GPT disk-GUID rewrite (a no-op when we are
        // not booting the disk; matters only if you intend to attach this to a
        // running system alongside the original).
        restoreDisk(inputPath, password, outputPath, diskNumber, /*keepDiskId*/ true, cb);

        std::cout << "\nDone. Raw image written to: " << outputArg << "\n"
                  << "Inspect with Sleuth Kit:\n"
                  << "  mmls \"" << outputArg << "\"\n"
                  << "  fls -o <ntfs_sector_offset> \"" << outputArg << "\"\n"
                  << "  tsk_recover -a -o <ntfs_sector_offset> \"" << outputArg << "\" outdir/\n";
    }
    catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
