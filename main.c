/***************************************************************************//**
 * @file main.c
 * @brief main() function.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/


/*

Mô tả chi tiết:
1. PC giao tiếp với EFR32xG21 thông qua UART để thực hiện các chức năng:
• Cấu hình: Chu kì đo nhiệt độ/độ ẩm, chu kì quảng bá của BLE.
• Lấy thông tin nhiệt độ và độ ẩm.
• Một số tính năng khác sinh viên tự đề xuất (không bắt buộc).
2. EFR32xG21 đọc nhiệt độ và độ ẩm từ cảm biến DHT1 theo chu kì và dữ liệu được
hiển thị trên LCD bao gồm: Giá trị nhiệt độ, độ ẩm và chu kì đọc cảm biến.
3. EFR32xG21 sử dụng BLE để quảng ra môi trường xung quanh. Gói tin quảng bá
bao gồm: Tên thiết bị, giá trị nhiệt độ và độ ẩm.
 
*/
#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "app.h"
#include "app_lcd.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_system_kernel.h"
#else // SL_CATALOG_KERNEL_PRESENT
#include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT

#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

#define BSP_TXPORT gpioPortA
#define BSP_RXPORT gpioPortA
#define BSP_TXPIN 5
#define BSP_RXPIN 6
#define BSP_ENABLE_PORT gpioPortD
#define BSP_ENABLE_PIN 4

uint16_t dht11_period = 1000, adv_period = 1000;

extern uint8_t rh_byte1, rh_byte2, temp_byte1, temp_byte2;

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void initGPIO(void)
{
  // Configure the USART TX pin to the board controller as an output
  GPIO_PinModeSet(BSP_TXPORT, BSP_TXPIN, gpioModePushPull, 1);

  // Configure the USART RX pin to the board controller as an input
  GPIO_PinModeSet(BSP_RXPORT, BSP_RXPIN, gpioModeInput, 0);

  /*
   * Configure the BCC_ENABLE pin as output and set high.  This enables
   * the virtual COM port (VCOM) connection to the board controller and
   * permits serial port traffic over the debug connection to the host
   * PC.
   *
   * To disable the VCOM connection and use the pins on the kit
   * expansion (EXP) header, comment out the following line.
   */
  GPIO_PinModeSet(BSP_ENABLE_PORT, BSP_ENABLE_PIN, gpioModePushPull, 1);
}

/**************************************************************************//**
 * @brief
 *    USART0 initialization
 *****************************************************************************/
void initUSART0(void)
{
  // Default asynchronous initializer (115.2 Kbps, 8N1, no flow control)
  //USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
  USART_InitAsync_TypeDef init;

  init.enable = usartEnable;
  init.refFreq = 0;
  init.baudrate = 115200;
  init.oversampling = usartOVS16;
  init.databits = usartDatabits8;
  init.parity = USART_FRAME_PARITY_NONE;
  init.stopbits = usartStopbits1;

  init.mvdis = false;
  init.prsRxEnable = false;
  init.prsRxCh = 0;

  init.autoCsEnable = false;
  init.csInv = false;
  init.autoCsHold = 0;
  init.autoCsSetup = 0;
  init.hwFlowControl = usartHwFlowControlNone;

  // Route USART0 TX and RX to the board controller TX and RX pins
  GPIO->USARTROUTE[0].TXROUTE = (BSP_TXPORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
            | (BSP_TXPIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (BSP_RXPORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
            | (BSP_RXPIN << _GPIO_USART_RXROUTE_PIN_SHIFT);

  // Enable RX and TX signals now that they have been routed
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_TXPEN;

  // Configure and enable USART0
  USART_InitAsync(USART0, &init);
}

int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, processing task(s) will be created by
  // this call.
  sl_system_init();

  // Initialize the application. For example, create periodic timer(s) or
  // task(s) if the kernel is present.
  app_init();
  memlcd_app_init();
#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Start the kernel. Task(s) created in app_init() will start running.
  sl_system_kernel_start();
#else // SL_CATALOG_KERNEL_PRESENT

  //uint8_t buffer;

  // Initialize GPIO and USART0
  initGPIO();
  initUSART0();

  while (1) {
    // Do not remove this call: Silicon Labs components process action routine
    // must be called from the super loop.
    sl_system_process_action();

    // Application process.
    app_process_action();

  // Zero out buffer
//    buffer = 0;
//   // Receive BUFLEN characters unless a new line is received first
//   do
//   {
//     // Wait for a character
//     buffer = USART_Rx(USART0);
//   }
//   while ( buffer == 0 );
//
//   switch (buffer)
//   {
//     case '1':
//        dht11_period = 0;
//        dht11_period = USART_Rx(USART0);
//        dht11_period |= (USART_Rx(USART0) << 8);
//        break;
//     case '2':
//        adv_period = 0;
//        adv_period = USART_Rx(USART0);
//        adv_period |= (USART_Rx(USART0) << 8);
//        break;
//      case '3':
//        USART_Tx(USART0, rh_byte1);
//        USART_Tx(USART0, rh_byte2);
//        USART_Tx(USART0, temp_byte1);
//        USART_Tx(USART0, temp_byte2);
//        break;
//     default:
//        // Do something else
//        break;
//   }

    

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#endif
  }
#endif // SL_CATALOG_KERNEL_PRESENT
}
