#ifndef HWCD_INCLUDE_GESTURE_H_
#define HWCD_INCLUDE_GESTURE_H_
extern RTC_SLOW_ATTR uint8_t mode;
void gesture_init();
void gesture_up();
void gesture_down();
void gesture_left();
void gesture_right();
void handle_gesture();

#endif // HWCD_INCLUDE_GESTURE_H_
