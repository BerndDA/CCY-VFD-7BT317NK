#ifndef BASE64_DECODER_H
#define BASE64_DECODER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>

class Base64Decoder
{
public:
  Base64Decoder();
  ~Base64Decoder();

  bool begin();
  bool processApiData(const char *apiUrl);
  bool isDataAvailable();
  void clearFiles();

  // Method to get total bytes written
  size_t getTotalBytesWritten() const { return totalBytesWritten; }

  // Set the output filename
  void setOutputFilename(const char *filename) { outputFilename = filename; }

  // Method to randomly select a segment and return it as a string
  String getRandomSegment();

private:
  // Buffer sizes
  static const int base64BufferSize = 512;
  static const int decodedBufferSize = 384;

  // Working buffer for accumulating decoded data until we find a separator
  static const int maxWorkingBufferSize = 1024; // 1KB buffer
  uint8_t *workingBuffer;
  int workingBufferPos;

  // File handling
  int segmentCount;
  const char *outputFilename = "/output.dat"; // Default output filename
  const char *metadataFilename = "/metadata.txt"; // File to store segment count
  File outputFile;

  // Track total bytes written to filesystem
  size_t totalBytesWritten;

  // Buffers
  char base64Buffer[base64BufferSize + 1];   // +1 for null terminator
  char decodedBuffer[decodedBufferSize + 1]; // +1 for null terminator

  // Separator character
  static const char SEPARATOR = '§';

  void processStream(WiFiClient *stream);
  void processDecodedData(char *data, int length);
  bool openOutputFile();
  void writeSegmentToFile();
  void closeOutputFile();
  void saveMetadata(); // New method to save segment count to metadata file
  void loadMetadata(); // New method to load segment count from metadata file
};

#endif