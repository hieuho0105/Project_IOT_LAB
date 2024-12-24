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


 void Set_Pin_Output(GPIO_Port_TypeDef port, unsigned int pin) {
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(port, pin, gpioModePushPull, 0);
}

 void Set_Pin_Input(GPIO_Port_TypeDef port, unsigned int pin) {
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(port, pin, gpioModeInput, 1);
}


void DHT11_Start(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin) {

  // MCU send out start signal and pull down voltage for at least 18ms to let DHT11 detect the signal
  Set_Pin_Output(dht11_port, dht11_pin);
  GPIO_PinOutClear(dht11_port, dht11_pin);
  sl_udelay_wait(18000); // wait for 18ms

  // MCU pull up voltage and waits for DHT respond (20 - 40us)
  GPIO_PinOutSet(dht11_port, dht11_pin);
  sl_udelay_wait(20); // wait for 20us
  Set_Pin_Input(dht11_port, dht11_pin);

}


uint8_t DHT11_Check_Response(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin) {
  uint8_t response = 0, timeout = 0;

  // DHT send out respond signal and keep it for 80us
  sl_udelay_wait(40);
  if (!GPIO_PinInGet(dht11_port, dht11_pin)) {
      // DHT pull up voltage and keep it for 80us
      sl_udelay_wait(80);
      if (GPIO_PinInGet(dht11_port, dht11_pin))
        response = 1;
      else
        response = 0;
  }
  else {
      response = 0;
  }

  while (GPIO_PinInGet(dht11_port, dht11_pin) == 1 && timeout < 40) { // wait for the pin go to low
      timeout++;
      sl_udelay_wait(1);
  }
  return response;
}

uint8_t DHT11_Read_Byte(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin) {
  uint8_t i = 0, timeout = 0;
  for (uint8_t j = 0; j < 8; j++) {
      while (!GPIO_PinInGet(dht11_port, dht11_pin));
      sl_udelay_wait(40);

      if (GPIO_PinInGet(dht11_port, dht11_pin))
         i |= 1 << (7 - j);

      while (GPIO_PinInGet(dht11_port, dht11_pin) == 1 && timeout < 60) { // wait for the pin go to low
          timeout++;
          sl_udelay_wait(1);
      }
  }

  return i;
}

dht11_data_t DHT11_Read(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin) {
  
  dht11_data_t dht11_data;
  dht11_data.rh_byte1 = 0;
  dht11_data.rh_byte2 = 0;
  dht11_data.temp_byte1 = 0;
  dht11_data.temp_byte2 = 0;
  dht11_data.checksum = 0;

  DHT11_Start(dht11_port, dht11_pin);
  uint8_t presence = DHT11_Check_Response(dht11_port, dht11_pin);

  if (presence == 1) {
      dht11_data.rh_byte1 = DHT11_Read_Byte(dht11_port, dht11_pin);
      dht11_data.rh_byte2 = DHT11_Read_Byte(dht11_port, dht11_pin);
      dht11_data.temp_byte1 = DHT11_Read_Byte(dht11_port, dht11_pin);
      dht11_data.temp_byte2 = DHT11_Read_Byte(dht11_port, dht11_pin);
      dht11_data.checksum = DHT11_Read_Byte(dht11_port, dht11_pin);
  }

  return dht11_data;
}
