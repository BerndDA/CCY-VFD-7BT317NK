#include "menuhandler.h"
#include "filedownload.h"
#include <Arduino.h>

#define BASE_URL "https://raw.githubusercontent.com/BerndDA/CCY-VFD-7BT317NK/refs/heads/main/assets"

MenuHandler::MenuHandler()
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
    // Check if JSON file exists
    if (!LittleFS.exists(jsonFilename))
    {
        if (!download_file((String(BASE_URL) + String(jsonFilename)).c_str(), jsonFilename))
        {
            Serial.printf("JSON file %s not found\n", jsonFilename);
            return false;
        }
    }
    return true;
}

bool MenuHandler::parseJsonFile(DynamicJsonDocument &doc)
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

std::vector<MenuItem> MenuHandler::getActiveMenuItems()
{
    std::vector<MenuItem> activeItems;

    // Allocate a buffer for the JSON document
    const size_t capacity = JSON_ARRAY_SIZE(10) + 10 * JSON_OBJECT_SIZE(3) + 256;
    DynamicJsonDocument doc(capacity);

    // Parse the JSON file
    if (!parseJsonFile(doc))
    {
        return activeItems; // Return empty vector on error
    }

    for (JsonVariant item : doc.as<JsonArray>())
    {
        int numrec = item["numrec"];

        MenuItem menuItem;
        menuItem.menu = item["menu"].as<String>();
        int filler = (6 - menuItem.menu.length()) / 2;
        for (int i = 0; i < filler; i++)
        {
            menuItem.menu = " " + menuItem.menu;
        }
        menuItem.intro = item["intro"].as<String>();
        menuItem.numrec = numrec;
        menuItem.file = item["menu"].as<String>() + ".txt";
        activeItems.push_back(menuItem);
        if(!LittleFS.exists(menuItem.file)){
            download_file((String(BASE_URL) + String("/") + menuItem.file).c_str(), menuItem.file.c_str());
        }
    }
    return activeItems;
}

String MenuHandler::getOutputFilename(const String &menuId)
{
    String filename = String("/") + menuId + ".txt";
    return filename;
}

String MenuHandler::getRandomRecord(const MenuItem &item)
{
    // Get output filename for this menu item
    String outputFilename = item.file;

    // Check if file exists
    if (!LittleFS.exists(outputFilename))
    {
        Serial.printf("Output file %s not found\n", outputFilename.c_str());
        return "";
    }

    // Get the number of records for this menu item
    int recordCount = item.numrec;

    // Return empty string if no records
    if (recordCount <= 0)
    {
        Serial.printf("No records available for menu %s\n", item.menu.c_str());
        return "";
    }

    // Generate a random segment number between 1 and recordCount
    int randomRecordNum = random(1, recordCount + 1);
    Serial.printf("Randomly selected record #%d of %d for menu %s\n",
                  randomRecordNum, recordCount, item.menu.c_str());

    // Open the file for reading
    File file = LittleFS.open(outputFilename, "r");
    if (!file)
    {
        Serial.printf("Failed to open file %s for reading\n", outputFilename.c_str());
        return "";
    }

    String record = "";
    int currentRecord = 0;

    // Read the file line by line (each line is a record)
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        currentRecord++;

        if (currentRecord == randomRecordNum)
        {
            record = line;
            break;
        }
    }

    file.close();

    // Trim any trailing whitespace or newline characters
    record.trim();
    record.replace("ä", "ae");
    record.replace("ö", "oe");
    record.replace("ü", "ue");
    record.replace("Ä", "Ae");
    record.replace("Ö", "Oe");
    record.replace("Ü", "Ue");
    record.replace("ß", "ss");
    return item.intro + "      " + record;
}