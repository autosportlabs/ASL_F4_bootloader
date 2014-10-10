ASL_F4_bootloader
=================

USB bootloader for STM32F407

Requirements
------------
* Python2.7
* crcmod (via pip)
* [GCC Arm Embedded 4.7](https://launchpad.net/gcc-arm-embedded)
* [XBVC](https://github.com/Jeff-Ciesielski/XBVC)
* [ihexpy](https://github.com/Jeff-Ciesielski/ihexpy)

Recommended
-----------
* OpenOCD


Building
--------
Simply type 'make'

This will generate both a bootloader binary (in .bin and .elf formats)
as well as a python .tgz that can be installed with pip.


Preparing Application Firmware
------------------------------
The application firmware will require the app_info.h file located in
the root of this repository.  It will also require a 2 word (8 octet)
reserved area in it's RAM region starting at 0x20000000 (see the
linker script for this bootloader for an example).

The app_info.h header contains a struct that will be needed in the
firmware.  No special placement is required, however, you will need to
have it aligned on a word boundary, like so:

    __attribute__((aligned (4)))
    static const struct app_info_block info_block = {
    	.magic_number = APP_INFO_MAGIC_NUMBER,
    	.info_crc = 0xDEADBEEF,
    };


Post procesing the firmware (which inserts CRC's and starting
information) can be accomplished thusly:

asl_f4_fw_postprocess -f $(TARGET).bin -o $(ADDRESS) -b $(TARGET).bin -i $(TARGET).ihex

The python package generated in the build process will provide two
shell scripts:
* asl_f4_fw_postprocess: This handles CRCing the firmware and
  generating the ihex file for use in the loader
* asl_f4_loader: A shell script which can be used to load new firmware

It is also, of course, a library that can be included in your own
applications for graphical loading, etc.


Loading Firmware
----------------

To load firmware, place your device in bootloader mode (On a
RaceCapture Pro Mk2, this can be accomplished via holding down the
reset and action buttons, then releasing reset).

Then run: asl_f4_loader -f path\_to\_fw.ihex

The loader software will automatically discover the device and update
the firmware



