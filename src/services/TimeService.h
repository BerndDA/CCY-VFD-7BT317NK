// services/TimeService.h
#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include <functional>
#include <time.h>
#include <Arduino.h>

class TimeService {
private:
    bool timeSynced;
    unsigned long lastSyncAttempt;
    unsigned long lastUpdateCheck;
    static constexpr unsigned long SYNC_INTERVAL = 3600000; // 1 hour
    static constexpr unsigned long SYNC_RETRY_INTERVAL = 10000; // 10 seconds
    
    std::function<void()> onTimeSyncCallback;
    
public:
    TimeService();
    ~TimeService();
    
    // Initialize the time service
    void begin();
    
    // Update function (call from main loop)
    void update();
    
    // Time synchronization
    bool syncTime();
    bool isTimeSynced() const { return timeSynced; }
    
    // Callbacks
    void onTimeSync(std::function<void()> callback) { onTimeSyncCallback = callback; }
    
    // Time info
    struct TimeInfo {
        int hour;
        int minute;
        int second;
        int day;
        int month;
        int year;
        int dayOfWeek;
    };
    
    TimeInfo getCurrentTime() const;
    String getFormattedTime() const;
    String getFormattedDate() const;
};

#endif // TIME_SERVICE_H