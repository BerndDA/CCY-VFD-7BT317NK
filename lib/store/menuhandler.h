#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>

// Structure to hold menu item information
struct MenuItem {
  String menu;   // Menu identifier
  String intro;  // Introduction text
  String file;   // Associated file
  int numrec;    // Number of records
};

// Class for handling menu operations
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
  const char* jsonFilename; // JSON filename
  
  // Helper method to parse JSON file
  bool parseJsonFile(JsonDocument& doc);
  
  // Create a MenuItem object from JSON data
  MenuItem createMenuItemFromJson(JsonVariant& item);
  
  // Read a specific record from a file
  String readRecordFromFile(const MenuItem& item, int recordNum);
  
  // Replace special characters with ASCII equivalents
  String replaceUmlautsAndSpecialChars(const String& text);
};

#endif // MENU_HANDLER_H