#include "base64_decoder.h"
#include "base64.hpp" // Include your custom base64 utilities

Base64Decoder::Base64Decoder() : workingBuffer(NULL),
                                 workingBufferPos(0),
                                 currentFileIndex(0)
{
}

Base64Decoder::~Base64Decoder()
{
  if (workingBuffer != NULL)
  {
    free(workingBuffer);
    workingBuffer = NULL;
  }
}

bool Base64Decoder::begin()
{
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

  return true;
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
        saveBufferToFile();
      }

      Serial.println("Stream processing complete!");
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

  http.end();
  if (workingBuffer != NULL)
  {
    free(workingBuffer);
    workingBuffer = NULL;
  }
  return false;
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
          Serial.printf("Processed %d bytes of base64 data\n", totalProcessed);
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
}

void Base64Decoder::processDecodedData(char *data, int length)
{
  // Process each byte in the decoded data
  for (int i = 0; i < length; i++)
  {
    // Check if we've found a separator
    if (data[i] == SEPARATOR)
    {
      // Save the current buffer to a file (excluding the separator)
      saveBufferToFile();

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
        // Buffer overflow - save current buffer and reset
        Serial.println("Warning: Working buffer overflow. Saving current segment.");
        saveBufferToFile();
        workingBufferPos = 0;

        // Now add the current byte
        workingBuffer[workingBufferPos++] = data[i];
      }
    }
  }
}

void Base64Decoder::saveBufferToFile()
{
  if (workingBufferPos == 0)
  {
    return; // Nothing to save
  }

  // Create a filename for this segment
  char filename[32];
  sprintf(filename, "/segment_%03d.dat", currentFileIndex++);

  // Open file for writing
  File file = LittleFS.open(filename, "w");
  if (!file)
  {
    Serial.printf("Failed to open file %s for writing\n", filename);
    return;
  }

  // Write data to file
  size_t bytesWritten = file.write(workingBuffer, workingBufferPos);
  file.close();

  // read file as string and print
  // file = LittleFS.open(filename, "r");
  // String fileContent = file.readString();
  // Serial.println(fileContent);
  // file.close();

  if (bytesWritten != workingBufferPos)
  {
    Serial.println("Error: File write incomplete");
  }
  else
  {
    Serial.printf("Saved segment to %s, size: %d bytes\n", filename, bytesWritten);
  }
}

void Base64Decoder::listFiles()
{
  Serial.println("Files in LittleFS:");
  Dir dir = LittleFS.openDir("/");
  while (dir.next())
  {
    Serial.printf(" - %s, size: %d bytes\n", dir.fileName().c_str(), dir.fileSize());
  }
}

void Base64Decoder::clearFiles()
{
  Serial.println("Clearing files in LittleFS...");
  Dir dir = LittleFS.openDir("/");
  while (dir.next())
  {
    // delete if filename starts with "segment_"
    if (dir.fileName().startsWith("/segment_"))
      LittleFS.remove(dir.fileName());
  }
  Serial.println("Files cleared!");
}
