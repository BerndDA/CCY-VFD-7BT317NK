// services/ConfigService.h
#ifndef CONFIG_SERVICE_H
#define CONFIG_SERVICE_H

class ConfigService {
public:
    struct Config {
        // Add your config fields here
        //uint8_t brightness = 2;
        // ... other config
    };
    
    ConfigService() {}
    
    void begin() {
        // Load config from EEPROM in the future
    }
    
    void save() {
        // Save config to EEPROM in the future
    }
    
    const Config& getConfig() const { return config; }
    
private:
    Config config;
};
#endif // CONFIG_SERVICE_H