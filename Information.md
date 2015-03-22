# Installation #

Linux:
  * untar the _tchip16`_`X.x.x.tar.gz_ archive
  * in the resulting folder, run `make clean all`
  * run `sudo make install`
  * to uninstall, run `sudo make uninstall`

Windows:
  * extract the _tchip16`_`X.x.x.zip_ archive
  * consider adding the resulting directory to your PATH

# Usage #

On linux, enter `man tchip16` or `tchip16 --help` or consult the README.
On windows, enter `tchip16.exe --help` or consult the README.

## tchip16 ##

Linux: `./tchip16 <source> [-o dest] [-m|--mmap] [-a|--align] [-z|--zero] [-r|--raw] [-h|--help] [--version]`

Windows: `tchip16.exe <source> [-o dest] [-m|--mmap] [-a|--align] [-z|--zero] [-r|--raw] [-h|--help] [--version]`

(< >:required, [ ]:optional)

  * compiles file(s) _source_
  * output file is _dest_ if _**-o**_ is specified, _output.c16_ otherwise
  * if _**-mmap**_ is specified, outputs _mmap.txt_ which lists the addresses of all the labels in the code. This is useful for hacking the ouput file, like _**-z**_
  * if _**--align**_ is specified, aligns labels and pads db's to 4-byte boundaries. This can be useful for optimized word-addressing, and hacking, but breaks text-displaying in current ROMs
  * if _**--zero**_ is specified, fill the output file with 0's after code until file size reache 64KB (65,536B). This is useful if you plan to hack the output file after compilation or modify its contents manually, and should be used with _**-m**_
  * if _**--raw**_ is specified, a headerless ROM is output.
  * if _**--help**_ is specified, the help text is displayed.
  * if _**--version**_ is specified, the version will be displayed.

## img\_conv ##

Windows: `img_conv.exe <source> <dest> [col_key]`

  * reads _source_ and outputs corresponding _dest_
  * if _col`_`key_ is specified, all pixels of color _col`_`key_ will be transparent

**Remember**: _source_ must be a 24-bit BMP file.

# FAQ #

## What is tchip16? What should I use it for? ##

_tchip16_ is an assembler for the Chip16 VM, which is detailed in the [thread](http://forums.ngemu.com/showthread.php?t=145620) on Emuforums.
You should use it to compile your code into a Chip16 binary file.

## Is tchip16 available for my system? ##

_tchip16_ is cross-platform, in the sense that it does not include platform-specific code, and should build without problems on Linux, Mac OS X, and Windows.
That said, no binaries are available for Max OS X for now -- you will have to compile from source if you use it.