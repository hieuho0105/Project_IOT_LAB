/*
 * dht11.h
 *
 *  Created on: Dec 13, 2024
 *      Author: Ho Cong Hieu
 */

#ifndef DHT11_H_
#define DHT11_H_

#include "em_gpio.h"
#include <stdint.h>


void DHT11_Start(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin);
uint8_t DHT11_Check_Response(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin);
uint8_t DHT11_Read(GPIO_Port_TypeDef dht11_port, unsigned int dht11_pin);




#endif /* DHT11_H_ */
