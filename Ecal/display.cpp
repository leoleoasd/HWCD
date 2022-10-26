#include "display.h"

#include "ical.h"
#include <iomanip>
#include <sstream>

constexpr size_t image_size = (EPD_7IN5B_V2_WIDTH / 8) * EPD_7IN5B_V2_HEIGHT;

uint8_t *black_image;
uint8_t *red_image;

static const char *TAG = "Ecal display";

void display_init() {
  pinMode(EPD_BUSY_PIN, INPUT);
  pinMode(EPD_RST_PIN, OUTPUT);
  pinMode(EPD_DC_PIN, OUTPUT);

  pinMode(EPD_SCK_PIN, OUTPUT);
  pinMode(EPD_MOSI_PIN, OUTPUT);
  pinMode(EPD_CS_PIN, OUTPUT);

  digitalWrite(EPD_CS_PIN, HIGH);
  digitalWrite(EPD_SCK_PIN, LOW);
  EPD_7IN5B_V2_Init();
  black_image = (uint8_t *)ps_malloc(image_size);
  red_image = (uint8_t *)ps_malloc(image_size);
  Paint_NewImage(black_image, EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT, 0,
                 WHITE);
  Paint_NewImage(red_image, EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT, 0, WHITE);
  Paint_SelectImage(black_image);
  Paint_Clear(WHITE);
  Paint_SelectImage(red_image);
  Paint_Clear(WHITE);
  if (black_image == nullptr || red_image == nullptr) {
    ESP_LOGW(TAG, "Failed to allocate memory for image buffers");
    return;
  }
}

void display_clear() { EPD_7IN5B_V2_Clear(); }

RTC_NOINIT_ATTR tm display_start_time;

void set_start_of_current_week() {
  getLocalTime(&display_start_time);
  // get monday of this week
  display_start_time.tm_mday -=
      (display_start_time.tm_wday - 1); // we want monday
  if (display_start_time.tm_mday <= 0) {
    display_start_time.tm_mon -= 1;
    if (display_start_time.tm_mon < 0) {
      display_start_time.tm_mon = 11;
      display_start_time.tm_year -= 1;
    }
    display_start_time.tm_mday += Date::DaysInMonthS(
        display_start_time.tm_mon, display_start_time.tm_year + 1900);
  }
  display_start_time.tm_hour = 0;
  display_start_time.tm_min = 0;
  display_start_time.tm_sec = 0;
}

void calendar_left() {
  display_start_time.tm_mday -= 7;
  if (display_start_time.tm_mday <= 0) {
    display_start_time.tm_mon -= 1;
    if (display_start_time.tm_mon < 0) {
      display_start_time.tm_mon = 11;
      display_start_time.tm_year -= 1;
    }
    display_start_time.tm_mday += Date::DaysInMonthS(
        display_start_time.tm_mon + 1, display_start_time.tm_year + 1900);
  }
}

void calendar_right() {
  display_start_time.tm_mday += 7;
  short mdays = Date::DaysInMonthS(display_start_time.tm_mon + 1,
                                   display_start_time.tm_year + 1900);
  ESP_LOGI(TAG, "Adding week, %d month have %d days", display_start_time.tm_mon,
           mdays);
  if (display_start_time.tm_mday > mdays) {
    display_start_time.tm_mon += 1;
    if (display_start_time.tm_mon > 11) {
      display_start_time.tm_mon = 0;
      display_start_time.tm_year += 1;
    }
    display_start_time.tm_mday -= mdays;
  }
}

