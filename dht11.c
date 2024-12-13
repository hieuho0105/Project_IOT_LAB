/*
 * dht11.c
 *
 *  Created on: Dec 13, 2024
 *      Author: Ho Cong Hieu
 */

#include "dht11.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "stdint.h"
#include "sl_sleeptimer.h"
#include "sl_udelay.h"


static void Set_Pin_Output(GPIO_Port_TypeDef port, unsigned int pin) {
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(port, pin, gpioModePushPull, 0);
}

static void Set_Pin_Input(GPIO_Port_TypeDef port, unsigned int pin) {
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(port, pin, gpioModeInputPullFilter, 1);
}


void DHT11_Start(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin) {

  Set_Pin_Output(dht11_port, dht11_pin);
  GPIO_PinOutClear(dht11_port, dht11_pin);
  sl_sleeptimer_delay_millisecond(18); // wait for 18ms
  GPIO_PinOutSet(dht11_port, dht11_pin);
  sl_udelay_wait(20); // wait for 20us
  Set_Pin_Input(dht11_port, dht11_pin);
}


uint8_t DHT11_Check_Response(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin) {
  uint8_t response = 0;

  sl_udelay_wait(40);

  if (!GPIO_PinOutGet(dht11_port, dht11_pin)) {
      sl_udelay_wait(80);
      if (GPIO_PinOutGet(dht11_port, dht11_pin))
        response = 1;
      else
        response = -1; // 255
  }

  while (GPIO_PinOutGet(dht11_port, dht11_pin)); // wait for the pin go to low

  return response;
}

uint8_t DHT11_Read(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin) {
  uint8_t i = 0;
  for (uint8_t j = 0; j < 8; j++) {
      while (!GPIO_PinOutGet(dht11_port, dht11_pin));
      sl_udelay_wait(40);

      if (!GPIO_PinOutGet(dht11_port, dht11_pin))
        i &= ~(1 << (7 - j));
      else
        i |= 1 << (7 - j);

      while (GPIO_PinOutGet(dht11_port, dht11_pin)); // wait for the pin go to low
  }

  return i;
}
