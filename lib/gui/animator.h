#include <Ticker.h>
#include <Arduino.h>
#include <gui.h>

// loading anim
//  Define the segments sequence for animation
//  Segments order: 3,4,5,6,7,15,14,13,12,11 with 4 active segments at each step

// First, define the array of segment values in the specified order
#define SEGMENT_ORDER {SEG_P3, SEG_P4, SEG_P5, SEG_P6, SEG_P7, SEG_P15, SEG_P14, SEG_P13, SEG_P12, SEG_P11}

// Define the segment combinations (4 consecutive segments) with rollover
#define SEG_STEP_1 (SEG_P3 | SEG_P4 | SEG_P5 | SEG_P6)     // Segments 3,4,5,6
#define SEG_STEP_2 (SEG_P4 | SEG_P5 | SEG_P6 | SEG_P7)     // Segments 4,5,6,7
#define SEG_STEP_3 (SEG_P5 | SEG_P6 | SEG_P7 | SEG_P15)    // Segments 5,6,7,15
#define SEG_STEP_4 (SEG_P6 | SEG_P7 | SEG_P15 | SEG_P14)   // Segments 6,7,15,14
#define SEG_STEP_5 (SEG_P7 | SEG_P15 | SEG_P14 | SEG_P13)  // Segments 7,15,14,13
#define SEG_STEP_6 (SEG_P15 | SEG_P14 | SEG_P13 | SEG_P12) // Segments 15,14,13,12
#define SEG_STEP_7 (SEG_P14 | SEG_P13 | SEG_P12 | SEG_P11) // Segments 14,13,12,11
#define SEG_STEP_8 (SEG_P13 | SEG_P12 | SEG_P11 | SEG_P3)  // Segments 13,12,11,3 (rollover)
#define SEG_STEP_9 (SEG_P12 | SEG_P11 | SEG_P3 | SEG_P4)   // Segments 12,11,3,4 (rollover)
#define SEG_STEP_10 (SEG_P11 | SEG_P3 | SEG_P4 | SEG_P5)   // Segments 11,3,4,5 (rollover)

// Define an array of steps for easy iteration
#define SEGMENT_STEPS {                                         \
    SEG_STEP_1, SEG_STEP_2, SEG_STEP_3, SEG_STEP_4, SEG_STEP_5, \
    SEG_STEP_6, SEG_STEP_7, SEG_STEP_8, SEG_STEP_9, SEG_STEP_10}

// Constant array that can be used in the code
const u32 segmentSteps[] = SEGMENT_STEPS;

// Number of steps in the animation sequence
#define SEGMENT_STEPS_COUNT (sizeof(segmentSteps) / sizeof(segmentSteps[0]))

// Example usage function for animation
void animate_segments(uint16_t delay_ms)
{
    static uint8_t current_step = 0;

    // Set the current step's segments
    vfd_gui_set_icon(segmentSteps[current_step], 1);

    // Move to next step with rollover
    current_step = (current_step + 1) % SEGMENT_STEPS_COUNT;

    // Delay is handled by the calling code
}

class Animator
{
private:
    Ticker _ticker;
    String _text;
    uint8_t _frame = 210;
    uint8_t _index = 0;
    uint8_t _length = 0;
    uint8_t _cycles = 1;
    uint8_t _currentCycle = 1;
    uint8_t _positions = 0;

    bool _running = false;
    void (*_endCallback)() = 0;
    void (*_startCallback)() = 0;
    std::function<void()> _animCallback = nullptr;
    static void _static_callback(Animator *instance)
    {
        instance->loop();
    }
    void text_callback()
    {
        vfd_gui_set_text(_text.substring(_index, _index + 6).c_str());
    }
    void loading_callback()
    {
        for (uint8_t i = 0; i < 6; i++) // Assuming 6 possible positions
        {
            if (_positions & (1 << i)) // Check if bit i is set
                vfd_gui_set_one_pattern(i, segmentSteps[_index]);
        }
    }
    void start(uint8_t cycles = 1)
    {
        _index = 0;
        _cycles = cycles;
        _currentCycle = 1;
        _running = true;
        if (_startCallback)
            _startCallback();
        _ticker.attach_ms(_frame, std::bind(&Animator::_static_callback, this));
    }

public:
    Animator() {}
    void set_text_and_run(const char *text, uint8_t frame = 210, uint8_t cycles = 1)
    {
        if (_running)
            stop();
        set_text(text, frame);
        start(cycles);
    }
    void start_loading(uint8_t positions)
    {
        if (_running)
            stop();
        _index = 0;
        _length = SEGMENT_STEPS_COUNT - 1;
        _frame = 80;
        _positions = positions;
        _animCallback = std::bind(&Animator::loading_callback, this);
        start(255);
    }
    void set_text(const char *text, uint8_t frame = 210)
    {
        _text = "     ";
        _text += text;
        _frame = frame;
        _length = _text.length();
        _animCallback = std::bind(&Animator::text_callback, this);
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
            // vfd_gui_set_text(_text.substring(_index, _index + 6).c_str());
            _animCallback();
            _index++;
        }
    }

    bool is_running()
    {
        return _running;
    }
};