void display_calendar() {
  tm current_time;
  getLocalTime(&current_time);
  ESP_LOGI(TAG, "Current time: %s", asctime(&current_time));
  Date begin;
  begin[YEAR] = display_start_time.tm_year + 1900;
  begin[MONTH] = display_start_time.tm_mon + 1;
  begin[DAY] = display_start_time.tm_mday;
  begin[HOUR] = display_start_time.tm_hour;
  begin[MINUTE] = display_start_time.tm_min;
  begin[SECOND] = display_start_time.tm_sec;
  Date end = begin;
  end[DAY] += 4;
  if (end[DAY] > end.DaysInMonth()) {
    end[DAY] -= end.DaysInMonth();
    end[MONTH] += 1;
    if (end[MONTH] > 12) {
      end[MONTH] = 1;
      end[YEAR] += 1;
    }
  }
  end[HOUR] = 23;
  end[MINUTE] = 59;
  end[SECOND] = 59;

  // Screen size: 800 x 480
  // 5 days a week, 8 hours per day, we have: 160 x 60 pixels for each hour
  // we only draw from 9am to 5pm

  Paint_SelectImage(black_image);
  Paint_Clear(WHITE);
  // draw lines
  for (int i = 0; i < 6; i++) {
    Paint_DrawLine(i * 160 + (i == 0), 0, i * 160 + (i == 0), 480, BLACK,
                   DOT_PIXEL_1X1, LINE_STYLE_DOTTED1);
  }
  for (int i = 0; i < 9; i++) {
    Paint_DrawLine(0, i * 60 + (i == 0), 800, i * 60 + (i == 0), BLACK,
                   DOT_PIXEL_1X1, LINE_STYLE_DOTTED1);
  }
  // draw each event
  Event *e;
  ICalendar::Query SearchQuery(&*calendar);
  SearchQuery.Criteria.From = begin;
  SearchQuery.Criteria.To = end;
  ESP_LOGI(TAG, "Searching for events between %s and %s", string(begin).c_str(),
           string(end).c_str());
  SearchQuery.ResetPosition();
  while (e = SearchQuery.GetNextEvent(), e != nullptr) {
    ESP_LOGI(TAG, "Event: %s", e->UID.c_str());
    ESP_LOGI(TAG, "Start: %s", e->DtStart.Format().c_str());
    ESP_LOGI(TAG, "End: %s", e->DtEnd.Format().c_str());
    ESP_LOGI(TAG, "Summary: %s", e->Summary.c_str());
    Date estart = e->DtStart;
    Date eend = e->DtEnd;
    // draw a rectangle
    int x;
    if (estart[MONTH] != begin[MONTH]) {
      int days_of_last_month = Date::DaysInMonthS(begin[MONTH], begin[YEAR]);
      x = (estart[DAY] - begin[DAY] + days_of_last_month) * 160;
    } else {
      x = (estart[DAY] - begin[DAY]) * 160;
    }
    x += (x == 0);
    int y =
        (estart[HOUR] - 9) * 60 + (estart[MINUTE]); // one pixel for each minute
    y += (y == 0);
    int w = 160;
    int h;
    if (eend[DAY] == estart[DAY]) {
      h = (eend[HOUR] - estart[HOUR]) * 60 + (eend[MINUTE] - estart[MINUTE]);
    } else {
      h = 480 - y;
    }
    if (h < 32)
      h = 32;
    if (x < 0) {
      continue;
    }
    if (y < 0) {
      continue;
    }
    if (x > 800) {
      continue;
    }
    if (y > 480) {
      continue;
    }
    if (x + w > 800)
      w = 800 - x;
    if (y + h > 480) {
      h = 480 - y;
      if (h < 32) {
        h = 32;
        y = 480 - 32;
      }
    }

    //    ESP_LOGI(TAG, "Drawing rectangle at %d,%d with size %dx%d", x, y, w,
    //    h);
    Paint_DrawRectangle(x, y, x + w, y + h, BLACK, DOT_PIXEL_1X1, DRAW_FILL_1);

    // draw summary
    std::string s = e->Summary;
    // for a width of 160 pixels, we can fit 160 / 7 = 14 characters
    int line = 0;
    int string_line = 0;
    if ((line + 1) * 16 <= h) {
      // 14:15 - 16:00
      stringstream ss;
      ss << setfill('0') << setw(2);
      ss << estart[HOUR] << ":" << setfill('0') << setw(2) << estart[MINUTE]
         << " - " << setfill('0') << setw(2) << eend[HOUR] << ":"
         << setfill('0') << setw(2) << eend[MINUTE];
      Paint_DrawString_EN(x + 1, y + 1 + line * 16, ss.str().c_str(), &Font16,
                          BLACK, TRANSPARENT);
      line++;
    }
    while (string_line * 14 < s.length() && (line + 1) * 16 <= h) {
      std::string line_str;
      if ((line + 2) * 16 > h && (string_line + 1) * 14 < s.length()) {
        line_str = s.substr(string_line * 14, 11);
        line_str += "...";
      } else {
        line_str = s.substr(string_line * 14, 14);
      }
      //      ESP_LOGI(TAG, "Drawing string at %d,%d: %s", x + 1, y + 1 + line *
      //      16,
      //               line_str.c_str());
      Paint_DrawString_EN(x + 1, y + 1 + line * 16, line_str.c_str(), &Font16,
                          BLACK, TRANSPARENT);
      line++;
      string_line++;
    }
    // if last more than one day
    if (eend[DAY] != estart[DAY]) {
      // draw day in the middle
      short now = estart[DAY] + 1;
      while (now != eend[DAY] && x < 800) {
        x += 160;
        y = 0;
        w = 160;
        h = 480;
        //        ESP_LOGI(TAG, "Drawing rectangle at %d,%d with size %dx%d", x,
        //        y, w, h);
        Paint_DrawRectangle(x, y, x + w, y + h, BLACK, DOT_PIXEL_1X1,
                            DRAW_FILL_1);
        now += 1;
      }
      while (x < 800) {
        x += 160;
        y = 0;
        w = 160;
        h = (eend[HOUR] - 9) * 60 + eend[MINUTE];
        //        ESP_LOGI(TAG, "Drawing rectangle at %d,%d with size %dx%d", x,
        //        y, w, h);
        if (h < 0)
          break;
        Paint_DrawRectangle(x, y, x + w, y + h, BLACK, DOT_PIXEL_1X1,
                            DRAW_FILL_1);
        break;
      }
    }
  }

  // draw a line at current time
  if (current_time.tm_hour >= 9 && current_time.tm_hour <= 17 &&
      0 <= current_time.tm_mday - begin[DAY] &&
      current_time.tm_mday - begin[DAY] < 5) {
    int x = (current_time.tm_mday - begin[DAY]) * 160;
    int y = (current_time.tm_hour - 9) * 60 +
            (current_time.tm_min); // one pixel for each minute
    ESP_LOGI(TAG, "Drawing line at %d,%d", x, y);
    Paint_SelectImage(red_image);
    Paint_Clear(WHITE);
    Paint_DrawLine(0, y, 800, y, RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(x, y, x + 160, y, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    Paint_DrawCircle(x, y, 3, RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawCircle(x + 160, y, 3, RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  }
  EPD_7IN5B_V2_Display(black_image, red_image);
}