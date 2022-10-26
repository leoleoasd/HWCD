#include "menu.h"
#include "gesture.h"

std::vector<Menu> menus;

static const char *TAG = "Ecal menu cpp";
size_t menu_count = 0;
RTC_SLOW_ATTR size_t global_selecting = -1;
RTC_SLOW_ATTR size_t this_selecting = -1;
void init_menu() {
  menus.reserve(30);
  auto root = Menu("root");
  menus.push_back(root);
  menus[0].id = 0;
  auto a = menus[0].addChild("a", []() { ESP_LOGI(TAG, "a"); });
  auto b = menus[0].addChild("b", []() { ESP_LOGI(TAG, "b"); });
  auto c = menus[0].addChild("c", []() { ESP_LOGI(TAG, "c"); });
  auto a1 = menus[a].addChild("a1", []() { ESP_LOGI(TAG, "a1"); });
  auto a2 = menus[a].addChild("a2", []() { ESP_LOGI(TAG, "a2"); });
  auto b1 = menus[b].addChild("b1", []() { ESP_LOGI(TAG, "b1"); });
  auto b2 = menus[b].addChild("b2", []() { ESP_LOGI(TAG, "b2"); });
  auto c1 = menus[c].addChild("c1", []() { ESP_LOGI(TAG, "c1"); });
  auto c2 = menus[c].addChild("c2", []() { ESP_LOGI(TAG, "c2"); });
  auto d = menus[0].addChild("d", []() { ESP_LOGI(TAG, "d"); });

  for(const auto &i: menus) {
    ESP_LOGI(TAG, "id: %d, name: %s, next: %d, prev: %d, child: %d, parent: %d", i.id, i.name.c_str(), i.next, i.prev, i.child, i.parent);
  }
}

void menu_down() {
  ESP_LOGI(TAG, "menu_down");
  if (global_selecting == -1) {
    global_selecting = 0;
    menus[global_selecting].enter();
  } else {
    menus[global_selecting].down();
  }
  ESP_LOGI(TAG, "menu_down finish, global_selecting: %d", global_selecting);
  global_selecting != -1 && ESP_LOGI(TAG, "menu_down finish, global_selecting: %s", menus[global_selecting].name.c_str());
}
void menu_up() {
  ESP_LOGI(TAG, "menu_up");
  if (global_selecting == -1) {
    global_selecting = 0;
    menus[global_selecting].enter();
  } else {
    menus[global_selecting].up();
  }
  ESP_LOGI(TAG, "menu_up finish, global_selecting: %d", global_selecting);
  global_selecting != -1 && ESP_LOGI(TAG, "menu_up finish, global_selecting: %s", menus[global_selecting].name.c_str());
}
void menu_enter() {
  ESP_LOGI(TAG, "menu_enter");
  if (global_selecting == -1) {
    ESP_LOGW(TAG, "SHOULDN't HAPPENING");
    global_selecting = 0;
    menus[global_selecting].enter();
  } else {
    if (this_selecting != -1) {
      if (menus[this_selecting].child != -1){
        global_selecting = this_selecting;
        menus[global_selecting].enter();
      } else {
        menus[this_selecting].enter();
      }
    } else {
      ESP_LOGW(TAG, "SHOULDN't HAPPENING");
    }
  }
  ESP_LOGI(TAG, "menu_enter finish, global_selecting: %d", global_selecting);
  global_selecting != -1 && ESP_LOGI(TAG, "menu_enter finish, global_selecting: %s", menus[global_selecting].name.c_str());
}
void menu_leave() {
  ESP_LOGI(TAG, "menu_leave");
  if (global_selecting == -1) {
    ESP_LOGW(TAG, "SHOULDN't HAPPENING");
    global_selecting = 0;
    menus[global_selecting].enter();
  } else {
    menus[global_selecting].leave();
    this_selecting = global_selecting;
    global_selecting = menus[global_selecting].parent;
    if (global_selecting == -1) {
      mode = 0;
    }
  }
  ESP_LOGI(TAG, "menu_leave finish, global_selecting: %d", global_selecting);
  global_selecting != -1 && ESP_LOGI(TAG, "menu_leave finish, global_selecting: %s", menus[global_selecting].name.c_str());
}