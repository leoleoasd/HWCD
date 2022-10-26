#ifndef HWCD_ECAL_MENU_H_
#define HWCD_ECAL_MENU_H_
#include "Arduino.h"
#include <functional>
#include <utility>
#include <vector>
RTC_SLOW_ATTR extern size_t this_selecting;
class Menu;
extern std::vector<Menu> menus;
class Menu {
public:
  size_t id;
  size_t next;
  size_t prev;
  size_t child;
  size_t parent;
  std::string name;
  std::function<void()> action;

  Menu()= default;

  // should only be used to create root
  explicit Menu(std::string name): name(std::move(name)),child(-1), next(-1), prev(-1), parent(-1){}

  Menu(std::string name, std::function<void()> action): name(std::move(name)), action(std::move(action)),child(-1), next(-1), prev(-1), parent(-1){};

  size_t addChild(std::string new_name, std::function<void()> new_action) {
    auto new_child = Menu(std::move(new_name), std::move(new_action));
    auto child_id = menus.size();
    menus.push_back(std::move(new_child));
    menus[child_id].parent = this->id;
    menus[child_id].id = child_id;
    if (this->child == -1) {
      this->child = child_id;
      menus[child_id].next = child_id;
      menus[child_id].prev = child_id;
    } else {
      auto last_child = menus[this->child].prev;
      menus[child_id].next = this->child;
      menus[child_id].prev = last_child;
      menus[this->child].prev = child_id;
      menus[last_child].next = child_id;
    }
    return child_id;
  }
  void enter() {
    ESP_LOGI("menu.h", "%s: enter", this->name.c_str());
    if (this->child != -1) {
      ESP_LOGI("menu.h", "%s: selecting %s", this->name.c_str(), menus[this->child].name.c_str());
      this_selecting = this->child;
    } else {
      this->action();
    }
  }
  void down() {
    ESP_LOGI("menu.h", "%s: down", this->name.c_str());
    if (this_selecting != -1 && menus[this_selecting].next != -1) {
      this_selecting = menus[this_selecting].next;
      ESP_LOGI("menu.h", "%s: selecting %s", this->name.c_str(), menus[this_selecting].name.c_str());
    }
  }
  void up() {
    ESP_LOGI("menu.h", "%s: up", this->name.c_str());
    if (this_selecting != -1 && menus[this_selecting].prev != -1) {
      this_selecting = menus[this_selecting].prev;
      ESP_LOGI("menu.h", "%s: selecting %s", this->name.c_str(), menus[this_selecting].name.c_str());
    }
  }
  void leave() {
    ESP_LOGI("menu.h", "%s: leave", this->name.c_str());
    this_selecting = -1;
  }
};
void init_menu();
void menu_up();
void menu_down();
void menu_enter();
void menu_leave();

#endif // HWCD_ECAL_MENU_H_
