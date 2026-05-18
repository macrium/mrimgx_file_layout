<img src="../assets/ReflectX.png" width="300"> <br>Experience data independence with love, from us to you. <img src="../assets/Love_Heart_symbol.svg" width="30">

***

## `img_to_raw` — Read and restore data from Macrium Reflect X files on macOS / Linux.

`img_to_vhdx` writes a VHDX file using the Windows `virtdisk.dll`
(`CreateVirtualDisk` / `AttachVirtualDisk`) APIs. Those APIs only exist on
Windows and are not implemented by Wine, so the tool can't run on macOS or
Linux even under translation layers.

`img_to_raw` is the POSIX equivalent: it walks the same backup-file structure
and writes the restored disk contents to a **sparse raw disk-image file**
instead of a VHDX. Only allocated blocks consume physical space on disk; the
file's logical size matches the size of the source disk.

The raw image can then be inspected with userspace forensic tooling, no
kernel-mode mount required.

---

| Requirement  | Specification |
| :---   | :---   |
| Operating System | macOS 13+, Linux |
| Architecture  | x86_64, arm64  |
| Image File(s)   | Macrium Reflect vX (`.mrimgx`, `.mrbakx`)  |

---

## Building

### macOS

```sh
brew install cmake openssl@3 zstd nlohmann-json
cd src
cmake -B build -S .
cmake --build build -j
# binary: build/img_to_raw/img_to_raw
```

### Linux (Debian/Ubuntu)

```sh
sudo apt install cmake libssl-dev libzstd-dev nlohmann-json3-dev uuid-dev
cd src
cmake -B build -S .
cmake --build build -j
# binary: build/img_to_raw/img_to_raw
```

---

## Usage

```
img_to_raw <input.mrimgx> [<output.raw>] [-p password] [-d disk_number] [-desc]

  <input.mrimgx>  Macrium Reflect X image (or file-and-folder backup)
  <output.raw>    destination raw disk image (sparse; required unless -desc)

  -p password     password for an encrypted backup
  -d disk_number  disk to restore (default: first disk in the backup)
  -desc           print backup metadata and exit (no output written)
  -h, --help      this message
```

### Example: inspect a backup

```sh
img_to_raw mybackup.mrimgx -desc
```

```
Reading backup metadata: mybackup.mrimgx

=== Backup Info ===
Disks: 1
  Disk 0:  size=500107862016 (465.76 GB), sector=512
    Partitions: 4
```

### Example: full restore

```sh
img_to_raw mybackup.mrimgx mybackup.raw
```

```
Disk size:    500107862016 bytes (465.76 GB)
Sector size:  512 bytes
Output:       mybackup.raw
Created sparse raw file (476940 MB logical size).
Restoring blocks...
  100.0%  61658 / 61658 MB  111.8 MB/s
Done. Raw image written to: mybackup.raw
Inspect with Sleuth Kit:
  mmls "mybackup.raw"
  fls -o <ntfs_sector_offset> "mybackup.raw"
  tsk_recover -a -o <ntfs_sector_offset> "mybackup.raw" outdir/
```

---

## Inspecting the raw image

The Sleuth Kit (`brew install sleuthkit` / `apt install sleuthkit`) provides
userspace tools that read NTFS, FAT, ext4, and many other filesystems
directly from a raw image — no kernel mount, no macFUSE required.

### List partitions

```sh
mmls mybackup.raw
```

```
DOS Partition Table
      Slot      Start        End          Length       Description
000:  Meta      0000000000   0000000000   0000000001   Primary Table (#0)
001:  -------   0000000000   0000002047   0000002048   Unallocated
002:  000:000   0000002048   0000718847   0000716800   NTFS / exFAT (0x07)
003:  000:001   0000718848   0205801471   0205082624   NTFS / exFAT (0x07)
```

### List the NTFS root of a partition

`-o` is the partition's starting sector (column "Start" above).

```sh
fls -o 718848 mybackup.raw
```

### Extract all allocated files from an NTFS partition

```sh
tsk_recover -a -o 718848 mybackup.raw outdir/
```

`-a` extracts allocated files only. Omit it to also recover files that were
in the MFT but marked deleted at backup time.

### Read a single file

```sh
icat -o 718848 mybackup.raw <inode-id> > one_file.txt
```

---

## Notes

- The raw image is a **sparse file**. `ls -lh` shows the logical size (full
  disk size), `du -h` shows the actual on-disk usage (only allocated
  clusters). On APFS / ext4 this works transparently.
- `img_to_raw` always writes the entire selected disk, including any partition
  in the backup that was empty at backup time. Those areas remain holes in
  the sparse file.
- Encrypted backups: pass `-p <password>`. Same crypto code path as
  `img_to_vhdx`.
- Performance on M-series Mac with USB-C SSD: ~100-150 MB/s sustained
  during decompression + write.
