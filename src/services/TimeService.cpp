// services/TimeService.cpp
#include "TimeService.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <coredecls.h> // for settimeofday_cb

// NTP Configuration
#define MY_NTP_SERVER "pool.ntp.org"
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"

// Static callback wrapper
static std::function<void()> staticTimeSyncCallback;
static void timeSync_cb() {
    if (staticTimeSyncCallback) {
        staticTimeSyncCallback();
    }
}

TimeService::TimeService() 
    : timeSynced(false), lastSyncAttempt(0), lastUpdateCheck(0) {
}

TimeService::~TimeService() {
    // Clear the static callback
    settimeofday_cb((void (*)(void))nullptr);
    staticTimeSyncCallback = nullptr;
}

void TimeService::begin() {
    Serial.println("TimeService: Initializing...");
    
    // Set up the time sync callback
    staticTimeSyncCallback = [this]() {
        Serial.println("TimeService: Time synchronized!");
        timeSynced = true;
        if (onTimeSyncCallback) {
            onTimeSyncCallback();
        }
    };
    
    settimeofday_cb(timeSync_cb);
    
    // Configure time zone and NTP
    configTime(MY_TZ, MY_NTP_SERVER);
    
    // Try initial sync if WiFi is connected
    if (WiFi.isConnected()) {
        syncTime();
    }
}

void TimeService::update() {
    unsigned long currentMillis = millis();
    
    // Check if we need to sync time
    if (!timeSynced) {
        // Retry sync every SYNC_RETRY_INTERVAL if not synced
        if (currentMillis - lastSyncAttempt >= SYNC_RETRY_INTERVAL) {
            if (WiFi.isConnected()) {
                syncTime();
            }
            lastSyncAttempt = currentMillis;
        }
    } else {
        // Re-sync every SYNC_INTERVAL to maintain accuracy
        if (currentMillis - lastSyncAttempt >= SYNC_INTERVAL) {
            if (WiFi.isConnected()) {
                syncTime();
            }
            lastSyncAttempt = currentMillis;
        }
    }
}

bool TimeService::syncTime() {
    if (!WiFi.isConnected()) {
        Serial.println("TimeService: Cannot sync - WiFi not connected");
        return false;
    }
    
    Serial.println("TimeService: Attempting time sync...");
    
    // The actual sync happens asynchronously
    // The callback will be called when sync is complete
    // For now, we just trigger the sync
    configTime(MY_TZ, MY_NTP_SERVER);
    
    return true; // Return true to indicate sync was initiated
}

TimeService::TimeInfo TimeService::getCurrentTime() const {
    TimeInfo info;
    time_t now;
    tm timeinfo;
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    info.hour = timeinfo.tm_hour;
    info.minute = timeinfo.tm_min;
    info.second = timeinfo.tm_sec;
    info.day = timeinfo.tm_mday;
    info.month = timeinfo.tm_mon + 1; // tm_mon is 0-11
    info.year = timeinfo.tm_year + 1900; // tm_year is years since 1900
    info.dayOfWeek = timeinfo.tm_wday; // 0 = Sunday
    
    return info;
}

String TimeService::getFormattedTime() const {
    if (!timeSynced) {
        return "NO NTP";
    }
    
    time_t now;
    tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    char buffer[10];
    strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);
    
    return String(buffer);
}

String TimeService::getFormattedDate() const {
    if (!timeSynced) {
        return "NO DATE";
    }
    
    time_t now;
    tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%A %d %B %Y", &timeinfo);
    
    return String(buffer);
}