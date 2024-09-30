<img src="../assets/ReflectX.png" width="300"> <br>Experience data independence with love, from us to you. <img src="../assets/Love_Heart_symbol.svg" width="30">

***
**This package includes the following components:**

1. `2394E9AA621DDC3A-00-00.mrimgx`: A Macrium Reflect Image file 
    containing a backup of a partition.
2. `image_to_vhdx.exe`: A utility that converts the .mrimgx file into a VHDX 
    (Windows Virtual Hard Disk) file.
3. `demo.bat`: A batch script that automates the conversion of the .mrimgx file 
    into a VHDX file.

When you run `demo.bat`, it invokes `image_to_vhdx.exe` with the .mrimgx file 
as a parameter. The utility then converts the .mrimgx file into a VHDX file 
in the same directory.

Here's how to use it:

1. Extract [demo.zip](demo.zip) to a directory of your choice.
2. Open a command prompt and navigate to the directory where you extracted 
   the files.
3. Run the `demo.bat` script by typing "demo.bat" and pressing Enter, or by 
   double-clicking `demo.bat` in Windows File Explorer.

Once the script completes, you'll find a new VHDX file in the same directory. 
This VHDX file will be mounted and accessible until you press any key. 

**While the VHDX file is mounted, you can access the restored file system in Windows File Explorer, 
or assign a drive letter using the Windows Disk Management Console.**

> [!NOTE]
> For detailed information on all command line parameters, and  `img_to_vhdx.exe` usage, see [here](../src/IMG_TO_VHDX.md)
