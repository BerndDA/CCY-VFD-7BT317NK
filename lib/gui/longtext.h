#include <Ticker.h>
#include <Arduino.h>
#include <gui.h>

class Longtext
{
private:
    Ticker _ticker;
    String _text;
    uint8_t _frame = 210;
    uint8_t _index = 0;
    uint8_t _length = 0;
    uint8_t _cycles = 1;
    uint8_t _currentCycle = 1;
    bool _running = false;
    void (*_endCallback)() = 0;
    void (*_startCallback)() = 0;
    static void _static_callback(Longtext *instance)
    {
        instance->loop();
    }

public:
    Longtext() {}
    void set_and_start(const char *text, uint8_t frame = 210, uint8_t cycles = 1)
    {
        set_text(text, frame);
        start(cycles);
    }
    void set_text(const char *text, uint8_t frame = 210)
    {
        _text = "     ";
        _text += text;
        _frame = frame;
        _length = _text.length();
    }
    void start(uint8_t cycles = 1)
    {
        if (_running)
            stop();
        _index = 0;
        _cycles = cycles;
        _currentCycle = 1;
        _running = true;
        if(_startCallback)
            _startCallback();
        _ticker.attach_ms(_frame, std::bind(&Longtext::_static_callback, this));
    }
    void stop()
    {
        _running = false;
        _ticker.detach();
        if (_endCallback)
            _endCallback();
    }
    void onEnd(void (*callback)())
    {
        _endCallback = callback;
    }
    void onStart(void (*callback)())
    {
        _startCallback = callback;
    }
    void loop()
    {
        if (_running)
        {
            if (_index > _length)
            {
                _currentCycle++;
                if (_currentCycle > _cycles)
                {
                    stop();
                    return;
                }
                _index = 0;
            }
            vfd_gui_set_text(_text.substring(_index, _index + 6).c_str());
            _index++;
        }
    }
    bool is_running()
    {
        return _running;
    }
};