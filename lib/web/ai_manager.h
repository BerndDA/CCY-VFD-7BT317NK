#ifndef AI_MANAGER_H
#define AI_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Forward declaration
class Animator;

class AiManager {
public:
    // Constructor
    AiManager(Animator* animator, const char* apiKey);
    
    // Destructor
    ~AiManager();
    
    // Initialize the AI manager
    void begin();
    
    // Main process function (call this in your loop)
    void process();
    
    // Start a conversation
    void startConversation(const char* prompt = "hi");
    
    // Is a conversation active?
    bool isActive() const;
    
    // Set callback for when conversation completes
    void onComplete(void (*callback)(const String& message));
    
    // Set callback for when error occurs
    void onError(void (*callback)(const String& errorMessage));
    
private:
    // State machine for the AI conversation process
    enum class State {
        IDLE,
        CREATE_THREAD,
        ADD_MESSAGE,
        CREATE_RUN,
        POLL_RUN,
        GET_MESSAGES,
        COMPLETE,
        ERROR
    };
    
    // Reset the state machine
    void resetState();
    
    // Helper to set common headers
    void setHeaders();
    
    // Handle error with message
    void handleError(const String& errorMessage);
    
    // Process the message (handle character replacements)
    String processMessage(const String& message);
    
    // Instance variables
    Animator* _animator;
    String _apiKey;
    String _assistantId = "asst_JiePRqehNsRO3iqruEcwsoH0";
    State _state = State::IDLE;
    
    WiFiClientSecure* _client = nullptr;
    HTTPClient* _http = nullptr;
    
    String _threadId;
    String _runId;
    String _prompt;
    
    unsigned long _stateStartTime = 0;
    unsigned long _lastPollTime = 0;
    const unsigned long _aiTimeout = 30000; // 30 seconds timeout
    
    void (*_completeCallback)(const String& message) = nullptr;
    void (*_errorCallback)(const String& errorMessage) = nullptr;
};

#endif // AI_MANAGER_H