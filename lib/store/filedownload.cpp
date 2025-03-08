#include "filedownload.h"

FileDownloader::FileDownloader() {
  // Initialize any class members here
}

bool FileDownloader::begin() {
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    return false;
  }
  return true;
}

bool FileDownloader::downloadFile(const char* url, const char* filename) {
  // Create a new HTTP client
  WiFiClientSecure client;
  
  // Skip certificate verification (for simplicity)
  client.setInsecure();
  
  HTTPClient https;
  Serial.print("Downloading from: ");
  Serial.println(url);
  
  if (!https.begin(client, url)) {
    Serial.println("HTTPS connection failed");
    return false;
  }
  
  int httpCode = https.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.print("HTTP GET failed, error code: ");
    Serial.println(httpCode);
    https.end();
    return false;
  }
  
  // Get the content length
  int contentLength = https.getSize();
  Serial.print("Content length: ");
  Serial.println(contentLength);
  
  // Create or open the file for writing
  File file = LittleFS.open(filename, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    https.end();
    return false;
  }
  
  return downloadToFile(https, file, contentLength);
}

bool FileDownloader::downloadToFile(HTTPClient& https, File& file, size_t contentLength) {
  // Get the HTTP stream
  WiFiClient* stream = https.getStreamPtr();
  
  // Read from HTTP and write to file
  uint8_t buffer[128];
  size_t bytesDownloaded = 0;
  
  while (https.connected() && (bytesDownloaded < contentLength)) {
    size_t bytesAvailable = stream->available();
    
    if (bytesAvailable) {
      size_t bytesToRead = min(bytesAvailable, sizeof(buffer));
      size_t bytesRead = stream->readBytes(buffer, bytesToRead);
      
      if (bytesRead > 0) {
        file.write(buffer, bytesRead);
        bytesDownloaded += bytesRead;
        
        // Print progress
        Serial.print("Downloaded: ");
        Serial.print(bytesDownloaded);
        Serial.print(" of ");
        Serial.println(contentLength);
      }
    }
    delay(1);
  }
  
  file.close();
  Serial.println("File saved to LittleFS");
  https.end();
  return true;
}

// Legacy function to maintain backward compatibility
bool download_file(const char* url, const char* filename) {
  FileDownloader downloader;
  return downloader.downloadFile(url, filename);
}