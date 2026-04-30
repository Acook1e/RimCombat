#pragma once

class Menu
{
public:
  Menu();
  ~Menu();

  static Menu& GetSingleton()
  {
    static Menu singleton;
    return singleton;
  }

  static void Settings();
  static void Debug();
};
