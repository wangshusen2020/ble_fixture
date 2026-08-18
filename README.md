# BLE Fixture (nice!nano)

nice!nano 上的 **Nordic Blinky** 夹具固件：通电后持续 legacy 广播，主广播包里带完整名字 `Nordic_Blinky`，scan response 里带 LED Button Service (LBS) UUID。工厂治具做被动扫描也能抓到名字。

## 刷写

1. 双击 reset，出现 `NICENANO` 盘。
2. 把 Actions 产物里的 **`zephyr.uf2`** 拷进去。
   - 原版 nice!nano 用 `ble-fixture-nice_nano`
   - nice!nano v2 用 `ble-fixture-nice_nano_v2`
3. 盘马上消失、文件管理器报 “No such file or directory” **是正常的**（MCU 复位比 OS 确认拷贝更快）。

不要刷旧的 `zmk.uf2`：那是 ZMK 键盘固件，HID 广播、可能休眠，治具扫不到 LBS / 名字。

## 怎么确认固件在跑

1. **看灯**  
   未连接时板载蓝灯约 1 Hz 闪烁 = 程序已起来，并在广播。  
   灯完全不亮：多半没刷进去，再双击 reset 刷一次。

2. **USB 枚举**

   | 现象 | 含义 |
   |---|---|
   | 出现 `NICENANO` 盘 | 还在 **bootloader**，应用没跑起来 |
   | 出现键盘 HID，没有串口 | 仍是 **ZMK**（旧产物 `zmk.uf2`） |
   | 出现 `Nordic_Blinky` CDC / `ttyACM0` | Blinky 固件已起来 |

3. **USB 串口**（`zephyr.uf2`）

   ```bash
   screen /dev/ttyACM0 115200
   ```

   应周期性打印：

   ```
   === BLE fixture / Nordic_Blinky ===
   Advertising as 'Nordic_Blinky' (name in ADV, LBS UUID in SR)
   up 2s  connected=0  adv=1  name=Nordic_Blinky  led=0
   ```

4. **扫描**
   - 被动扫描 / 产线治具：ADV 里就是完整名字 `Nordic_Blinky`（不再只放 scan response）。
   - nRF Connect：关 Filter，应看到 `Nordic_Blinky`，scan response 有 UUID  
     `00001523-1212-efde-1523-785feabcd123`。
   - 连接后可写 LED characteristic（`...1525...`）控制灯。

## 广播内容

| 包 | 字段 |
|---|---|
| ADV（约 20–30 ms，+8 dBm，connectable） | Flags + Complete Local Name `Nordic_Blinky` |
| Scan Response | 128-bit LBS UUID |

无配对、无隐私地址、无休眠。USB 供电即可，不必接电池。

## 本地编译

依赖 [ZMK 构建镜像](https://hub.docker.com/r/zmkfirmware/zmk-build-arm)（用来拿 Zephyr 和 nice!nano 板级文件）：

```bash
west init -l config
west update --narrow
west zephyr-export
west build -s app -b nice_nano_v2 -- \
  -DCONF_FILE=$PWD/app/prj.conf \
  -DDTC_OVERLAY_FILE=$PWD/app/app.overlay
# 产物: build/zephyr/zephyr.uf2
```
