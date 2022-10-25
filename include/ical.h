//
// Created by Leo on 2022/10/23.
//

#ifndef HWCD_ECAL_ICAL_H_
#define HWCD_ECAL_ICAL_H_

#include "icalendar.h"
#include <memory>
extern unique_ptr<ICalendar> calendar;

void download();
void read();

#endif // HWCD_ECAL_ICAL_H_
