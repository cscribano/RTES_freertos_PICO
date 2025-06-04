# FreeRTOS Project for Raspberry PI Pico (WITHOUT microROS)
This branch `noros` does not include microROS library or examples. If you are interested in working with microROS switch to the branch corresponding to your ROS2 distro.

## Dependencies

### 0. Configure Groups
``` bash
# add user to groups
sudo usermod -a -G plugdev $USER
sudo usermod -a -G dialout $USER

# To mount PICO without sudo
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", ATTR{idProduct}=="0003", MODE="0660", GROUP="plugdev"' | sudo tee /etc/udev/rules.d/99-rpi-pico.rules > /dev/null
```
(reboot your system after)

### 1. Install Pico SDK
First, make sure the Pico SDK is properly installed and configured:

```bash
# Install dependencies
sudo apt install cmake g++ gcc-arm-none-eabi doxygen libnewlib-arm-none-eabi git python3
git clone --recurse-submodules https://github.com/raspberrypi/pico-sdk.git $HOME/pico-sdk

# Configure environment
echo "export PICO_SDK_PATH=$HOME/pico-sdk" >> ~/.bashrc
source ~/.bashrc
```

### 3. Clone this repository


```bash
git clone --recursive -b noros https://github.com/cscribano/RTES_freertos_PICO.git
```

## Running Examples

#### Build all the examples

```bash
cd RTES_freertos_PICO
mkdir build
cd build
cmake ..
make
```

#### Flash an example

To flash (example 01_task) hold the boot button, plug the USB and run:
```bash
cp 01_task.uf2 $(findmnt -rn -o TARGET -S LABEL=RPI-RP2)/
```

## How to use Pico SDK?

Here is a Raspberry Pi Pico C/C++ SDK documentation:
https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf


## Notes
 This package is released for teaching and educational purposes only.