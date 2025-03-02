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
#include <service.h>
#include <web_server.h>
#include <WiFiManager.h>
#include <longtext.h>

#define MY_NTP_SERVER "at.pool.ntp.org"
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"

void set_key_listener();
IRAM_ATTR void handle_key_interrupt();
void getTimeInfo();
void configModeTimeoutError();
void set_tick();
void task_time_refresh_fun();
void vfd_synchronous();
void power_handle(u8 state);
void alarm_handle(u8 state);
void countdown_handle(u8 state, u8 hour, u8 min, u8 sec);

u8 power = 1;      // Power
u8 countdounw = 0; // Whether to enable countdown mode

u32 key_filter_sec = 0; // Key debounce
u32 k1_last_time = 0;   // Last trigger time record of key 1
u8 key_last_pin = 0;    // Record the last pressed key PIN

tm timeinfo; // Time object
time_t now;

String time_str = String();
u8 mh_state = 0;    // Colon display state
u8 light_level = 1; // Brightness level

const u8 style_max = 3;        // Total number of supported page display styles
u8 style_page = STYLE_DEFAULT; // Page display style

// Timer initialization
Ticker task_time_refresh; // Time refresh

WiFiManager wifimanager;
Longtext longtext;

void setup()
{
    Serial.begin(115200);

    set_key_listener();
    configTime(MY_TZ, MY_NTP_SERVER);

    vfd_gui_init();
    vfd_gui_set_blk_level(light_level);
    vfd_gui_set_text(" boot");
    // Initialize StoreFS
    store_init();
    // Read data
    store_get_setting(&setting_obj);
    wifimanager.autoConnect("VFD-03");
    vfd_gui_set_pic(PIC_WIFI, true);
    web_setup();
    // OTA upgrade initialization
    ArduinoOTA.setPassword("lonelybinary");
    ArduinoOTA.onStart([]()
                       {
        task_time_refresh.detach();
        longtext.set_and_start("*OTA*", 120, 100);
        vfd_gui_set_pic(PIC_REC, true); });
    ArduinoOTA.onEnd([]()
                     {
        longtext.stop();
        vfd_gui_set_pic(PIC_REC, false);
        vfd_gui_set_text("reboot");
        ESP.restart(); });
    ArduinoOTA.begin();
    longtext.onStart([]()
                     { style_page = STYLE_TEXT; });
    longtext.onEnd([]()
                   { style_page = STYLE_DEFAULT; });
    longtext.set_and_start(WiFi.localIP().toString().c_str());
}

void loop()
{
    ArduinoOTA.handle();
    getTimeInfo();
    if (power)
    {
        // If timing is in progress, this method cannot be executed, otherwise blocking will cause inaccurate timing
        if (!countdounw)
        {
            web_loop();
            vfd_synchronous();
            set_tick();
        }
        logic_handler_countdown(&timeinfo, countdown_handle);
    }
    // Power on/off handling
    logic_handler_auto_power(&timeinfo, power_handle);
    logic_handler_alarm_clock(&timeinfo, alarm_handle);
}

void set_key_listener()
{
    pinMode(KEY1, INPUT);
    pinMode(KEY2, INPUT);
    pinMode(KEY3, INPUT);
    // Register key interrupt function
    attachInterrupt(digitalPinToInterrupt(KEY1), handle_key_interrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(KEY2), handle_key_interrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(KEY3), handle_key_interrupt, CHANGE);
}

void getTimeInfo()
{
    time(&now); // read the current time
    localtime_r(&now, &timeinfo);
}

//////////////////////////////////////////////////////////////////////////////////
//// External key interrupt handling
//////////////////////////////////////////////////////////////////////////////////

IRAM_ATTR void handle_key_interrupt()
{
    u32 filter_sec = (micros() - key_filter_sec) / 1000;
    if (filter_sec < 500)
    {
        return;
    }
    if (!power)
    {
        // If in sleep mode, trigger the key to power on directly
        power_handle(1);
        key_filter_sec = micros();
        return;
    }
    if (countdounw)
    {
        // If in countdown mode, click the key to cancel the countdown
        countdounw = 0;
        logic_handler_countdown_stop();
        key_filter_sec = micros();
        return;
    }
    if (digitalRead(KEY3) && !digitalRead(KEY1) && !digitalRead(KEY2))
    {
        // WIFI settings on or off
        if (WiFi.isConnected())
        {
            // If on, turn off
            web_stop();
        }
        else
        {
            // If off, turn on
            // Turning on is time-consuming and also needs to check if the network is configured. The whole process is actually initialized in setup. It is better to restart the ESP.
            ESP.restart();
        }
    }
    else if (!digitalRead(KEY1))
    {
        key_last_pin = KEY1;
        k1_last_time = 0;
        Serial.println("Light-");
        light_level = light_level == 1 ? 7 : 1;
        vfd_gui_set_blk_level(light_level);
    }
    else if (!digitalRead(KEY2))
    {
        key_last_pin = KEY2;
        k1_last_time = 0;
        Serial.println("Light+");
        if (light_level == 2)
        {
            light_level = 7;
        }
        else if (light_level == 1)
        {
            light_level = 2;
        }
        vfd_gui_set_blk_level(light_level);
    }
    else if (!digitalRead(KEY3))
    {
        key_last_pin = KEY3;
        Serial.println("FN");
        style_page = (style_page + 1) % style_max;
        k1_last_time = micros();
        longtext.stop();
    }
    else if (digitalRead(KEY3))
    {
        // High
        if (digitalRead(KEY1) && digitalRead(KEY2) && key_last_pin == KEY3)
        {
            u32 sec = (micros() - k1_last_time) / 1000;
            if (k1_last_time != 0 && sec > 2000)
            {
                Serial.println("Long press operation triggered");
                // If long press for 2 seconds, perform WIFI reset operation
                ESP.restart();
            }
            else
            {
                k1_last_time = 0;
            }
        }
    }
    key_filter_sec = micros();
}

