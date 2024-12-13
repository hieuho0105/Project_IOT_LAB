#include <stdio.h>
#include "sl_board_control.h"
#include "em_assert.h"
#include "glib.h"
#include "dmd.h"
#include "em_timer.h"
#include "em_cmu.h"

#ifndef LCD_MAX_LINES
#define LCD_MAX_LINES      11
#endif

/******************************************************************************/
/***************************  LOCAL VARIABLES   ******************************/
/******************************************************************************/
static GLIB_Context_t glibContext;
static uint8_t hours = 0, minutes = 0, seconds = 0;  // Giờ, phút, giây

/******************************************************************************/
/**************************   GLOBAL FUNCTIONS   *****************************/
/******************************************************************************/

/***************************************************************************//**
 * Khởi tạo ứng dụng
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

  // Start the timer to update the clock every second
  setup_timer();

  // Cập nhật lần đầu tiên
  update_time_display();
}

/***************************************************************************//**
 * Cập nhật giờ và hiển thị lên LCD
 ******************************************************************************/
void update_time_display(void)
{
  char time_string[9];  // Chứa chuỗi giờ:phút:giây (hh:mm:ss)

  // Định dạng giờ:phút:giây
  snprintf(time_string, sizeof(time_string), "%02d:%02d:%02d", hours, minutes, seconds);

  // Vẽ chuỗi lên LCD
  GLIB_clear(&glibContext); // Xóa màn hình trước khi vẽ
  GLIB_drawStringOnLine(&glibContext, "Ho Cong Hieu", 0, GLIB_ALIGN_CENTER, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, time_string, 2, GLIB_ALIGN_CENTER, 5, 5, true);

  // Cập nhật màn hình LCD
  DMD_updateDisplay();
}

/***************************************************************************//**
 * Thiết lập Timer để cập nhật đồng hồ mỗi giây
 ******************************************************************************/
void setup_timer(void)
{
  CMU_ClockEnable(cmuClock_TIMER0, true);  // Kích hoạt bộ đếm Timer0

  TIMER_Init_TypeDef timer_init = TIMER_INIT_DEFAULT;
  timer_init.prescale = timerPrescale1024;
  timer_init.enable = true;
  TIMER_Init(TIMER0, &timer_init);

  TIMER_TopSet(TIMER0, 78125);
  TIMER_IntEnable(TIMER0, TIMER_IF_OF);

  NVIC_EnableIRQ(TIMER0_IRQn);  // Kích hoạt ngắt Timer0
}

/***************************************************************************//**
 * Xử lý sự kiện ngắt từ Timer
 ******************************************************************************/
void TIMER0_IRQHandler(void)
{
  // Xóa cờ ngắt
  TIMER_IntClear(TIMER0, TIMER_IF_OF);

  // Cập nhật giây
  seconds++;
  if (seconds >= 60) {
    seconds = 0;
    minutes++;
    if (minutes >= 60) {
      minutes = 0;
      hours++;
      if (hours >= 24) {
        hours = 0;
      }
    }
  }

  // Cập nhật màn hình LCD
  update_time_display();
}

/***************************************************************************//**
 * Ticking function.
 * Chức năng này có thể sử dụng nếu bạn cần xử lý thêm các tác vụ khác.
 ******************************************************************************/
void memlcd_app_process_action(void)
{
  return;
}
