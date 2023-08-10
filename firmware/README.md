# Toolchain Setup

## GCC
* Download the [8-bit GCC toolchain](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers).
* Extract and append the `PATH` environment variable to point to the `bin/` directory.
  For example:
  `export PATH=$PATH:/opt/avr8-gnu-toolchain-linux_x86_64/bin`

## Device pack
* Download the [AVR-Dx device pack](http://packs.download.atmel.com/)
* Unzip it and set the `AVR_DX_DFP` environment variable to point to it.
  For example:
  `export AVR_DX_DFP=/opt/AVR-Dx_DFP`

## Serial Port UPDI

Install pymcuprog
```
python3 -m pip install pymcuprog
```

Add yourself to the dialout group
```
sudo usermod -aG dialout $USER
```

Reboot for group settings to take effect
