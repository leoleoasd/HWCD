#include "display.h"

#include "ical.h"
#include <iostream>

// 800 x 480
// Single cell:
// 160 x 40
constexpr size_t image_size = (EPD_7IN5B_V2_WIDTH / 8) * EPD_7IN5B_V2_HEIGHT;

uint8_t *black_image;
uint8_t *red_image;

static const char* TAG = "Ecal display";
using std::cout;

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
  EPD_7IN5B_V2_Clear();
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
  // get first day of this week
  timeinfo.tm_mday -= timeinfo.tm_wday;
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
  end[DAY] += 7;


  Paint_SelectImage(black_image);
  Paint_Clear(WHITE);
  Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
  Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
  Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
  Paint_DrawPoint(10, 110, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
  Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
  Paint_DrawLine(70, 70, 20 , 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
  Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(20, 170, 70, 220, BLACK, DOT_PIXEL_1X1, DRAW_FILL_1);
  Paint_DrawRectangle(80, 170, 130, 220, BLACK, DOT_PIXEL_1X1, DRAW_FILL_2);
  Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
  Paint_DrawString_CN(130, 20, "微雪电子", &Font24CN, WHITE, BLACK);
  Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);

  //2.Draw red image
  Paint_SelectImage(red_image);
  Paint_Clear(WHITE);
  Paint_DrawCircle(160, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  Paint_DrawCircle(210, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED1);
  Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED2);
  Paint_DrawString_CN(130, 0, "你好abc", &Font12CN, BLACK, WHITE);
  Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);
  Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);

  EPD_7IN5B_V2_Display(black_image, red_image);
  DEV_Delay_ms(2000);

  Event* CurrentEvent;
  ICalendar::Query SearchQuery(&*calendar);

  SearchQuery.Criteria.From = begin;
  SearchQuery.Criteria.To = end;
  cout<<"Searching for events between "<<begin<<" and "<<end<<endl;
  SearchQuery.ResetPosition();

  while(CurrentEvent = SearchQuery.GetNextEvent(), CurrentEvent != nullptr) {
    cout << CurrentEvent->UID << endl;
    cout << CurrentEvent->DtStart.Format() << endl;
    cout << CurrentEvent->DtEnd.Format() << endl;
    cout << CurrentEvent->Summary << endl;
    cout << CurrentEvent->Categories << endl;
    cout << "\t" << CurrentEvent->RRule.Freq << endl;
    cout << "\t" << CurrentEvent->RRule.Interval << endl;
    cout << "\t" << CurrentEvent->RRule.Until << endl << endl;
    // CurrentEvent->BaseEvent->Description = "aasfjanfkjahsf";
    // Calendar.ModifyEvent(CurrentEvent->BaseEvent);
  }
}