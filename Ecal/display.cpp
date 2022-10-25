#include "display.h"

#include "ical.h"
#include <sstream>
#include <iomanip>

constexpr size_t image_size = (EPD_7IN5B_V2_WIDTH / 8) * EPD_7IN5B_V2_HEIGHT;

uint8_t *black_image;
uint8_t *red_image;

static const char* TAG = "Ecal display";

void display_init(){
  pinMode(EPD_BUSY_PIN,  INPUT);
  pinMode(EPD_RST_PIN , OUTPUT);
  pinMode(EPD_DC_PIN  , OUTPUT);

  pinMode(EPD_SCK_PIN, OUTPUT);
  pinMode(EPD_MOSI_PIN, OUTPUT);
  pinMode(EPD_CS_PIN , OUTPUT);

  digitalWrite(EPD_CS_PIN , HIGH);
  digitalWrite(EPD_SCK_PIN, LOW);
  EPD_7IN5B_V2_Init();
//  EPD_7IN5B_V2_Clear();
  black_image = (uint8_t *)ps_malloc(image_size);
  red_image = (uint8_t *)ps_malloc(image_size);
  if (black_image == nullptr || red_image == nullptr) {
    ESP_LOGW(TAG, "Failed to allocate memory for image buffers");
    return;
  }


  Paint_NewImage(black_image, EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT , 0, WHITE);
  Paint_NewImage(red_image, EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT , 0, WHITE);
  Paint_SelectImage(black_image);
  Paint_Clear(WHITE);
  Paint_SelectImage(red_image);
  Paint_Clear(WHITE);
}

