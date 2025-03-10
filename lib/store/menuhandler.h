#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>
#include <functional>

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

  // Get the current menu items list
  const std::vector<MenuItem>& getMenuItems() const;

  // Initialize the menu items vector with special items
  void initializeMenuItems();
  
  // Scroll to the next menu item and return the new item's text
  String scrollToNextItem();
  
  // Select and process the current menu item
  String selectCurrentItem();
  
  // Get current menu item index
  uint8_t getCurrentMenuIndex() const;
  
  // Set callback for special menu actions
  void setSpecialActionCallback(std::function<void(const char* item)> callback);
  
  // Flash/blink the current menu item text (for visual feedback)
  void flashCurrentMenuItem();

private:
  const char* jsonFilename; // JSON filename
  std::vector<MenuItem> menuItems; // Menu items vector
  uint8_t currentMenuIndex; // Current selected menu index
  uint8_t fileItems; // Number of items from config file
  std::function<void(const char* item)> specialActionCallback; // Callback for special actions
  
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