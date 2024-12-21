/*
 * lcd_app.h
 *
 *  Created on: Nov 4, 2024
 *      Author: Phat_Dang
 */

#ifndef APP_LCD_H_
#define APP_LCD_H_
#include "app_timer.h"

void memlcd_app_init(void);

void display(void);

void memlcd_app_process_action(void);

void lcd_update_timer_cb(app_timer_t *timer, void *data);
#endif /* APP_LCD_H_ */
