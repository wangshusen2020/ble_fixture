# BLE Fixture (nice!nano)

nice!nano 上的 **Nordic Blinky** 夹具固件：持续广播 `Nordic_Blinky`，并带 LED Button Service (LBS) UUID，方便 nRF Connect / 测试仪扫描。

## 刷写

1. 双击 reset，出现 `NICENANO` 盘。
2. 把 Actions 产物里的 `zephyr.uf2` 拷进去。
3. 盘马上消失、文件管理器报 “No such file or directory” **是正常的**（MCU 复位比 OS 确认拷贝更快）。

## 怎么确认固件在跑（按这个顺序）

1. **看灯**  
   未连接时板载蓝灯约 1 Hz 闪烁 = 程序已起来，并在广播。  
   灯完全不亮：多半没刷进去，再双击 reset 刷一次。

2. **先确认 USB 枚举（比串口更靠谱）**

   ```bash
   dmesg | tail -40
   lsusb
   ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
   ls /media/$USER /run/media/$USER 2>/dev/null
   ```

   | 现象 | 含义 |
   |---|---|
   | 出现 `NICENANO` 盘 | 还在 **bootloader**，应用固件没跑起来 |
   | 出现键盘 HID，没有串口 | 当前是 **ZMK** 固件（旧产物 `zmk.uf2`），本来就没有 `/dev/ttyACM0` |
   | 出现 `Nordic_Blinky` CDC / `ttyACM0` | 新 Blinky 固件已起来，可以 `cat /dev/ttyACM0` |

   **现在 CI 产物仍是 `zmk.uf2`，没有 USB 串口是正常的**，不要找 `/dev/ttyACM0`。

3. **USB 串口**（只有新 Blinky 固件才有）  
   刷完 `zephyr.uf2` 后才会出现 CDC 串口：

   ```bash
   # Linux
   screen /dev/ttyACM0 115200
   # 或
   cat /dev/ttyACM0
   ```

   应周期性打印：

   ```
   === BLE fixture / Nordic_Blinky ===
   Advertising as 'Nordic_Blinky' (LBS UUID 1523)
   up 2s  connected=0  adv_name=Nordic_Blinky  led=0
   ```

   - 有这段日志但手机搜不到：查手机蓝牙开关、距离、是否过滤了 UUID。
   - 完全没有串口：固件没跑或 USB 栈没起来，回到第 1 步看灯。

4. **nRF Connect（手机）**  
   - Scanner 里不要只搜名字，先关 Filter。  
   - 应看到 `Nordic_Blinky`，Advertising data 里有 128-bit UUID  
     `00001523-1212-efde-1523-785feabcd123`。  
   - 点连接后可写 LED characteristic（`...1525...`）控制灯。

## 本地编译

依赖 [ZMK 构建镜像](https://hub.docker.com/r/zmkfirmware/zmk-build-arm) 或本机 west + Zephyr。CI 使用 ZMK 的 west 工作区只为拿到 Zephyr 和 nice!nano 板级文件。
