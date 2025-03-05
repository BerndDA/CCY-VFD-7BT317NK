#include "base64_decoder.h"
#include "base64.hpp" // Include your custom base64 utilities

Base64Decoder::Base64Decoder() : workingBuffer(NULL),
                                 workingBufferPos(0),
                                 segmentCount(0),
                                 totalBytesWritten(0)
{
}

Base64Decoder::~Base64Decoder()
{
  if (workingBuffer != NULL)
  {
    free(workingBuffer);
    workingBuffer = NULL;
  }
  
  // Ensure the file is closed
  if (outputFile)
  {
    outputFile.close();
  }
}

bool Base64Decoder::begin()
{
  // Reset total bytes written
  totalBytesWritten = 0;
  segmentCount = 0;
  
  // Allocate working buffer
  workingBuffer = (uint8_t *)malloc(maxWorkingBufferSize);
  if (workingBuffer == NULL)
  {
    Serial.println("Failed to allocate working buffer!");
    return false;
  }

  // Initialize LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("Failed to mount LittleFS");
    return false;
  }

  // Open the output file
  if (!openOutputFile())
  {
    return false;
  }

  return true;
}

bool Base64Decoder::openOutputFile()
{
  // Open file for writing (will create or truncate existing file)
  outputFile = LittleFS.open(outputFilename, "w");
  if (!outputFile)
  {
    Serial.printf("Failed to open output file %s for writing\n", outputFilename);
    return false;
  }
  
  Serial.printf("Opened output file: %s\n", outputFilename);
  return true;
}

void Base64Decoder::closeOutputFile()
{
  if (outputFile)
  {
    outputFile.close();
    Serial.printf("Closed output file: %s\n", outputFilename);
    
    // Save metadata after closing the file
    saveMetadata();
  }
}

// New method to save metadata (segment count)
void Base64Decoder::saveMetadata()
{
  File metadataFile = LittleFS.open(metadataFilename, "w");
  if (!metadataFile)
  {
    Serial.printf("Failed to open metadata file %s for writing\n", metadataFilename);
    return;
  }
  
  metadataFile.println(segmentCount);
  metadataFile.close();
  
  Serial.printf("Saved metadata: %d segments\n", segmentCount);
}

// New method to load metadata (segment count)
void Base64Decoder::loadMetadata()
{
  if (!LittleFS.exists(metadataFilename))
  {
    Serial.printf("Metadata file %s not found, using current segment count: %d\n", 
                 metadataFilename, segmentCount);
    return;
  }
  
  File metadataFile = LittleFS.open(metadataFilename, "r");
  if (!metadataFile)
  {
    Serial.printf("Failed to open metadata file %s for reading\n", metadataFilename);
    return;
  }
  
  String countStr = metadataFile.readStringUntil('\n');
  metadataFile.close();
  
  segmentCount = countStr.toInt();
  Serial.printf("Loaded metadata: %d segments\n", segmentCount);
}

bool Base64Decoder::processApiData(const char *apiUrl)
{
  this->begin();
  WiFiClientSecure client;
  HTTPClient http;

  // Skip SSL certificate verification
  client.setInsecure();

  Serial.println("Making API request...");

  // Make the request
  http.begin(client, apiUrl);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    Serial.printf("HTTP response code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK)
    {
      // Get the response stream
      WiFiClient *stream = http.getStreamPtr();

      // Reset working buffer position
      workingBufferPos = 0;

      // Process the incoming data in chunks
      Serial.println("Starting to process stream...");
      processStream(stream);

      // Process any remaining data in the working buffer
      if (workingBufferPos > 0)
      {
        writeSegmentToFile();
      }

      // Close the output file
      closeOutputFile();

      Serial.printf("Stream processing complete! Total bytes written: %u KB\n", totalBytesWritten / 1024);
      Serial.printf("Total segments written: %d\n", segmentCount);
      
      http.end();
      if (workingBuffer != NULL)
      {
        free(workingBuffer);
        workingBuffer = NULL;
      }
      return true;
    }
  }
  else
  {
    Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
    Serial.println(httpCode);
  }

  // Close the output file
  closeOutputFile();

  http.end();
  if (workingBuffer != NULL)
  {
    free(workingBuffer);
    workingBuffer = NULL;
  }
  return false;
}

bool Base64Decoder::isDataAvailable()
{
  this->loadMetadata();
  return segmentCount > 0;
}

