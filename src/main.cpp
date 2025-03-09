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

void set_key_listener();
IRAM_ATTR void handle_key_interrupt();
IRAM_ATTR void keyISR();
void scrollMenu();
void selectMenuItem();
void getTimeInfo();
void set_tick();
void task_time_refresh_fun();
void vfd_synchronous();

u32 key_filter_sec = 0; // Key debounce
u32 k1_last_time = 0;   // Last trigger time record of key 1
u8 key_last_pin = 0;    // Record the last pressed key PIN

tm timeinfo; // Time object
time_t now;

u8 mh_state = 0;    // Colon display state
u8 light_level = 1; // Brightness level

u8 style_page = STYLE_TIME; // Page display style

// Timer initialization
Ticker task_time_refresh; // Time refresh

WiFiManager wifimanager;
Animator animator;
Kiwi kiwi;
MenuHandler menuhandler;

// key habndling
volatile bool keyPressed = false;
volatile unsigned long pressStartTime = 0;
Ticker task_key_pressed;
bool isLongPress = false; // Tracks whether long press was detected

// Menu items
std::vector<MenuItem> menuItems;
u8 currentMenuIndex = 0; // Track current menu selection

void setup()
{
    Serial.begin(115200);

    set_key_listener();
    configTime(MY_TZ, MY_NTP_SERVER);

    vfd_gui_init();
    vfd_gui_set_blk_level(light_level);
    vfd_gui_set_text(" boot");
    animator.start_loading(0x01 | 0x20);

    wifimanager.autoConnect("VFD-03");
    vfd_gui_set_pic(PIC_WIFI, true);
    web_setup();
    // OTA upgrade initialization
    ArduinoOTA.setPassword("lonelybinary");
    ArduinoOTA.onStart([]()
                       {
        task_time_refresh.detach();
        animator.set_text_and_run("*OTA*", 120, 100);
        vfd_gui_set_pic(PIC_REC, true); });
    ArduinoOTA.onEnd([]()
                     {
        animator.stop();
        vfd_gui_set_pic(PIC_REC, false);
        vfd_gui_set_text("reboot");
        ESP.restart(); });
    ArduinoOTA.begin();
    animator.onStart([]()
                     { style_page = STYLE_TEXT; });
    animator.onEnd([]()
                   { style_page = STYLE_TIME; });

    menuhandler.begin();
    menuItems = menuhandler.getActiveMenuItems();
    MenuItem random;
    random.menu = "random";
    menuItems.insert(menuItems.begin(), random);
    MenuItem upd;
    upd.menu = "update";
    menuItems.push_back(upd);

    if (!kiwi.isDataAvailable())
    {
        animator.set_text_and_run("Kiwi Loading...", 210, 20);
        if (kiwi.processApiData("https://kiwidesschicksals.de/kiwi2.php"))
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
}

void loop()
{
    ArduinoOTA.handle();
    getTimeInfo();
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

void getTimeInfo()
{
    time(&now); // read the current time
    localtime_r(&now, &timeinfo);
}

//////////////////////////////////////////////////////////////////////////////////
//// External key interrupt handling
//////////////////////////////////////////////////////////////////////////////////
void IRAM_ATTR keyISR()
{
    if (!digitalRead(KEY1))
    { // Button Pressed
        pressStartTime = millis();
        keyPressed = true;
        isLongPress = false;                              // Reset long press flag
        task_key_pressed.attach_ms(SCROLL_INTERVAL, scrollMenu); // Start scrolling every 500ms
    }
    else
    {                       // Button Released
        task_key_pressed.detach(); // Stop menu scrolling
        keyPressed = false;
        style_page = STYLE_TIME;
        unsigned long pressDuration = millis() - pressStartTime;
        if (pressDuration < LONG_PRESS_TIME && !isLongPress)
        {
            selectMenuItem(); // Short press action
        }
        if (isLongPress) // released 
        {
            for (int i = 0; i < 4; i++)
            {
                vfd_gui_set_text(" ");
                delay(100);
                vfd_gui_set_text(menuItems[currentMenuIndex].menu.c_str());
                delay(100);
            }
        }
    }
}

void scrollMenu()
{
    if (keyPressed && millis() - pressStartTime >= LONG_PRESS_TIME)
    {
        animator.stop();
        style_page = STYLE_MENU;
        isLongPress = true; // Mark as long press
        currentMenuIndex = (currentMenuIndex + 1) % menuItems.size();
        Serial.println(menuItems.at(currentMenuIndex).menu);
        vfd_gui_set_text(menuItems.at(currentMenuIndex).menu.c_str());
    }
}

void selectMenuItem()
{
    Serial.println("Short Press: Select Menu Item");
    // if first item is selected, random record is selected
    size_t menuitem = currentMenuIndex;
    if (currentMenuIndex == 0)
    {
        menuitem = random(1, menuItems.size() - 1);
    }
    // check for last item
    if (currentMenuIndex == menuItems.size() - 1)
    {
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
    // Your short press action logic here
    String msg = menuhandler.getRandomRecord(menuItems.at(menuitem));
    animator.set_text_and_run(msg.c_str(), 210);
}

//////////////////////////////////////////////////////////////////////////////////
//// Task asynchronous tasks
//////////////////////////////////////////////////////////////////////////////////

void task_time_refresh_fun()
{
    if (style_page == STYLE_TIME)
    {
        char buffer[60];
        if (timeinfo.tm_sec % 20 == 0)
        {
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
}