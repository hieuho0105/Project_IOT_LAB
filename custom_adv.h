#ifndef _CUSTOM_ADV_H_
#define _CUSTOM_ADV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_bt_api.h"
#include "app_assert.h"

#define NAME_MAX_LENGTH 14

// Define for packet
#define FLAG  0x06
#define COMPANY_ID  0x02FF

  typedef struct
  {
    uint8_t len_flags;
    uint8_t type_flags;
    uint8_t val_flags;

    uint8_t len_manuf;
    uint8_t type_manuf;
    uint8_t company_LO;
    uint8_t company_HI;
    uint8_t payload[6];

    uint8_t len_name;
    uint8_t type_name;
    char name[NAME_MAX_LENGTH];

    uint8_t data_size;
  } CustomAdv_t;


void fill_adv_packet(CustomAdv_t *pData, uint8_t flags, uint16_t companyID, uint16_t temperature, uint16_t humidity, char *name);
void update_adv_data(CustomAdv_t *pData, uint8_t advertising_set_handle, uint16_t temperature, uint16_t humidity);
void start_adv(CustomAdv_t *pData, uint8_t advertising_set_handle);

#ifdef __cplusplus
}
#endif

#endif
