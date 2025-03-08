#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>

struct MenuItem {
  String menu;   // Menu identifier
  String intro;  // Introduction text
  String file;
  int numrec;    // Number of records
};

class MenuHandler {
public:
  MenuHandler();
  
  // Initialize the menu handler
  bool begin();
  
  // Get all menu items that have at least one record
  std::vector<MenuItem> getActiveMenuItems();
  
  // Get a random record from a specific menu item
  String getRandomRecord(const MenuItem& item);

private:
  const char* jsonFilename = "/data.json"; // Default JSON filename
  
  // Helper method to parse JSON file
  bool parseJsonFile(DynamicJsonDocument& doc);
  
  // Helper method to get output filename for a menu item
  String getOutputFilename(const String& menuId);
};

#endif // MENU_HANDLER_H