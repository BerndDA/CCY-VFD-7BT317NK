/***********************************************************************************************
 * Copyright Statement:
 * The copyright of this source code belongs to [saisaiwa].
 *
 * Without the explicit authorization of the copyright owner, no part of this code may be used for commercial purposes, including but not limited to sale, rental, licensing, or publication.
 * It is only for personal learning, research, and non-profit use. If you have other usage needs, please contact
 * [yustart@foxmail.com] for authorization.
 *
 * You are free to modify, use, and distribute this code under the following conditions:
 * - You must retain all content of this copyright statement.
 * - You may not use it for any illegal, abusive, or malicious activities.
 * - Your use should not damage the reputation of any individual, organization, or author.
 *
 * The author does not assume any warranty liability arising from the use of this code. The author is not responsible for any direct or indirect losses caused by the use of this code.
 * Github: https://github.com/ccy-studio/CCY-VFD-7BT317NK
 ***********************************************************************************************/

/*
 * @Description:
 * @Author: chenzedeng
 * @Date: 2023-07-28 21:57:30
 * @LastEditTime: 2023-08-22 16:28:54
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <Ticker.h>
#include <coredecls.h>
#include <gui.h>
#include <web_server.h>
#include <WiFiManager.h>
#include <animator.h>
#include <kiwi.h>
#include <menuhandler.h>
#include <config.h>
#include "mqtt_manager.h"
#include <EEPROM.h>

struct mqtt_config
{
    bool enable;
    char server[40];
    char port[6];
};

void set_key_listener();
IRAM_ATTR void handle_key_interrupt();
IRAM_ATTR void keyISR();
void set_tick();
void task_time_refresh_fun();
void vfd_synchronous();

bool isTimeSet = false;

u8 mh_state = 0;    // Colon display state
u8 light_level = 1; // Brightness level

volatile u8 style_page = STYLE_TIME; // Page display style

// Timer initialization
Ticker task_time_refresh; // Time refresh

WiFiManager wifimanager;
Animator animator;
Kiwi kiwi;
MenuHandler menuhandler;
MqttManager *mqttManager = nullptr;

// key handling
volatile bool keyPressed = false;
volatile unsigned long pressStartTime = 0;
volatile unsigned long lastDebounceTime = 0; // the last time the button state was checked
const unsigned long debounceDelay = 50;      // the debounce time in milliseconds
Ticker task_key_pressed;
bool isLongPress = false; // Tracks whether long press was detected

// MQTT message callback
void handleMqttMessage(const char *message)
{
    vfd_gui_set_pic(PIC_PLAY, true);
    animator.set_text_and_run(message, 210);
}

// Callback function for the update menu item
void handleUpdateAction(const char *item)
{
    Serial.println("special action: " + String(item));
    if (strcmp(item, "update") == 0)
    {
        // Clear all files in the LittleFS
        Dir dir = LittleFS.openDir("/");
        while (dir.next())
        {
            String fileName = dir.fileName();
            Serial.print("Deleting file: ");
            Serial.println(fileName);
            LittleFS.remove(fileName);
        }
        ESP.restart();
    }
    if (strcmp(item, "config") == 0)
    {
        wifimanager.erase();
        ESP.restart();
    }
}

void setup()
{
    Serial.begin(115200);
    EEPROM.begin(80);
    mqtt_config config;
    EEPROM.get(0, config);
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", config.server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", config.port, 6);
    wifimanager.addParameter(&custom_mqtt_server);
    wifimanager.addParameter(&custom_mqtt_port);
    wifimanager.setSaveConfigCallback([&config, &custom_mqtt_server, &custom_mqtt_port]()
                                      {
        strcpy(config.server, custom_mqtt_server.getValue());
        strcpy(config.port, custom_mqtt_port.getValue());
        EEPROM.put(0, config);
        EEPROM.commit(); });
    wifimanager.setAPCallback([](WiFiManager *wifimanager)
                              { animator.set_text_and_run("AP Mode: VFD-03", 210, 200); });
    set_key_listener();
    settimeofday_cb([]() { // set callback to execute after time is retrieved
        style_page = STYLE_TIME;
        isTimeSet = true;
        Serial.println("Time set");
        vfd_gui_set_pic(PIC_CLOCK, true);
    });
    configTime(MY_TZ, MY_NTP_SERVER);

    vfd_gui_init();
    vfd_gui_set_blk_level(light_level);
    vfd_gui_set_text(" boot");
    animator.start_loading(0x01 | 0x20);

    wifimanager.autoConnect("VFD-03");
    vfd_gui_set_pic(PIC_WIFI, true);
    web_setup();
    initOTA();
    animator.onStart([]()
                     { style_page = STYLE_TEXT; });
    animator.onEnd([]()
                   { 
                     vfd_gui_set_pic(PIC_PLAY, false); 
                     style_page = isTimeSet ? STYLE_TIME : STYLE_NOTIME; });

    menuhandler.begin();
    menuhandler.initializeMenuItems();
    menuhandler.setSpecialActionCallback(handleUpdateAction);

    if (!kiwi.isDataAvailable())
    {
        animator.set_text_and_run("Kiwi Loading...", 210, 20);
        if (kiwi.processApiData())
        {
            Serial.println("Data processing completed successfully!");
        }
        else
        {
            Serial.println("Failed to process API data!");
        }
        ESP.restart();
    }
    animator.set_text_and_run(WiFi.localIP().toString().c_str(), 210);
    // Initialize MQTT with message callback
    if (strlen(config.server) > 0)
    {
        mqttManager = new MqttManager(config.server, atoi(config.port), "text/set");
        mqttManager->onMessage(handleMqttMessage);
        mqttManager->begin();
    }
    task_time_refresh.attach_ms(VFD_TIME_FRAME, task_time_refresh_fun);
}

void initOTA()
{
    // OTA upgrade initialization
    ArduinoOTA.setPassword("lonelybinary");
    ArduinoOTA.onStart([]()
                       {
         task_time_refresh.detach();
         task_key_pressed.detach();
         animator.set_text_and_run("*OTA*", 210, 200);
         vfd_gui_set_pic(PIC_REC, true); });
    ArduinoOTA.onEnd([]()
                     {
         animator.stop();
         vfd_gui_set_pic(PIC_REC, false);
         vfd_gui_set_text("reboot");
         ESP.restart(); });

    ArduinoOTA.begin();
}

void loop()
{
    if (mqttManager)
        mqttManager->loop(); // Handle MQTT connection and callback
    ArduinoOTA.handle();
    web_loop();
    vfd_synchronous();
    set_tick();
}

void set_key_listener()
{
    pinMode(KEY1, INPUT);
    //  Register key interrupt function
    attachInterrupt(digitalPinToInterrupt(KEY1), keyISR, CHANGE);
}

//////////////////////////////////////////////////////////////////////////////////
//// External key interrupt handling
//////////////////////////////////////////////////////////////////////////////////
void IRAM_ATTR keyISR()
{
    // Get current time for debouncing
    unsigned long currentTime = millis();
    // Check if enough time has passed since the last state change (debounce)
    if (currentTime - lastDebounceTime < debounceDelay)
    {
        return; // Exit if we're within the debounce period
    }
    // Update the debounce timer
    lastDebounceTime = currentTime;

    // Process button states with debouncing applied
    if (!digitalRead(KEY1))
    { // Button Pressed
        pressStartTime = millis();
        keyPressed = true;
        isLongPress = false;                               // Reset long press flag
        task_key_pressed.attach_ms(SCROLL_INTERVAL, []() { // Use lambda for the callback
            if (keyPressed && millis() - pressStartTime >= LONG_PRESS_TIME)
            {
                animator.stop();
                style_page = STYLE_MENU;
                isLongPress = true; // Mark as long press
                String menuText = menuhandler.scrollToNextItem();
                vfd_gui_set_text(menuText.c_str());
            }
        });
    }
    else
    {                              // Button Released
        task_key_pressed.detach(); // Stop menu scrolling
        keyPressed = false;
        style_page = STYLE_TIME;
        unsigned long pressDuration = millis() - pressStartTime;
        if (pressDuration < LONG_PRESS_TIME && !isLongPress)
        {
            // Short press action: select current menu item
            String msg = menuhandler.selectCurrentItem();
            if (msg.length() > 0)
            {
                animator.set_text_and_run(msg.c_str(), 210);
            }
        }
        if (isLongPress)
        { // Released after long press
            menuhandler.flashCurrentMenuItem();
            // Visual feedback - blink current menu text
            for (int i = 0; i < 4; i++)
            {
                vfd_gui_set_text(" ");
                delay(100);
                vfd_gui_set_text(menuhandler.getMenuItems().at(menuhandler.getCurrentMenuIndex()).menu.c_str());
                delay(100);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
//// Task asynchronous tasks
//////////////////////////////////////////////////////////////////////////////////

void task_time_refresh_fun()
{
    if (style_page == STYLE_TIME)
    {
        time_t now;
        tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        char buffer[60];
        if (timeinfo.tm_sec % 20 == 0)
        {
            if (mqttManager)
                mqttManager->publishDynamic();
            strftime(buffer, sizeof(buffer), "%A %d %B %Y", &timeinfo);
            animator.set_text_and_run(buffer, 210);
            return;
        }
        strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);
        vfd_gui_set_text(buffer);
        vfd_gui_set_maohao1(mh_state);
        vfd_gui_set_maohao2(mh_state);
        mh_state = !mh_state;
    }
    else if (style_page == STYLE_NOTIME)
    {
        vfd_gui_set_text("NO NTP");
        vfd_gui_set_pic(PIC_CLOCK, false);
    }
    vfd_gui_set_pic(PIC_WIFI, WiFi.isConnected());
}

void set_tick()
{
}

//////////////////////////////////////////////////////////////////////////////////
//// Synchronous blocking tasks
//////////////////////////////////////////////////////////////////////////////////

void vfd_synchronous()
{
}