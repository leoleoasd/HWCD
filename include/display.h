//
// Created by Leo on 2022/10/24.
//

#ifndef HWCD_ECAL_DISPLAY_H_
#define HWCD_ECAL_DISPLAY_H_
#include "DEV_Config.h"
#include "utility/Debug.h"
#include "utility/EPD_7in5b_V2.h"
#include "GUI_Paint.h"

void display_init();
void display_calendar();
void display_clear();
void set_start_of_current_week();
void calendar_left();
void calendar_right();

#endif // HWCD_ECAL_DISPLAY_H_
