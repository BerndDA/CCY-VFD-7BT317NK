#include "mqtt_manager.h"
#include <ESP8266WiFi.h>
#include <LittleFS.h>

// Constructor with default values
MqttManager::MqttManager(const char *server, int port, const char *inTopic)
    : mqttClient(wifiClient),
      mqttServer(server),
      mqttPort(port),
      mqttInTopic("vfd/" + String(ESP.getChipId(), HEX) + "/" + inTopic),
      mqttOutTopic("vfd/" + String(ESP.getChipId(), HEX) + "/"),
      mqttClientId(String(ESP.getChipId(), HEX)),
      previousConnectionState(false)
{
    // Set default callbacks (empty)
    messageCallback = [](const char *message) {};
    connectionStateCallback = [](bool connected) {};
}

// Initialize MQTT connection
void MqttManager::begin()
{
    // Set server and callback
    mqttClient.setServer(mqttServer.c_str(), mqttPort);

    // Set up callback
    mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length)
                           {
         // Convert payload to null-terminated string
         char message[length + 1];
         memcpy(message, payload, length);
         message[length] = '\0';
         
         // Debug output
         Serial.print("MQTT message arrived [");
         Serial.print(topic);
         Serial.print("]: ");
         Serial.println(message);
         
         // Call the user callback with the message
         if (messageCallback) {
             messageCallback(message);
         } });

    // Try to connect
    reconnect();
}

// Reconnect to MQTT server if connection is lost
void MqttManager::reconnect()
{
    unsigned long now = millis();
    if (now - lastReconnectAttempt < 2000 || !WiFi.isConnected()) {
        // Do nothing if the last attempt was less than 2 seconds ago. 
        // we are inside the runloop and get called every loop
        return;
    }
    lastReconnectAttempt = now;
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqttServer);
    Serial.print(":");
    Serial.print(mqttPort);
    Serial.print("...");

    // Attempt to connect
    if (mqttClient.connect(mqttClientId.c_str(), (mqttOutTopic + "online").c_str(), 0, true, "0"))
    {
        Serial.println("connected");

        // Once connected, publish an announcement
        this->publish("VFD device connected");
        this->publishStatus();
        // Subscribe to the input topic
        mqttClient.subscribe(mqttInTopic.c_str());
        
        // Call the connection state callback if state changed
        if (!previousConnectionState && connectionStateCallback) {
            connectionStateCallback(true);
        }
        previousConnectionState = true;
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" retry in 2 seconds");
        
        // Call the connection state callback if state changed
        if (previousConnectionState && connectionStateCallback) {
            connectionStateCallback(false);
        }
        previousConnectionState = false;
    }
}

// Main loop function to maintain MQTT connection
void MqttManager::loop()
{
    bool currentConnectionState = mqttClient.connected();
    
    // Check for connection state changes
    if (currentConnectionState != previousConnectionState) {
        // Connection state has changed, call the callback
        if (connectionStateCallback) {
            connectionStateCallback(currentConnectionState);
        }
        previousConnectionState = currentConnectionState;
    }
    
    if (!currentConnectionState)
    {
        reconnect();
    }
    mqttClient.loop();
}

// Set message callback
void MqttManager::onMessage(MessageCallback callback)
{
    messageCallback = callback;
}

// Set connection state change callback
void MqttManager::onConnectionStateChange(ConnectionStateCallback callback)
{
    connectionStateCallback = callback;
    
    // Call the callback immediately with the current connection state
    if (callback) {
        bool currentState = mqttClient.connected();
        callback(currentState);
    }
}

// Publish a message to the out topic
bool MqttManager::publish(const char *message)
{
    return publish("lastmsg", message);
}

// Publish a message to a specific topic
bool MqttManager::publish(const char *topic, const char *message, bool retain)
{
    return mqttClient.publish((mqttOutTopic + topic).c_str(), message, retain);
}

// Subscribe to a topic
bool MqttManager::subscribe(const char *topic)
{
    return mqttClient.subscribe(topic);
}

// Check if connected to MQTT server
bool MqttManager::isConnected()
{
    return mqttClient.connected();
}

// Set server configuration
void MqttManager::setServer(const char *server, int port)
{
    mqttServer = server;
    mqttPort = port;
    mqttClient.setServer(mqttServer.c_str(), mqttPort);
}

// Set topic configuration
void MqttManager::setTopics(const char *inTopic, const char *outTopic)
{
    // If already connected, unsubscribe from old topic and subscribe to new one
    if (mqttClient.connected())
    {
        mqttClient.unsubscribe(mqttInTopic.c_str());
    }

    mqttInTopic = inTopic;
    mqttOutTopic = outTopic;

    if (mqttClient.connected())
    {
        mqttClient.subscribe(mqttInTopic.c_str());
    }
}

void MqttManager::publishStatus()
{
    publish("online", "1", true);
    publish("chip-id", String(ESP.getChipId(), HEX).c_str());
    publish("ip", WiFi.localIP().toString().c_str());
    publish("mac", WiFi.macAddress().c_str());
    // flash chip size
    publish("flash-size", String(ESP.getFlashChipSize()).c_str());
    // core version
    publish("core-version", ESP.getCoreVersion().c_str());
    // sdk version
    publish("sdk-version", ESP.getSdkVersion());
    // reset reason
    publish("reset-reason", ESP.getResetReason().c_str());
    // CPU frequency
    publish("cpu-frequency", String(ESP.getCpuFreqMHz()).c_str());
    // get fsinfo from littlefs
    FSInfo fs_info;
    LittleFS.info(fs_info);
    publish("fs-total", String(fs_info.totalBytes).c_str());
    publish("fs-used", String(fs_info.usedBytes).c_str());
    publishDynamic();
}

void MqttManager::publishDynamic()
{
    // free heap
    publish("free-heap", String(ESP.getFreeHeap()).c_str());
    // heap fragmentation
    publish("heap-frag", String(ESP.getHeapFragmentation()).c_str());
    // stack
    publish("free-stack", String(ESP.getFreeContStack()).c_str());
}