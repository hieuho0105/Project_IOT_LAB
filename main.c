#include <app_adv.h>
#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "app_lcd.h"
#include "string.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_system_kernel.h"
#else // SL_CATALOG_KERNEL_PRESENT
#include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT
#include <stdio.h>
#include "app_assert.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "sl_bluetooth.h"
#include "custom_adv.h"
#include <app_adv.h>

#define BSP_TXPORT gpioPortA
#define BSP_RXPORT gpioPortA
#define BSP_TXPIN 5
#define BSP_RXPIN 6
#define BSP_ENABLE_PORT gpioPortD
#define BSP_ENABLE_PIN 4

#define BUFLEN 256

uint8_t buffer[BUFLEN];
uint8_t buffer_index = 0;
// cc 86 ec 7d bd 86
uint16_t dht11_period = 1000, adv_period = 2000;

extern uint8_t rh_byte1, rh_byte2, temp_byte1, temp_byte2;
extern uint16_t temperature;
extern uint16_t humidity;

extern app_timer_t lcd_update_timer, adv_update_timer;

extern CustomAdv_t sData;
extern uint8_t advertising_set_handle;

bool flag = false;

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void initGPIO(void)
{
  GPIO_PinModeSet(BSP_TXPORT, BSP_TXPIN, gpioModePushPull, 1);
  GPIO_PinModeSet(BSP_RXPORT, BSP_RXPIN, gpioModeInput, 0);
  GPIO_PinModeSet(BSP_ENABLE_PORT, BSP_ENABLE_PIN, gpioModePushPull, 1);
}

/**************************************************************************//**
 * @brief
 *    USART0 initialization
 *****************************************************************************/
void initUSART0(void)
{
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

  GPIO->USARTROUTE[0].TXROUTE = (BSP_TXPORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
            | (BSP_TXPIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (BSP_RXPORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
            | (BSP_RXPIN << _GPIO_USART_RXROUTE_PIN_SHIFT);

  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_TXPEN;
  USART_InitAsync(USART0, &init);
}

void send_data(const char *data) {
  for (size_t i = 0; i < strlen(data); i++) {
    USART_Tx(USART0, data[i]);
  }
}

/**************************************************************************//**
 * @brief
 *    Process UART commands
 *****************************************************************************/
void process_command(void) {

  if (strncmp((char *)buffer + 1, "SET DHT ", 8) == 0) {

    dht11_period = atoi((char *)&buffer[9]);

    sl_status_t sc;
    //  Khởi tạo timer để doc va ghi dữ liệu len lcd
    sc = app_timer_start(&lcd_update_timer,
                             dht11_period,       // ms
                             lcd_update_timer_cb,
                             NULL,
                             true);
    app_assert_status(sc);

    sl_status_t sc_adv;
    // Khởi tạo timer để cập nhật dữ liệu quảng bá
    sc_adv = app_timer_start(&adv_update_timer,
                             dht11_period,       // ms
                             adv_update_timer_cb,
                             NULL,
                             true);
    app_assert_status(sc_adv);

  } else if (strncmp((char *)buffer + 1, "SET ADV ", 8) == 0) {

    adv_period = atoi((char *)&buffer[9]);

    flag = true;


  } else if (strncmp((char *)buffer + 1, "GET DATA", 8) == 0) {

    char response[64];
    snprintf(response, sizeof(response), "TEMP: %d.%d C, RH: %d.%d %%\n",
                                         temp_byte1, temp_byte2, rh_byte1, rh_byte2);
    send_data(response);

  } else {

    send_data("Unknown command.\n");

  }
}


void USART0_RX_IRQHandler(void) {

  uint8_t received_char = USART0->RXDATA;
  USART_IntClear(USART0, USART_IF_RXDATAV);

  if (received_char == '\n') {
    //flag = true;
    buffer[buffer_index] = '\0'; // Null-terminate the string
    buffer_index = 0;           // Reset index for next command
    process_command();
  } else if (buffer_index < BUFLEN - 1) {
      buffer[buffer_index++] = received_char;
  }

}

int main(void)
{
  sl_system_init();
  adv_app_init();
  memlcd_app_init();

#if defined(SL_CATALOG_KERNEL_PRESENT)
  sl_system_kernel_start();
#else // SL_CATALOG_KERNEL_PRESENT
  initGPIO();
  initUSART0();
  USART_IntEnable(USART0, USART_IEN_RXDATAV);
  NVIC_EnableIRQ(USART0_RX_IRQn);

  while (1) {
    sl_system_process_action();
    app_process_action();

    if (flag) {
        flag = false;
        sl_bt_advertiser_stop(advertising_set_handle);

        sl_status_t sc_adv_period;
        // Tạo handle quảng bá
        //sc_adv_period = sl_bt_advertiser_create_set(&advertising_set_handle);
        //app_assert_status(sc_adv_period);

        // Thiết lập interval quảng bá
        sc_adv_period = sl_bt_advertiser_set_timing(
                        advertising_set_handle,
                        adv_period * 1.6, // Min interval (1000 ms)
                        adv_period * 1.6, // Max interval (1000 ms)
                        0,   // Thời gian quảng bá (0 = vô hạn)
                        0);  // Số sự kiện quảng bá (0 = vô hạn)
        app_assert_status(sc_adv_period);

        // Thiết lập kênh quảng bá
//        sc_adv_period = sl_bt_advertiser_set_channel_map(advertising_set_handle, 7);
//        app_assert_status(sc_adv_period);

        // Tạo gói quảng bá ban đầu
        fill_adv_packet(&sData, FLAG, COMPANY_ID, temperature, humidity, "DEADLINE");

        // Bắt đầu quảng bá
        start_adv(&sData, advertising_set_handle);
    }

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_sleep();
#endif
  }
#endif // SL_CATALOG_KERNEL_PRESENT
}
