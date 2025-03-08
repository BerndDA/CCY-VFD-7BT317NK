#ifndef FILE_DOWNLOADER_H
#define FILE_DOWNLOADER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Class for handling file downloads from the internet
class FileDownloader {
public:
  FileDownloader();
  
  // Initialize the file system
  bool begin();
  
  // Download a file from URL and save it to the specified filename
  bool downloadFile(const char* url, const char* filename);
  
private:
  // Helper function to download content to a file
  bool downloadToFile(HTTPClient& https, File& file, size_t contentLength);
};

// Legacy function to maintain backward compatibility
bool download_file(const char* url, const char* filename);

#endif // FILE_DOWNLOADER_H