# BLE Beacon for nice!nano

This project turns a [nice!nano](https://nicekeyboards.com/nice-nano/) into a BLE beacon broadcasting as `Nordic_Blinky`.

## Features
- Device name: `Nordic_Blinky`
- Advertises Nordic Blinky (LBS) service UUID
- Connectable BLE peripheral
- Built using Zephyr RTOS

## How to Build

### Option 1: GitHub Actions (Recommended)
1. Fork this repository
2. Go to Actions tab
3. Run the "Build BLE Beacon Firmware" workflow
4. Download the generated `ble-beacon-nice-nano.uf2` from artifacts

### Option 2: Local Build
```bash
west init -l app
west update
west build -b nice_nano_v2 app