void Base64Decoder::processStream(WiFiClient *stream)
{
  int base64Pos = 0;
  int totalProcessed = 0;
  bool inChunk = false;
  int chunkSize = 0;
  int bytesReadInChunk = 0;
  char hexBuffer[10] = {0}; // Buffer to hold chunk size in hex
  int hexPos = 0;

  // While there's data available from the stream
  while (stream->available() > 0 || base64Pos > 0)
  {
    if (!inChunk)
    {
      // We're at the start of a new chunk or the beginning of the response
      // Read the chunk size (in hex)
      while (stream->available() > 0 && hexPos < 9) // Limit to prevent overflow
      {
        char c = stream->read();

        // Chunk size is terminated by CRLF
        if (c == '\r')
        {
          // Expect LF next
          if (stream->available() > 0)
          {
            char lf = stream->read();
            if (lf != '\n')
            {
              Serial.println("Error: Expected LF after CR in chunk header");
            }
          }
          break;
        }

        // Store hex characters
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
        {
          hexBuffer[hexPos++] = c;
        }
      }

      // Null terminate the hex buffer
      hexBuffer[hexPos] = '\0';

      // Convert hex string to integer
      chunkSize = strtol(hexBuffer, NULL, 16);
      Serial.printf("Chunk size: %d bytes (0x%s)\n", chunkSize, hexBuffer);

      // Reset for next chunk
      hexPos = 0;
      memset(hexBuffer, 0, sizeof(hexBuffer));

      // If chunk size is 0, we've reached the end of the response
      if (chunkSize == 0)
      {
        // Skip the final CRLF after the zero chunk
        if (stream->available() >= 2)
        {
          stream->read(); // CR
          stream->read(); // LF
        }
        break;
      }

      inChunk = true;
      bytesReadInChunk = 0;
    }

    // We're inside a chunk, read data
    while (inChunk && stream->available() > 0 && bytesReadInChunk < chunkSize)
    {
      // Read one byte from the chunk
      char c = stream->read();
      bytesReadInChunk++;

      // Add to our base64 buffer if there's space
      if (base64Pos < base64BufferSize)
      {
        base64Buffer[base64Pos++] = c;
      }
      else
      {
        // Process the full buffer if it's full
        base64Buffer[base64Pos] = '\0';

        // Process complete base64 chunks (must be multiple of 4)
        int processLength = (base64Pos / 4) * 4;
        if (processLength > 0)
        {
          // Temporarily null terminate at the process length
          char originalChar = base64Buffer[processLength];
          base64Buffer[processLength] = '\0';

          // Decode the chunk using the provided b64_decode function
          int decodedLength = decode_base64((const unsigned char *)base64Buffer, processLength, (unsigned char *)decodedBuffer);

          // Restore the original character
          base64Buffer[processLength] = originalChar;

          // Process the decoded data to find separators
          processDecodedData(decodedBuffer, decodedLength);

          // Move remaining data to the beginning of the buffer
          memmove(base64Buffer, base64Buffer + processLength, base64Pos - processLength);
          base64Pos -= processLength;

          totalProcessed += processLength;
        }

        // Now we have space again, add the current byte
        base64Buffer[base64Pos++] = c;
      }

      // If we've read the entire chunk, look for the trailing CRLF
      if (bytesReadInChunk >= chunkSize)
      {
        // Expect CRLF after chunk data
        if (stream->available() >= 2)
        {
          char cr = stream->read();
          char lf = stream->read();
          if (cr != '\r' || lf != '\n')
          {
            Serial.println("Error: Expected CRLF after chunk data");
          }
        }

        inChunk = false; // Done with this chunk
        break;
      }
    }

    // Process the base64 buffer if we're not in a chunk or no more data is immediately available
    if ((!inChunk || !stream->available()) && base64Pos > 0)
    {
      base64Buffer[base64Pos] = '\0';

      // Process complete base64 chunks (must be multiple of 4)
      int processLength = (base64Pos / 4) * 4;
      if (processLength > 0)
      {
        // Temporarily null terminate at the process length
        char originalChar = base64Buffer[processLength];
        base64Buffer[processLength] = '\0';

        // Decode the chunk
        int decodedLength = decode_base64((const unsigned char *)base64Buffer, processLength, (unsigned char *)decodedBuffer);

        // Restore the original character
        base64Buffer[processLength] = originalChar;

        // Process the decoded data
        processDecodedData(decodedBuffer, decodedLength);

        // Move remaining data to the beginning of the buffer
        memmove(base64Buffer, base64Buffer + processLength, base64Pos - processLength);
        base64Pos -= processLength;

        totalProcessed += processLength;

        // Print progress every ~10KB of base64 data
        if (totalProcessed % 10240 < 512)
        {
          Serial.printf("Processed %d bytes of base64 data, written %u KB to filesystem\n", 
                        totalProcessed, totalBytesWritten / 1024);
        }
      }
    }

    // Give the system some time to breathe
    yield();
  }

  // Process any remaining base64 data (may need padding)
  if (base64Pos > 0)
  {
    // Add padding if needed
    while (base64Pos % 4 != 0)
    {
      if (base64Pos < base64BufferSize)
      {
        base64Buffer[base64Pos++] = '=';
      }
    }
    base64Buffer[base64Pos] = '\0';

    // Decode final chunk
    int decodedLength = decode_base64((const unsigned char *)base64Buffer, base64Pos, (unsigned char *)decodedBuffer);
    processDecodedData(decodedBuffer, decodedLength);
  }

  Serial.printf("Total processed: %d bytes of base64 data\n", totalProcessed);
  Serial.printf("Total written to filesystem: %u KB\n", totalBytesWritten / 1024);
  Serial.printf("Total segments written: %d\n", segmentCount);
}

