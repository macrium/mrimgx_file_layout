<img src="../assets/ReflectX.png" width="300"> <br>Experience data independence with love, from us to you. <img src="../assets/Love_Heart_symbol.svg" width="30">


***

## `img_to_vhdx.exe` - Read and Restore data from Macrium Reflect X files.
> [!NOTE]
> For a demonstration featuring an included image file and automated execution, please visit this [link](https://github.com/macriumsoftware/mrimg_file_layout/tree/main/demo)
***
| Requirement  | Specification |
| :---   | :---   |
| Minimum Operating System   | Windows 10  |
| Architecture  | x64  |
| Image File(s)   | Macrium Reflect vX  |
***
This Visual Studio 2022 solution contains a Windows 64 bit executable `img_to_vhdx.exe`. This program restores a given Macrium Reflect X file to a dynamically created, and mounted, VHDX file. By default, the VHDX file is created in the same folder as the .mrimgx or .mrbakx file, using the same name as the backup file with the extension changed to .vhdx. 
> [!TIP]  
> Using the `[-o output_path]` parameter, the VHDX file can be created at a different location.  <br>
> **Example:**  `img_to_vhdx.exe C:\D684BA87241263E2-demo-00-00.mrimgx -o "\\nas\my backups\demo"`  
***
## Restoring a backup file

Use the fully qualified path of a Macrium Reflect X image file ( .mrimgx ) or a File and Folder Backup file ( .mrbakx ) as the first parameter. 

**All backup types are supported:**
- Compressed
- Encrypted
- Full
- Differential 
- Incremental
- Incremental Delta
- Split Files
- Consolidated Files

> [!NOTE]  
> For detailed information on how Macrium Reflect handles encryption. Please see [here](../docs/ENCRYPTION.md)

**Example restore of a Full image:**

```console
C:\>img_to_vhdx.exe C:\D684BA87241263E2-demo-00-00.mrimgx

=================================================================
  DISCLAIMER:
  This software is for demonstration purposes only.
  It shows how to access data in a Macrium Reflect X backup file.
  This software comes with no warranty, expressed or implied.
  Use this software at your own risk.
=================================================================

Restoring:      C:\D684BA87241263E2-demo-00-00.mrimgx
To:             C:\D684BA87241263E2-demo-00-00.vhdx

Progress: [##################################################] 100% 93.00 MB / 93.00 MB at 74.40 Mb/s

Restore successful.
The restored file system(s) can be viewed in Windows File Explorer.
If not visible, please check the Windows Disk Management Console
and assign drive letter(s) if necessary.

Press any key to dismount the VHDX and exit the program . . .
```
***
**Parameter:** `[-h help]`  <br><br>
All parameters are printed using the -h parameter: 

```console
Usage: filename [-p password] [-d disk] [-k keep_id] [-o output_path] [-desc describe] [-j json] [-h help]
filename: The name of the file to process.
-p password:    The password for the backup file (optional).
-d disk:        The disk number to restore (defaults to first disk if not supplied).
-k keep_id:     Do not update the disk ID when mounting.
-o output_path: The path for the VHDX (optional, defaults to backup file location).
-desc describe: Outputs the backup file's structure and content.
-j json:        Outputs the backup file's json metadata.
-h help:        Display this help message.

Examples:
        img_to_vhdx.exe c:\backup.mrimgx -p mypassword -d 1 -o C:\output
        img_to_vhdx.exe c:\backup.mrimgx -p mypassword -d 1
        img_to_vhdx.exe c:\backup.mrimgx -p mypassword
        img_to_vhdx.exe c:\backup.mrimgx
        img_to_vhdx.exe c:\backup.mrimgx describe
        img_to_vhdx.exe c:\backup.mrimgx json
```
***
**Parameter:** `[-k keep_id]`  <br><br>
By default, when img_to_vhdx.exe runs, the output vhdx file is created with an updated disk ID to prevent disk ID conflicts when mounting. Specifying the -k parameter will create a vhdx file with an unchanged disk ID, enabling a vhdx file of system disk to be bootable in a virtual machine.<br><br>
Specifying the -k parameter will create a vhdx file with an unchanged disk ID that can be booted in a virtual machine.
***
**Parameter:** `[-desc describe]`  <br><br>
`img_to_vhdx.exe {file name} -desc` - will print detailed information about the backup:

```console
C:\>img_to_vhdx.exe C:\D684BA87241263E2-demo-00-00.mrimgx describe
=================================================================
  DISCLAIMER:
  This software is for demonstration purposes only.
  It shows how to access data in a Macrium Reflect X backup file.
  This software comes with no warranty, expressed or implied.
  Use this software at your own risk.
=================================================================

Macrium Reflect Backup File Structure
________________________________________________________________________________________________________________________

Json:           v1
Macrium:        v10.7919
________________________________________________________________________________________________________________________

File Name:      B:\vx\D684BA87241263E2-demo-00-00.mrimgx
Image ID:       D684BA87241263E2
Backup GUID:    f4487e0d-cdb5-483d-9757-12c514ace5a0
Backup Time:    2024-04-14 15:18:45
Netbios:        MS-W-0
Compression:    medium
Encryption:     not set
Comment:
________________________________________________________________________________________________________________________

Disk 4:
ID:     5D4001D3-7B31-4300-BED8-48C82AF9DE24
Format: gpt
Size:   120.00 GB

        Partition 1:
        Letter: H
        Size:   119.98 GB
        Volume GUID:    \??\Volume{28e8f8e1-94e4-49dd-a54b-4babc99677dd}
        Label:  Demo
        Format: NTFS
________________________________________________________________________________________________________________________
```
> [!NOTE]
> To restore a specific disk from multi-disk image files, use the `[-d disk]` parameter. Specify the disk number as shown in the output, for instance, "Disk 4:" in this example `[-d 4]`.

***
**Parameter:** `[-j json]`  <br><br>
`img_to_vhdx.exe {file name} -j`  - will print the json metadata in the file.

```console
C:\>img_to_vhdx.exe C:\D684BA87241263E2-demo-00-00.mrimgx json
{
    "_auxiliary_data": {
        "backup_definition": {
            "auto_verify": false,
            "backup_definition_file": "",
            "backup_format": "partition",
            "check_filesystem": false,
            "comment": "",
            "compression_level": "medium",
            "consolidation_type": "none",
            "cpu_priority": "below_normal",
            "disk_space_management": {
                "apply_to_all_sets_in_folder": false,
                "differential_retention": {
                    "enable": true,
                    "interval": 4,
                    "period": "count"
                },
                "full_retention": {
                    "enable": true,
                    "interval": 13,
                    "period": "count"
                },
                "incremental_retention": {
                    "enable": true,
                    "interval": 10,
                    "period": "count"
                },
                "incrementals_forever": false,
                "low_space_gb_threshold": 5,
                "low_space_purge": true,
                "run_before": true
            },
            "disks": [
                {
                    "number": 4,
                    "partitions": [
                        1
                    ],
                    "signature": "5D4001D3-7B31-4300-BED8-48C82AF9DE24"
                }
            ],
            "email": {
                "failure_attach_log": false,
                "failure_attach_vss_log": false,
                "failure_content": "",
                "failure_enable": false,
                "failure_include_cancelled": false,
                "failure_include_skipped": true,
                "failure_recipients": "",
                "failure_subject": "",
                "success_attach_log": false,
                "success_attach_vss_log": false,
                "success_content": "",
                "success_enable": false,
                "success_recipients": "",
                "success_subject": "",
                "warning_attach_log": false,
                "warning_attach_vss_log": false,
                "warning_content": "",
                "warning_enable": false,
                "warning_recipients": "",
                "warning_subject": ""
            },
            "file_type": "image",
            "filename": "B:\\vx\\D684BA87241263E2-demo-00-00.mrimgx",
            "filename_template": "demo",
            "intelligent_sector_copy": true,
            "limit_file_size": false,
            "post_backup_shutdown": {
                "enable": false
            },
            "prefix_unique": true,
            "rate_limit": 20000
        },
        "bootable": false,
        "destination": "B:\\vx\\D684BA87241263E2-demo-00-00.mrimgx",
        "highest_file_offset": 0,
        "macrium_reflect": {
            "build": 7919,
            "file_version": 10,
            "version": 8
        },
        "target_drive_type": "local",
        "unique_prefix_type": "imageid"
    },
    "_compression": {
        "compression_level": "medium",
        "compression_method": "zstd"
    },
    "_encryption": {
        "enable": false,
        "key_iterations": 0
    },
    "_header": {
        "backup_format": "partition",
        "backup_guid": "f4487e0d-cdb5-483d-9757-12c514ace5a0",
        "backup_time": 1713104325,
        "backup_type": "full",
        "backupset_time": 1713104325,
        "delta_index": false,
        "file_number": 0,
        "imaged_disks_count": 1,
        "imageid": "D684BA87241263E2",
        "increment_number": 0,
        "index_file_position": 1294336,
        "json_version": 1,
        "netbios_name": "MS-W-0",
        "split_file": false
    },
    "disks": [
        {
            "_descriptor": {
                "disk_description": "Msft     Virtual Disk     1.0",
                "disk_manufacturer": "Msft    ",
                "disk_productid": "Virtual Disk    ",
                "disk_revisonno": "1.0 ",
                "disk_serialno": ""
            },
            "_geometry": {
                "bytes_per_sector": 512,
                "cylinders": 15665,
                "disk_size": 128849018880,
                "media_type": "fixed_media",
                "sectors_per_track": 63,
                "tracks_per_cylinder": 255
            },
            "_header": {
                "disk_format": "gpt",
                "disk_number": 4,
                "disk_signature": "5D4001D3-7B31-4300-BED8-48C82AF9DE24",
                "imaged_partition_count": 1
            },
            "partitions": [
                {
                    "_file_system": {
                        "bitlocker_state": "none",
                        "drive_letter": 72,
                        "end": 128846921727,
                        "free_clusters": 31428909,
                        "lcn0_file_number": 0,
                        "lcn0_offset": 16777216,
                        "mft_offset": 3238002688,
                        "mft_record_size": 1024,
                        "partition_index": 1,
                        "sectors_per_cluster": 8,
                        "shadow_copy": "\\\\?\\GLOBALROOT\\Device\\HarddiskVolumeShadowCopy5",
                        "start": 16777216,
                        "total_clusters": 31452672,
                        "type": "NTFS",
                        "volume_guid": "\\??\\Volume{28e8f8e1-94e4-49dd-a54b-4babc99677dd}",
                        "volume_label": "Demo"
                    },
                    "_geometry": {
                        "boot_sector_offset": 0,
                        "end": 128846921727,
                        "length": 128830144512,
                        "start": 16777216
                    },
                    "_header": {
                        "block_count": 1965792,
                        "block_size": 65536,
                        "file_history": [
                            {
                                "file_name": "B:\\vx\\D684BA87241263E2-demo-00-00.mrimgx",
                                "file_number": 0
                            }
                        ],
                        "file_history_count": 1,
                        "partition_file_offset": 0,
                        "partition_number": 1
                    },
                    "_partition_table_entry": {
                        "active": false,
                        "boot_sector": 32768,
                        "end_cylinder": 0,
                        "end_head": 0,
                        "num_sectors": 251621376,
                        "partition_type": "primary",
                        "start_cylinder": 0,
                        "start_head": 0,
                        "status": 0,
                        "type": 7
                    }
                }
            ]
        }
    ]
}
```