//////////////////////////////////////////////////////////////////////////////////
//// Task asynchronous tasks
//////////////////////////////////////////////////////////////////////////////////

void task_time_refresh_fun()
{
    if (style_page == STYLE_DEFAULT)
    {
        char buffer[60];
        if (timeinfo.tm_sec % 20 == 0)
        {
            strftime(buffer, sizeof(buffer), "%A %d %B %Y", &timeinfo);
            longtext.set_and_start(buffer, 210);
            return;
        }
        strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);
        vfd_gui_set_text(buffer);
        vfd_gui_set_maohao1(mh_state);
        vfd_gui_set_maohao2(mh_state);
        mh_state = !mh_state;
    }
    vfd_gui_set_pic(PIC_CLOCK, true);
    vfd_gui_set_pic(PIC_WIFI, WiFi.isConnected());
}

void set_tick()
{
    if (!task_time_refresh.active())
    {
        task_time_refresh.attach_ms(VFD_TIME_FRAME, task_time_refresh_fun);
    }
}

//////////////////////////////////////////////////////////////////////////////////
//// Synchronous blocking tasks
//////////////////////////////////////////////////////////////////////////////////

void vfd_synchronous()
{
    if (style_page == STYLE_CUSTOM_1)
    {
        static char long_text[50];
        if (strlen(setting_obj.custom_long_text) != 0 &&
            strcmp(long_text, setting_obj.custom_long_text))
        {
            delay(50);
            memset(long_text, 0, sizeof(long_text));
            memcpy(long_text, setting_obj.custom_long_text,
                   sizeof(setting_obj.custom_long_text));
        }
        longtext.set_and_start(long_text, setting_obj.custom_long_text_frame, 2);
    }
    else if (style_page == STYLE_CUSTOM_2)
    {
        if (WiFi.isConnected())
        {
            longtext.set_and_start(WiFi.localIP().toString().c_str(), 210, 1);
        }
        else
        {
            longtext.set_and_start("WiFi not connected", 210, 1);
        }
    }
}

/**
 * Handle power on/off
 */
void power_handle(u8 state)
{
    if (power == state)
    {
        return;
    }
    if (!state)
    {
        countdounw = 0;
        logic_handler_countdown_stop();
        web_stop();
        task_time_refresh.detach();
        longtext.stop();
        vfd_gui_stop();
    }
    else
    {
        style_page = STYLE_DEFAULT;
        vfd_gui_init();
    }
    power = state;
}

/*
 Alarm handling
*/
void alarm_handle(u8 state)
{
    // Handle state
    static u8 handle_state = 1;
    if (handle_state)
    {
        // Handled
        handle_state = 0;
        task_time_refresh.detach();
        vfd_gui_set_text("alarm.");
        for (size_t i = 0; i < 10; i++)
        {
            delay(350);
        }
        // Reset to pending
        handle_state = 0;
    }
}

/**
 * Countdown handling
 */
void countdown_handle(u8 state, u8 hour, u8 min, u8 sec)
{
    countdounw = state;
    if (countdounw)
    {
        task_time_refresh.detach();
        longtext.stop();
        time_str.clear();
        // Concatenate time string format: HH:mm:ss
        time_str += (hour < 10 ? "0" : "");
        time_str += hour;
        time_str += (min < 10 ? "0" : "");
        time_str += min;
        time_str += (sec < 10 ? "0" : "");
        time_str += sec;
        vfd_gui_set_text(time_str.c_str());
        vfd_gui_set_icon(PIC_REC, 1);
        if (WiFi.isConnected())
        {
            vfd_gui_set_icon(PIC_WIFI | vfd_gui_get_save_icon(), 0);
        }
    }
}
