# BLE Fixture (nice!nano)

Turns a [nice!nano v2](https://nicekeyboards.com/nice-nano/) into a continuously advertising BLE peripheral named `Nordic_Blinky`.

Firmware is built with [ZMK](https://zmk.dev/) using a mock 1-key matrix so the controller can run without a keyboard PCB.

## Build

Push to GitHub (or run **Actions → Build ZMK firmware**) and download the `firmware` artifact (`ble-fixture-nice-nano.uf2`).

## Flash

1. Double-tap reset on the nice!nano to enter the bootloader.
2. Copy `ble-fixture-nice-nano.uf2` onto the `NICENANO` drive.
