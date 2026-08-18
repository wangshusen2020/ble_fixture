#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>

/* Nordic Blinky / LED Button Service (LBS) */
#define BT_UUID_LBS_VAL \
	BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_LBS_BUTTON_VAL \
	BT_UUID_128_ENCODE(0x00001524, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_LBS_LED_VAL \
	BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd123)

static const struct bt_uuid_128 lbs_uuid = BT_UUID_INIT_128(BT_UUID_LBS_VAL);
static const struct bt_uuid_128 lbs_button_uuid = BT_UUID_INIT_128(BT_UUID_LBS_BUTTON_VAL);
static const struct bt_uuid_128 lbs_led_uuid = BT_UUID_INIT_128(BT_UUID_LBS_LED_VAL);

#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
static bool led_ready;
static uint8_t led_state;
static uint8_t button_state;
static volatile bool connected;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LBS_VAL),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void apply_led(uint8_t on)
{
	led_state = on ? 1 : 0;
	if (led_ready) {
		gpio_pin_set_dt(&led, led_state);
	}
}

static ssize_t read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			   void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &button_state,
				 sizeof(button_state));
}

static ssize_t read_led(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &led_state, sizeof(led_state));
}

static ssize_t write_led(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	uint8_t value;

	if (offset != 0 || len != 1) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	value = ((const uint8_t *)buf)[0];
	apply_led(value);
	printk("LBS LED write -> %u\n", led_state);
	return len;
}

static void button_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	printk("LBS button CCC %s\n", value ? "notify on" : "notify off");
}

BT_GATT_SERVICE_DEFINE(lbs_svc,
	BT_GATT_PRIMARY_SERVICE(&lbs_uuid),
	BT_GATT_CHARACTERISTIC(&lbs_button_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_button, NULL, NULL),
	BT_GATT_CCC(button_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(&lbs_led_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			       read_led, write_led, NULL),
);

static void start_adv(void)
{
#ifdef BT_LE_ADV_CONN_FAST_1
	int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
#else
	int err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
#endif

	if (err == -EALREADY) {
		return;
	}
	if (err) {
		printk("Advertising FAILED (%d)\n", err);
	} else {
		printk("Advertising as '%s' (LBS UUID 1523)\n", DEVICE_NAME);
	}
}

static void connected_cb(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	if (err) {
		printk("Connect failed %s (err %u)\n", addr, err);
		connected = false;
		start_adv();
		return;
	}
	connected = true;
	printk("Connected %s\n", addr);
}

static void disconnected_cb(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	connected = false;
	printk("Disconnected %s (reason 0x%02x)\n", addr, reason);
	start_adv();
}

BT_CONN_CB_DEFINE(conn_cbs) = {
	.connected = connected_cb,
	.disconnected = disconnected_cb,
};

static int init_led(void)
{
	if (!gpio_is_ready_dt(&led)) {
		printk("LED GPIO not ready (firmware still runs)\n");
		return -ENODEV;
	}
	if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) != 0) {
		printk("LED configure failed\n");
		return -EIO;
	}
	led_ready = true;
	printk("LED ready on P0.15 — blinks while advertising\n");
	return 0;
}

int main(void)
{
	int err;

	printk("\n=== BLE fixture / Nordic_Blinky ===\n");
	printk("Board: nice!nano  build: %s %s\n", __DATE__, __TIME__);

	init_led();

	err = bt_enable(NULL);
	if (err) {
		printk("bt_enable FAILED (%d) — check USB log / LED pattern\n", err);
	} else {
		printk("Bluetooth ready\n");
		start_adv();
	}

	uint32_t tick = 0;
	while (1) {
		tick++;
		if (!connected && led_ready) {
			/* 1 Hz heartbeat: firmware is alive and advertising */
			gpio_pin_toggle_dt(&led);
		}
		if ((tick % 4) == 0) {
			printk("up %us  connected=%d  adv_name=%s  led=%u\n",
			       tick / 2, connected, DEVICE_NAME, led_state);
		}
		k_sleep(K_MSEC(500));
	}

	return 0;
}