void Base64Decoder::processDecodedData(char *data, int length)
{
  // Process each byte in the decoded data
  for (int i = 0; i < length; i++)
  {
    // Check if we've found a separator
    if (data[i] == SEPARATOR)
    {
      // Write the current buffer to the file (excluding the separator)
      writeSegmentToFile();

      // Reset the working buffer position for the next segment
      workingBufferPos = 0;
    }
    else
    {
      // Add the byte to the working buffer if there's space
      if (workingBufferPos < maxWorkingBufferSize)
      {
        workingBuffer[workingBufferPos++] = data[i];
      }
      else
      {
        // Buffer overflow - write current buffer and reset
        Serial.println("Warning: Working buffer overflow. Writing current segment.");
        writeSegmentToFile();
        workingBufferPos = 0;

        // Now add the current byte
        workingBuffer[workingBufferPos++] = data[i];
      }
    }
  }
}

void Base64Decoder::writeSegmentToFile()
{
  if (workingBufferPos == 0 || !outputFile)
  {
    return; // Nothing to write or file not open
  }

  // Write the segment data
  size_t bytesWritten = outputFile.write(workingBuffer, workingBufferPos);
  
  // Write a newline character after the segment
  if (bytesWritten == workingBufferPos) {
    outputFile.write('\n');
    bytesWritten++; // Include the newline in the count
  }

  if (bytesWritten != workingBufferPos + 1) // +1 for newline
  {
    Serial.println("Error: File write incomplete");
  }
  else
  {
    totalBytesWritten += bytesWritten;
    segmentCount++;
    Serial.printf("Wrote segment #%d, size: %d bytes\n", segmentCount, workingBufferPos);
    Serial.printf("Total bytes written: %u KB\n", totalBytesWritten / 1024);
  }
}

void Base64Decoder::clearFiles()
{
  Serial.println("Clearing output file...");
  // Close file if it's open
  if (outputFile)
  {
    outputFile.close();
  }
  
  // Remove the output file
  if (LittleFS.exists(outputFilename))
  {
    LittleFS.remove(outputFilename);
    Serial.printf("Removed file: %s\n", outputFilename);
  }
  
  // Remove the metadata file
  if (LittleFS.exists(metadataFilename))
  {
    LittleFS.remove(metadataFilename);
    Serial.printf("Removed file: %s\n", metadataFilename);
  }
  
  Serial.println("Files cleared!");
  
  // Reset counters
  totalBytesWritten = 0;
  segmentCount = 0;
}

String Base64Decoder::getRandomSegment()
{
  // Load metadata to ensure we have the latest segment count
  loadMetadata();
  
  // Return empty string if no segments or file doesn't exist
  if (segmentCount == 0 || !LittleFS.exists(outputFilename))
  {
    Serial.println("No segments available to select");
    return "";
  }
  
  // Generate a random segment number between 1 and segmentCount
  int randomSegmentNum = random(1, segmentCount + 1);
  Serial.printf("Randomly selected segment #%d of %d\n", randomSegmentNum, segmentCount);
  
  // Open the file for reading
  File file = LittleFS.open(outputFilename, "r");
  if (!file)
  {
    Serial.printf("Failed to open file %s for reading\n", outputFilename);
    return "";
  }
  
  String segment = "";
  int currentSegment = 0;
  
  // Read the file line by line (each line is a segment)
  while (file.available())
  {
    String line = file.readStringUntil('\n');
    currentSegment++;
    
    if (currentSegment == randomSegmentNum)
    {
      segment = line;
      break;
    }
  }
  
  file.close();
  
  // Trim any trailing whitespace or newline characters
  segment.trim();
  
  Serial.printf("Retrieved segment of length %d bytes\n", segment.length());
  return segment;
}