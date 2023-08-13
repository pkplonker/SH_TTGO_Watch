#define SERIAL_BAUD_RATE 115200
#define SCREEN_BRIGHTNESS 255

#define CPU_FREQ_MIN 80
#define CPU_FREQ_MAX 240

#define SILENT_WAKE_TIME_SECONDS 30
#define SCREEN_ACTIVE_TIME 15000
#define SCREEN_ACTIVE_TIMEOUT_ENABLED true

#define SILENT_WAKE_ENABLE true
#define BT_NAME "SH TTGO WATCH"

#define POWER_CHARGE_CURRENT 500

#define IRQ_POWER_FLAG _BV(1)
#define IRQ_BMA_FLAG _BV(2)
#define IRQ_TOUCH_FLAG _BV(3)
#define IRQ_STEP_FLAG _BV(4)

#define GPIO_POWER ((uint64_t)1 << 35)
#define GPIO_RTC ((uint64_t)1 << 37)
#define GPIO_TOUCH ((uint64_t)1 << 38)
#define GPIO_BMA ((uint64_t)1 << 39)