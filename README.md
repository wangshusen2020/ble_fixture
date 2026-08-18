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

2. **USB 串口**（最方便的 debug）  
   刷完后用数据线连电脑，会出现 `Nordic_Blinky` CDC 串口（Linux 一般是 `/dev/ttyACM0`）：

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

3. **nRF Connect（手机）**  
   - Scanner 里不要只搜名字，先关 Filter。  
   - 应看到 `Nordic_Blinky`，Advertising data 里有 128-bit UUID  
     `00001523-1212-efde-1523-785feabcd123`。  
   - 点连接后可写 LED characteristic（`...1525...`）控制灯。

## 本地编译

依赖 [ZMK 构建镜像](https://hub.docker.com/r/zmkfirmware/zmk-build-arm) 或本机 west + Zephyr。CI 使用 ZMK 的 west 工作区只为拿到 Zephyr 和 nice!nano 板级文件。
