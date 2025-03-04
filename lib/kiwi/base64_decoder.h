#ifndef BASE64_DECODER_H
#define BASE64_DECODER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>


class Base64Decoder {
  public:
    Base64Decoder();
    ~Base64Decoder();
    
    bool begin();
    bool processApiData(const char* apiUrl);
    void listFiles();
    void clearFiles();
    
  private:
    // Buffer sizes
    static const int base64BufferSize = 512;
    static const int decodedBufferSize = 384;
    
    // Working buffer for accumulating decoded data until we find a separator
    static const int maxWorkingBufferSize = 1024; // 1KB buffer
    uint8_t* workingBuffer;
    int workingBufferPos;
    
    // File handling
    int currentFileIndex;
    
    // Buffers
    char base64Buffer[base64BufferSize + 1]; // +1 for null terminator
    char decodedBuffer[decodedBufferSize + 1]; // +1 for null terminator
    
    // Separator character
    static const char SEPARATOR = '§';
    
    void processStream(WiFiClient* stream);
    void processDecodedData(char* data, int length);
    void saveBufferToFile();
};

#endif