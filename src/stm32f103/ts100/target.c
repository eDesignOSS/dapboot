#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>

#include "config.h"

void target_pre_main(void) {
  rcc_set_usbpre(0x1);
  rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USBEN | RCC_APB1ENR_I2C1EN);
}
