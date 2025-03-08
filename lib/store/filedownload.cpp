#include <filedownload.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecure.h>

bool download_file(const char* url, const char* filename) {
  // Create a new HTTP client
  WiFiClientSecure client;
  
  // Skip certificate verification (for simplicity)
  // In production, you might want to use a fingerprint or certificate
  client.setInsecure();
  
  HTTPClient https;
  Serial.print("Downloading from: ");
  Serial.println(url);
  
  if (https.begin(client, url)) {
    int httpCode = https.GET();
    
    if (httpCode == HTTP_CODE_OK) {
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
    } else {
      Serial.print("HTTP GET failed, error code: ");
      Serial.println(httpCode);
      https.end();
      return false;
    }
  } else {
    Serial.println("HTTPS connection failed");
    return false;
  }
}