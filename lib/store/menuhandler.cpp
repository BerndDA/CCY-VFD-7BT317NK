#include "menuhandler.h"
#include "filedownload.h"
#include <Arduino.h>

#define BASE_URL "https://raw.githubusercontent.com/BerndDA/CCY-VFD-7BT317NK/refs/heads/main/assets"
#define DATA_FILENAME "/data.json"

MenuHandler::MenuHandler() : jsonFilename(DATA_FILENAME), currentMenuIndex(0)
{
}

bool MenuHandler::begin()
{
    // Initialize LittleFS if not already initialized
    if (!LittleFS.begin())
    {
        Serial.println("Failed to mount LittleFS");
        return false;
    }

    // Check if JSON file exists, if not download it
    if (!LittleFS.exists(jsonFilename))
    {
        String url = String(BASE_URL) + String(jsonFilename);
        FileDownloader downloader;
        if (!downloader.downloadFile(url.c_str(), jsonFilename))
        {
            Serial.printf("JSON file %s not found and could not be downloaded\n", jsonFilename);
            return false;
        }
    }
    return true;
}

bool MenuHandler::parseJsonFile(JsonDocument &doc)
{
    // Open JSON file for reading
    File jsonFile = LittleFS.open(jsonFilename, "r");
    if (!jsonFile)
    {
        Serial.printf("Failed to open JSON file %s for reading\n", jsonFilename);
        return false;
    }

    // Read the entire file into a string
    String jsonString = jsonFile.readString();
    jsonFile.close();

    // Parse the JSON string
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error)
    {
        Serial.printf("Failed to parse JSON: %s\n", error.c_str());
        return false;
    }

    return true;
}

void MenuHandler::loadMenuItems()
{
    JsonDocument doc;

    // Parse the JSON file
    if (!parseJsonFile(doc))
    {
        return;
    }

    for (JsonVariant item : doc.as<JsonArray>())
    {
        MenuItem menuItem = createMenuItemFromJson(item);
        menuItems.push_back(menuItem);
        if(menuItem.type == "file")
            fileMenuItems.push_back(menuItem);
        // Download the menu file if it doesn't exist
        if (menuItem.type == "file" && !LittleFS.exists(menuItem.file))
        {
            String url = String(BASE_URL) + String("/") + menuItem.file;
            FileDownloader downloader;
            downloader.downloadFile(url.c_str(), menuItem.file.c_str());
        }
    }
}

MenuItem MenuHandler::createMenuItemFromJson(JsonVariant &item)
{
    MenuItem menuItem;
    menuItem.menu = item["menu"].as<String>();

    // Center the menu text
    int filler = (6 - menuItem.menu.length()) / 2;
    for (int i = 0; i < filler; i++)
    {
        menuItem.menu = " " + menuItem.menu;
    }

    menuItem.intro = item["intro"].as<String>();
    menuItem.numrec = item["numrec"];
    menuItem.file = item["menu"].as<String>() + ".txt";
    menuItem.type = item["type"].as<String>();

    return menuItem;
}

String MenuHandler::getRandomRecord(const MenuItem &item)
{
    // If this is a special menu item without a file
    if (item.file.isEmpty() || item.numrec <= 0)
    {
        return item.intro;
    }

    // Check if file exists
    if (!LittleFS.exists(item.file))
    {
        Serial.printf("Output file %s not found\n", item.file.c_str());
        return "";
    }

    // Generate a random segment number between 1 and recordCount
    int randomRecordNum = random(1, item.numrec + 1);
    Serial.printf("Randomly selected record #%d of %d for menu %s\n",
                  randomRecordNum, item.numrec, item.menu.c_str());

    return readRecordFromFile(item, randomRecordNum);
}

String MenuHandler::readRecordFromFile(const MenuItem &item, int recordNum)
{
    // Open the file for reading
    File file = LittleFS.open(item.file, "r");
    if (!file)
    {
        Serial.printf("Failed to open file %s for reading\n", item.file.c_str());
        return "";
    }

    String record = "";
    int currentRecord = 0;

    // Read the file line by line (each line is a record)
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        currentRecord++;

        if (currentRecord == recordNum)
        {
            record = line;
            break;
        }
    }

    file.close();

    // Process the record text
    record.trim();
    record = replaceUmlautsAndSpecialChars(record);

    return item.intro + "      " + record;
}

String MenuHandler::replaceUmlautsAndSpecialChars(const String &text)
{
    String result = text;
    result.replace("ä", "ae");
    result.replace("ö", "oe");
    result.replace("ü", "ue");
    result.replace("Ä", "Ae");
    result.replace("Ö", "Oe");
    result.replace("Ü", "Ue");
    result.replace("ß", "ss");
    return result;
}

// New methods for menu management

void MenuHandler::initializeMenuItems()
{
    // Get the active menu items from JSON
    loadMenuItems();

    // Reset index to beginning
    currentMenuIndex = 0;
}

const std::vector<MenuItem> &MenuHandler::getMenuItems() const
{
    return menuItems;
}

uint8_t MenuHandler::getCurrentMenuIndex() const
{
    return currentMenuIndex;
}

String MenuHandler::scrollToNextItem()
{
    // Move to the next menu item
    currentMenuIndex = (currentMenuIndex + 1) % menuItems.size();
    Serial.println(menuItems.at(currentMenuIndex).menu);
    return menuItems.at(currentMenuIndex).menu;
}

String MenuHandler::selectCurrentItem()
{
    Serial.println("Select Menu Item");

    MenuItem* selectedItem = &menuItems.at(currentMenuIndex);
    if (selectedItem->type == "random")
    {
        selectedItem = &fileMenuItems.at(random(0, fileMenuItems.size()));
    }
    else if (selectedItem->type != "file") // Check for spezial item
    {
        // Execute the special action callback if set
        if (specialActionCallback)
        {
            specialActionCallback(selectedItem->type.c_str());
        }
        return "";
    }

    // Get a random record from the selected file menu item
    return getRandomRecord(*selectedItem);
}

void MenuHandler::setSpecialActionCallback(std::function<void(const char *item)> callback)
{
    specialActionCallback = callback;
}

void MenuHandler::flashCurrentMenuItem()
{
    // This method would be called to flash/blink the current menu item text
    // Implementation would depend on the display logic, but we're providing the interface
    // The actual implementation would interact with the display
    Serial.println("Flashing menu item: " + menuItems[currentMenuIndex].menu);
}