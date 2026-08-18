# BLE Fixture (nice!nano)

nice!nano 上的 **Nordic Blinky** 夹具固件：持续广播 `Nordic_Blinky`，并带 LED Button Service (LBS) UUID，方便 nRF Connect / 测试仪扫描。

这是一个**普通 Zephyr 应用**（不是 ZMK 键盘固件）。

## 刷写

1. 双击 reset，出现 `NICENANO` 盘。
2. 把 Actions 产物 `ble-fixture-nice-nano.uf2` 拷进去。
3. 盘马上消失、文件管理器报 "No such file or directory" **是正常的**（MCU 复位比 OS 确认拷贝更快）。

> 注意产物名：现在是 `ble-fixture-nice-nano.uf2`。旧的 `zmk.uf2` 是**不含蓝牙**的错误产物，别再用了。

## 之前搜不到信号的原因

CI 之前构建的是 **ZMK 自己的 app**，并且用的是 `-b nice_nano`（不带 `//zmk` 变体）。
只有 `nice_nano//zmk` 变体的 defconfig 里才有 `CONFIG_ZMK_BLE=y`；`ZMK_BLE` 自身没有
`default y`，所以它保持为 `n`，`select BT` 从未发生 —— 编译出来的固件里**根本没有蓝牙协议栈**。
灯在闪、USB 也可能枚举，但射频部分完全不存在，因此任何扫描器都搜不到。

同时 `config/*.conf`（设置广播名的那些文件）从未被传给构建，属于死文件。

现在改为：构建本仓库的 `app/`（普通 Zephyr 应用），直接用 Zephyr 原生符号
`CONFIG_BT=y` / `CONFIG_BT_PERIPHERAL=y`，并在 CI 里加了断言，
一旦 `CONFIG_BT` 不是 `y`、或链接地址不是 `0x26000`，构建直接失败。

## 怎么确认固件在跑（按这个顺序）

1. **看灯**
   - 约 1 Hz 慢闪 = 程序已起来，并在广播。
   - 约 5 Hz 快闪 = `bt_enable()` 失败（蓝牙没起来），请看串口日志。
   - 完全不亮 = 多半没刷进去，再双击 reset 刷一次。

2. **USB 串口**

   ```bash
   ls /dev/ttyACM*
   screen /dev/ttyACM0 115200   # 或 cat /dev/ttyACM0
   ```

   应周期性打印：

   ```
   === BLE fixture / Nordic_Blinky ===
   Bluetooth ready
   Advertising as 'Nordic_Blinky' (LBS UUID 1523)
   up 2s  connected=0  advertising=1  adv_name=Nordic_Blinky  led=0
   ```

   - `advertising=1` 却搜不到：查手机蓝牙开关、距离、扫描器是否开了过滤。
   - 出现 `bt_enable FAILED`：把这行日志贴出来。

3. **nRF Connect（手机）**
   - Scanner 里先关掉 Filter，不要只按名字搜。
   - 应看到 `Nordic_Blinky`；名字在广播包里，LBS 的 128-bit UUID
     `00001523-1212-efde-1523-785feabcd123` 在扫描响应里。
   - 连接后可写 LED characteristic（`...1525...`）控制灯。

## 构建

CI 用 ZMK 的 west 工作区，只是为了拿到 Zephyr 和 nice!nano 板级定义
（`zmk/app/module/boards/nicekeyboards/nice_nano`），构建的始终是本仓库的 `app/`：

```bash
west build -p always -s app -b nice_nano -- \
  -DBOARD_ROOT=<ws>/zmk/app/module \
  -DDTS_ROOT=<ws>/zmk/app/module
```

`config/west.yml` 把 ZMK 固定在某个 commit，避免上游 `main` 变动再次弄坏构建。

## 仓库结构

```
app/            固件本体（普通 Zephyr 应用）
  prj.conf      Zephyr 配置：CONFIG_BT 等
  app.overlay   led0 别名 + USB CDC 控制台
  src/main.c    广播 + LBS GATT 服务
config/west.yml west manifest（拉取 Zephyr / 板级定义）
```
