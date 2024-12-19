#include <stdio.h>
#include "sl_board_control.h"
#include "em_assert.h"
#include "glib.h"
#include "dmd.h"
#include "em_timer.h"
#include "em_cmu.h"
#include "dht11.h"
#include "em_gpio.h"
#include "app_assert.h"
#include "app_timer.h"
#include "app_lcd.h"

#ifndef LCD_MAX_LINES
#define LCD_MAX_LINES      11
#endif


#define DHT11_PORT gpioPortB
#define DHT11_PIN  1
/******************************************************************************/
/***************************  GLOBAL VARIABLES   ******************************/
/******************************************************************************/
uint8_t presence, rh_byte1, rh_byte2, temp_byte1, temp_byte2, checksum;
extern uint16_t dht11_period;

/******************************************************************************/
/***************************  LOCAL VARIABLES    ******************************/
/******************************************************************************/

//This action creates a memory area for our "timer variable".
static app_timer_t update_timer;
static GLIB_Context_t glibContext;

/******************************************************************************/
/**************************   GLOBAL FUNCTIONS   *****************************/

/******************************************************************************/
/**************************************************************************//**
 * Messure humidity and temperature. Update to LCD
 *****************************************************************************/
static void update_timer_cb(app_timer_t *timer, void *data)
{
  (void)data;
  (void)timer;

  //update the temperature and humidity
  DHT11_Start(DHT11_PORT, DHT11_PIN);
  dht11_data_t dht11_data = DHT11_Read(DHT11_PORT, DHT11_PIN);

  rh_byte1 = dht11_data.rh_byte1;
  rh_byte2 = dht11_data.rh_byte2;
  temp_byte1 = dht11_data.temp_byte1;
  temp_byte2 = dht11_data.temp_byte2;
  checksum = dht11_data.checksum;

  // Update the display
  display();
}


/***************************************************************************//**
 * Update
 ******************************************************************************/
void display(void)
{
  char temp_string[16];
  char hum_string[16];

  snprintf(temp_string, 16, "Temp: %d.%d*C", temp_byte1, temp_byte2);
  snprintf(hum_string, 16, "Hum: %d.%d %%", rh_byte1, rh_byte2);

  // Draw the string on the LCD
  GLIB_clear(&glibContext); // Clear the screen
  GLIB_drawStringOnLine(&glibContext, "Deadline Team", 0, GLIB_ALIGN_CENTER, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, temp_string, 2, GLIB_ALIGN_CENTER, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, hum_string, 4, GLIB_ALIGN_CENTER, 5, 5, true);

  // Update the display
  DMD_updateDisplay();
}

/***************************************************************************//**
 * Initialize the application.
 ******************************************************************************/
void memlcd_app_init(void)
{
  uint32_t status;

  // Enable the memory LCD
  status = sl_board_enable_display();
  EFM_ASSERT(status == SL_STATUS_OK);

  // Initialize the DMD support for memory LCD display
  status = DMD_init(0);
  EFM_ASSERT(status == DMD_OK);

  // Initialize the GLIB context
  status = GLIB_contextInit(&glibContext);
  EFM_ASSERT(status == GLIB_OK);

  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;

  // Fill LCD with background color
  GLIB_clear(&glibContext);

  // Use Narrow font
  GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_FontNarrow6x8);

  // Update the display for the first time
  display();

  sl_status_t sc;
  // Init IRQ update data.
  sc = app_timer_start(&update_timer,
                             dht11_period,              //ms
                             update_timer_cb,
                             NULL,
                             true);
  app_assert_status(sc);
}


/***************************************************************************//**
 * Ticking function.
 ******************************************************************************/
void memlcd_app_process_action(void)
{
  return;
}
