#include <string.h>
#include "custom_adv.h"
#include "stdio.h"

// Function to fill the advertisement packet
void fill_adv_packet(CustomAdv_t *pData, uint8_t flags, uint16_t companyID, uint16_t temperature, uint16_t humidity, char *name)
{
    int n;

    // Flags
    pData->len_flags = 0x02;
    pData->type_flags = 0x01;
    pData->val_flags = flags;

    // Manufacturer-specific data
    pData->len_manuf = 7; // 1+2+4 bytes for type, company ID and the payload
    pData->type_manuf = 0xFF;
    pData->company_LO = companyID & 0xFF;
    pData->company_HI = (companyID >> 8) & 0xFF;

    // Encode temperature
    pData->payload[0] = ((temperature / 1000) & 0x0F) << 4 | ((temperature / 100) % 10); // Hàng nghìn và trăm
    pData->payload[1] = ((temperature / 10) % 10) << 4 | (temperature % 10);             // Hàng chục và đơn vị

    // Encode humidity
    pData->payload[2] = ((humidity / 1000) & 0x0F) << 4 | ((humidity / 100) % 10); // Hàng nghìn và trăm
    pData->payload[3] = ((humidity / 10) % 10) << 4 | (humidity % 10);             // Hàng chục và đơn vị

    // Device name
    n = strlen(name);
    pData->type_name = (n > NAME_MAX_LENGTH) ? 0x08 : 0x09; // Shortened or complete local name
    strncpy(pData->name, name, NAME_MAX_LENGTH);
    pData->len_name = 1 + ((n > NAME_MAX_LENGTH) ? NAME_MAX_LENGTH : n);

    // Total data size
    pData->data_size = 3 + (1 + pData->len_manuf) + (1 + pData->len_manuf) + (1 + pData->len_name);
}

// Function to update advertisement data
void update_adv_data(CustomAdv_t *pData, uint8_t advertising_set_handle, uint16_t temperature, uint16_t humidity)
{
    sl_status_t sc;

    // Encode temperature
    pData->payload[0] = ((temperature / 1000) & 0x0F) << 4 | ((temperature / 100) % 10); // Hàng nghìn và trăm
    pData->payload[1] = ((temperature / 10) % 10) << 4 | (temperature % 10);             // Hàng chục và đơn vị

    // Encode humidity
    pData->payload[2] = ((humidity / 1000) & 0x0F) << 4 | ((humidity / 100) % 10); // Hàng nghìn và trăm
    pData->payload[3] = ((humidity / 10) % 10) << 4 | (humidity % 10);             // Hàng chục và đơn vị

    // Update advertisement data
    sc = sl_bt_legacy_advertiser_set_data(advertising_set_handle, 0, pData->data_size, (const uint8_t *)pData);
    app_assert(sc == SL_STATUS_OK, "[E: 0x%04x] Failed to update advertising data\n", (int)sc);
}

void start_adv(CustomAdv_t *pData, uint8_t advertising_set_handle)
{
  sl_status_t sc;
  // Đặt dữ liệu quảng bá
  sc = sl_bt_legacy_advertiser_set_data(advertising_set_handle, 0, pData->data_size, (const uint8_t *)pData);
  app_assert(sc == SL_STATUS_OK, "[E: 0x%04x] Failed to set advertising data\n", (int)sc);

  // Bắt đầu quảng bá
  sc = sl_bt_legacy_advertiser_start(advertising_set_handle, sl_bt_advertiser_connectable_scannable);
  app_assert(sc == SL_STATUS_OK, "[E: 0x%04x] Failed to start advertising\n", (int)sc);
}
