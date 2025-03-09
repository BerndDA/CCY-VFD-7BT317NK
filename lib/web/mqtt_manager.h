 #ifndef MQTT_MANAGER_H
 #define MQTT_MANAGER_H
 
 #include <Arduino.h>
 #include <PubSubClient.h>
 #include <WiFiClient.h>
 #include <functional>
 #include <vector>
 
 class MqttManager {
 public:
     // Constructor with default values
     MqttManager(const char* server, 
                 int port, 
                 const char* inTopic = "text/set");
     
     // Initialize MQTT connection
     void begin();
     
     // Reconnect to MQTT server if connection is lost
     void reconnect();
     
     // Main loop function to maintain MQTT connection
     void loop();
     
     // Set message callback
     typedef std::function<void(const char*)> MessageCallback;
     void onMessage(MessageCallback callback);
     
     // Publish a message to the out topic
     bool publish(const char* message);
     
     // Publish a message to a specific topic
     bool publish(const char* topic, const char* message, bool retain = false);

     // publish status values
     void publishStatus();

     // publish dynamic values
     void publishDynamic();
     
     // Subscribe to a topic
     bool subscribe(const char* topic);
     
     // Check if connected to MQTT server
     bool isConnected();
     
     // Set server configuration
     void setServer(const char* server, int port);
     
     // Set topic configuration
     void setTopics(const char* inTopic, const char* outTopic);
 
 private:
     WiFiClient wifiClient;
     PubSubClient mqttClient;
     
     String mqttServer;
     int mqttPort;
     String mqttInTopic;
     String mqttOutTopic;
     String mqttClientId;
     
     MessageCallback messageCallback;
 };
 
 #endif // MQTT_MANAGER_H