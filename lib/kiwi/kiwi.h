#ifndef BASE64_DECODER_H
#define BASE64_DECODER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>

class Kiwi
{
public:
  Kiwi();
  ~Kiwi();

  bool begin();
  bool processApiData(const char *apiUrl);
  bool isDataAvailable();
  void clearFiles();

  // Method to get total bytes written
  size_t getTotalBytesWritten() const { return totalBytesWritten; }

  // Set the output filename
  void setOutputFilename(const char *filename) { outputFilename = filename; }

private:
  // Buffer sizes
  static const int base64BufferSize = 512;
  static const int decodedBufferSize = 384;

  // Working buffer for accumulating decoded data until we find a separator
  static const int maxWorkingBufferSize = 1024; // 1KB buffer
  uint8_t *workingBuffer;
  size_t workingBufferPos;

  // File handling
  uint16_t segmentCount;
  const char *outputFilename = "/kiwi.txt"; // Default output filename
  const char *jsonFilename = "/data.json";
  File outputFile;

  // Track total bytes written to filesystem
  size_t totalBytesWritten;

  // Buffers
  char base64Buffer[base64BufferSize + 1];   // +1 for null terminator
  char decodedBuffer[decodedBufferSize + 1]; // +1 for null terminator

  // Separator character
  static const char SEPARATOR = '\xA7';

  void processStream(WiFiClient *stream);
  void processDecodedData(char *data, int length);
  bool openOutputFile();
  void writeSegmentToFile();
  void closeOutputFile();
  void loadMetadata(); 
  void updateJsonFile(uint16_t segmentCount); 
};

#endif