void display_calendar() {
  tm timeinfo;
  getLocalTime(&timeinfo);
  // get monday of this week
  timeinfo.tm_mday -= (timeinfo.tm_wday - 1);
  if(timeinfo.tm_mday < 0 ){
    timeinfo.tm_mon -= 1;
    if (timeinfo.tm_mon < 0) {
      timeinfo.tm_mon = 11;
      timeinfo.tm_year -= 1;
    }
    timeinfo.tm_mday += Date::DaysInMonthS(timeinfo.tm_mon, timeinfo.tm_year + 1900);
  }
  timeinfo.tm_hour = 0;
  timeinfo.tm_min = 0;
  timeinfo.tm_sec = 0;
  Date begin;
  begin[YEAR] = timeinfo.tm_year + 1900;
  begin[MONTH] = timeinfo.tm_mon + 1;
  begin[DAY] = timeinfo.tm_mday;
  begin[HOUR] = timeinfo.tm_hour;
  begin[MINUTE] = timeinfo.tm_min;
  begin[SECOND] = timeinfo.tm_sec;
  Date end = begin;
  end[DAY] += 5;

  // Screen size: 800 x 480
  // 5 days a week, 8 hours per day, we have: 160 x 60 pixels for each hour
  // we only draw from 9am to 5pm

  Paint_SelectImage(black_image);
  Paint_Clear(WHITE);
  // draw lines
  for(int i = 0; i < 6; i++){
    Paint_DrawLine(i * 160 + (i == 0), 0, i * 160 + (i == 0), 480, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED1);
  }
  for(int i = 0; i < 9; i++){
    Paint_DrawLine(0, i * 60 + (i == 0), 800, i * 60 + (i == 0), BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED1);
  }
  // draw each event
  Event*e;
  ICalendar::Query SearchQuery(&*calendar);
  SearchQuery.Criteria.From = begin;
  SearchQuery.Criteria.To = end;
  ESP_LOGI(TAG, "Searching for events between %s and %s", string(begin).c_str(), string(end).c_str());
  SearchQuery.ResetPosition();
  while(e = SearchQuery.GetNextEvent(), e != nullptr) {
//    cout << e->UID << endl;
//    cout << e->DtStart.Format() << endl;
//    cout << e->DtEnd.Format() << endl;
//    cout << e->Summary << endl;
//    cout << e->Categories << endl;
//    cout << "\t" << e->RRule.Freq << endl;
//    cout << "\t" << e->RRule.Interval << endl;
//    cout << "\t" << e->RRule.Until << endl << endl;
    ESP_LOGI(TAG, "Event: %s", e->UID.c_str());
    ESP_LOGI(TAG, "Start: %s", e->DtStart.Format().c_str());
    ESP_LOGI(TAG, "End: %s", e->DtEnd.Format().c_str());
    ESP_LOGI(TAG, "Summary: %s", e->Summary.c_str());

    // if begin and ends at the same day
    if(e->DtStart[DAY] == e->DtEnd[DAY]) {
      // draw a rectangle
      int x = (e->DtStart[DAY] - begin[DAY]) * 160;
      x += (x == 0);
      int y = (e->DtStart[HOUR] - 9) * 60 + (e->DtStart[MINUTE]); // one pixel for each minute
      y += (y == 0);
      int w = 160;
      int h = (e->DtEnd[HOUR] - e->DtStart[HOUR]) * 60 + (e->DtEnd[MINUTE] - e->DtStart[MINUTE]); // one pixel for each minute
      if(h < 32) h = 32;
      if(x < 0) {continue;}
      if(y < 0) {continue;}
      if(x > 800) {continue;}
      if(y > 480) {continue;}
      if(x + w > 800) w = 800 - x;
      if(y + h > 480) {
        h = 480 - y;
        if(h < 32) {
          h = 32;
          y = 480 - 32;
        }
      }

      ESP_LOGI(TAG, "Drawing rectangle at %d,%d with size %dx%d", x, y, w, h);
      Paint_DrawRectangle(x, y, x + w, y + h, BLACK, DOT_PIXEL_1X1, DRAW_FILL_1);

      // draw summary
      std::string s = e->Summary;
      // for a width of 160 pixels, we can fit 160 / 7 = 14 characters
      int line = 0;
      int string_line = 0;
      if((line + 1) * 16 <= h) {
        // 14:15 - 16:00
        // TODO: use format
        stringstream ss;
        ss<<setfill('0')<<setw(2);
        ss << e->DtStart[HOUR] << ":" << setfill('0')<<setw(2) << e->DtStart[MINUTE] << " - " << setfill('0')<<setw(2) << e->DtEnd[HOUR] << ":" << setfill('0')<<setw(2) <<  e->DtEnd[MINUTE];
        Paint_DrawString_EN(x + 1, y + 1 + line * 16, ss.str().c_str(), &Font16, BLACK, TRANSPARENT);
        line++;
      }
      while(string_line * 14 < s.length() && (line+1) * 16 <= h) {
        std::string line_str;
        if((line + 2) * 16 > h && (string_line + 1) * 14 < s.length()) {
          line_str = s.substr(string_line * 14, 11);
          line_str += "...";
        } else {
          line_str = s.substr(string_line * 14, 14);
        }
        ESP_LOGI(TAG, "Drawing string at %d,%d: %s", x + 1, y + 1 + line * 16, line_str.c_str());
        Paint_DrawString_EN(x + 1, y + 1 + line * 16, line_str.c_str(), &Font16, BLACK, TRANSPARENT);
        line++;
        string_line++;
      }
    }


    // e->BaseEvent->Description = "aasfjanfkjahsf";
    // Calendar.ModifyEvent(e->BaseEvent);
  }

//  Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
//  Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
//  Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
//  Paint_DrawPoint(10, 110, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
//  Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
//  Paint_DrawLine(70, 70, 20 , 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
//  Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
//  Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
//  Paint_DrawRectangle(20, 170, 70, 220, BLACK, DOT_PIXEL_1X1, DRAW_FILL_1);
//  Paint_DrawRectangle(80, 170, 130, 220, BLACK, DOT_PIXEL_1X1, DRAW_FILL_2);
//  Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
//  Paint_DrawString_CN(130, 20, "微雪电子", &Font24CN, WHITE, BLACK);
//  Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);

//  //2.Draw red image
//  Paint_SelectImage(red_image);
//  Paint_Clear(WHITE);
//  Paint_DrawCircle(160, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
//  Paint_DrawCircle(210, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
//  Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED1);
//  Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED2);
//  Paint_DrawString_CN(130, 0, "你好abc", &Font12CN, BLACK, WHITE);
//  Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);
//  Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);

  EPD_7IN5B_V2_Display(black_image, red_image);
}