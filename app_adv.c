/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <app_adv.h>
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app_log.h"
#include "custom_adv.h"
#include "app_timer.h"

/******************************************************************************/
/***************************  GLOBAL VARIABLES   ******************************/
/******************************************************************************/
extern uint8_t rh_byte1, rh_byte2, temp_byte1, temp_byte2;
extern uint16_t adv_period;
CustomAdv_t sData; // Our custom advertising data stored here
static uint16_t temperature;
static uint16_t humidity;
/******************************************************************************/
/***************************  LOCAL VARIABLES   ******************************/
/******************************************************************************/
//This action creates a memory area for our "timer variable".
app_timer_t adv_update_timer;

// The advertising set handle allocated from Bluetooth stack.
uint8_t advertising_set_handle = 0xff;

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void adv_update_timer_cb(app_timer_t *timer, void *data)
{
  (void)data;
  (void)timer;
  //You can update other data in this void
  temperature = (temp_byte1 * 10) + temp_byte2; // Ví dụ: 28.8 -> 0288
  humidity = (rh_byte1 * 10) + rh_byte2;     // Ví dụ: 80.1 -> 0801
  // Log thông tin
  //app_log("Updating advertisement: Temp = %d.%1d, Hum = %d.%1d\r\n",
  //        temp_byte1, temp_byte2, rh_byte1, rh_byte2);

  // Cập nhật dữ liệu quảng bá
  update_adv_data(&sData, advertising_set_handle, temperature, humidity);
}

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void adv_app_init(void)
{
  sl_status_t sc;

  // Khởi tạo timer để cập nhật dữ liệu quảng bá mỗi 5 giây
  sc = app_timer_start(&adv_update_timer,
                       adv_period, // 1000 (ms)
                       adv_update_timer_cb,
                       NULL,
                       true);
  app_assert_status(sc);
}

/**************************************************************************//**
 * Xử lý sự kiện từ BLE stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      app_log("System booted\r\n");

      // Tạo handle quảng bá
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Thiết lập interval quảng bá
      sc = sl_bt_advertiser_set_timing(
          advertising_set_handle,
          adv_period*1.6, // Min interval (1000 ms)
          adv_period*1.6, // Max interval (1000 ms)
          0,   // Thời gian quảng bá (0 = vô hạn)
          0);  // Số sự kiện quảng bá (0 = vô hạn)
      app_assert_status(sc);

      // Thiết lập kênh quảng bá
      sc = sl_bt_advertiser_set_channel_map(advertising_set_handle, 7);
      app_assert_status(sc);

      // Tạo gói quảng bá ban đầu
      fill_adv_packet(&sData, FLAG, COMPANY_ID, temperature, humidity, "DEADLINE");
      //app_log("fill_adv_packet completed\r\n");

      // Bắt đầu quảng bá
      start_adv(&sData, advertising_set_handle);
      //app_log("Started advertising with default data\r\n");
      break;

    case sl_bt_evt_connection_opened_id:
      //app_log("Connection opened\r\n");
      break;

    case sl_bt_evt_connection_closed_id:
      // Restart quảng bá sau khi ngắt kết nối
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);

      //app_log("Connection closed, restarted advertising\r\n");
      break;

    default:
      break;
  }
}

void app_process_action() {

}
