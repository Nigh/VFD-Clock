
# VFD-Clock

Pomodoro-VFD version

## prepare

### Get pico SDK

```shell
git clone https://gitee.com/xianii/pico-sdk
cd pico-sdk
git submodule update --init

## Add pico-sdk Path to your environment
echo export PICO_SDK_PATH=$PWD >> ~/.profile
```

### Install dependencies

```shell
sudo apt update && sudo apt install -y cmake make ninja-build gcc g++ openssl libssl-dev cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

## Compile

```shell
make
```

## Usage

```shell
# build
make
# clang-format
make format
# clear build
make clean
# rebuild
make rebuild
```

# Hardware

## pin map

- Button Left: `GP6` (Active Low)
- Button Right: `GP7` (Active Low)
- Buzzer: `GP5` (Active Low)
- VFD CS: `GP13`
- VFD RST: `GP11`
- VFD CP: `GP14`
- VFD DA: `GP15`
- VFD HV: `GP10`
