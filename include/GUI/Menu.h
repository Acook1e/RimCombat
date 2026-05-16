#pragma once

class Menu
{
public:
  static Menu& GetSingleton()
  {
    static Menu singleton;
    return singleton;
  }

private:
  Menu();
  ~Menu();
};
