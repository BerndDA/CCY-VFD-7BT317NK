#include <Ticker.h>
#include <Arduino.h>
#include <gui.h>
#include <functional>

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

// Define fade patterns - percentage of segments to show (from 0% to 100%)
// These are masks that will be applied to character patterns
#define FADE_PATTERN_0 0x000000   // 0% - No segments visible
#define FADE_PATTERN_25 0x901800  // 25% - Only a few segments visible (vertical segments)
#define FADE_PATTERN_50 0xB09C00  // 50% - Half of the segments visible (vertical + some horizontal)
#define FADE_PATTERN_75 0xF8DC1C  // 75% - Most segments visible
#define FADE_PATTERN_100 0xFFFFFF // 100% - All segments visible

// Define an array of fade patterns for easy iteration
#define FADE_PATTERNS { \
    FADE_PATTERN_0, FADE_PATTERN_25, FADE_PATTERN_50, FADE_PATTERN_75, FADE_PATTERN_100}

// Constant array that can be used in the code
const u32 fadePatterns[] = FADE_PATTERNS;

// Number of fade steps
#define FADE_SEGMENTS_COUNT (sizeof(fadePatterns) / sizeof(fadePatterns[0]))

// Animation type enum to track the current animation
enum AnimationType
{
    ANIM_TEXT,
    ANIM_LOADING,
    ANIM_FADE_IN,
    ANIM_FADE_OUT,
    ANIM_ADVANCED_FADE_IN,
    ANIM_ADVANCED_FADE_OUT,
    ANIM_RANDOM_FADE_IN,
    ANIM_RANDOM_FADE_OUT,
    ANIM_WAVE,
    ANIM_TYPEWRITER,
    ANIM_REVEAL
};

class Animator
{
private:
    Ticker _ticker;
    Ticker _delayedCallbackTicker; // New ticker for delayed callback execution
    String _text;
    uint8_t _frame = 210;
    uint8_t _index = 0;
    uint8_t _length = 0;
    uint8_t _cycles = 1;
    uint8_t _currentCycle = 1;
    uint8_t _positions = 0;
    AnimationType _animType = ANIM_TEXT; // Track animation type

    // Array to store the original patterns for each character position during fade
    u32 _originalPatterns[6];

    bool _running = false;
    void (*_globalEndCallback)() = nullptr;   // Global callback for all animations
    void (*_startCallback)() = nullptr;
    std::function<void()> _animCallback = nullptr;
    
    // New: Animation-specific callback
    std::function<void()> _currentAnimEndCallback = nullptr;

    static void _static_callback(Animator *instance)
    {
        instance->loop();
    }

    // Helper method to execute a delayed callback
    void executeDelayedCallback(std::function<void()> callback, unsigned long delayMs)
    {
        if (callback) {
            _delayedCallbackTicker.once_ms(delayMs, [callback]() {
                callback();
            });
        }
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

    void fade_in_callback()
    {
        char buffer[7]; // Buffer for storing displayed text (6 chars + null terminator)
        strncpy(buffer, _text.c_str(), 6);
        buffer[6] = '\0'; // Ensure null termination

        // First display the text
        vfd_gui_set_text(buffer);

        // Then apply the fade pattern mask to each character
        u32 fadePattern = fadePatterns[_index];

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            u32 charPattern = gui_get_font(buffer[i]);
            // Apply fade mask to the character
            u32 maskedPattern = charPattern & fadePattern;
            // Update just this position with the masked pattern
            vfd_gui_set_one_pattern(i, maskedPattern);
        }
    }

    void fade_out_callback()
    {
        char buffer[7]; // Buffer for storing displayed text (6 chars + null terminator)
        strncpy(buffer, _text.c_str(), 6);
        buffer[6] = '\0'; // Ensure null termination

        // Apply the fade pattern mask to each character
        u32 fadePattern = fadePatterns[_index];

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            u32 charPattern = gui_get_font(buffer[i]);
            // Apply fade mask to the character
            u32 maskedPattern = charPattern & fadePattern;
            // Update just this position with the masked pattern
            vfd_gui_set_one_pattern(i, maskedPattern);
        }
    }

    void advanced_fade_in_callback()
    {
        char buffer[7];
        strncpy(buffer, _text.c_str(), 6);
        buffer[6] = '\0';

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            u32 original = _originalPatterns[i];
            u32 current = 0;

            // Add segments one by one based on the index
            // We don't know exactly which bits in the pattern correspond to which segments,
            // so we'll simply activate bits in sequence for demonstration

            for (uint8_t bit = 0; bit < 24; bit++)
            {
                if ((original & (1UL << bit)) && (bit <= _index))
                {
                    current |= (1UL << bit);
                }
            }

            vfd_gui_set_one_pattern(i, current);
        }
    }

    void advanced_fade_out_callback()
    {
        char buffer[7];
        strncpy(buffer, _text.c_str(), 6);
        buffer[6] = '\0';

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            u32 original = _originalPatterns[i];
            u32 current = 0;

            // Remove segments one by one based on the index
            for (uint8_t bit = 0; bit < 24; bit++)
            {
                if ((original & (1UL << bit)) && (bit >= _index))
                {
                    current |= (1UL << bit);
                }
            }

            vfd_gui_set_one_pattern(i, current);
        }
    }

    void random_fade_in_callback()
    {
        char buffer[7];
        strncpy(buffer, _text.c_str(), 6);
        buffer[6] = '\0';

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            u32 original = _originalPatterns[i];
            u32 current = 0;

            // Random fade - gradually increase probability of segment being shown
            float probability = _index / (float)_length;

            for (uint8_t bit = 0; bit < 24; bit++)
            {
                if (original & (1UL << bit))
                {
                    // Use a random value compared against our probability threshold
                    float rnd = random(100) / 100.0;
                    if (rnd < probability)
                    {
                        current |= (1UL << bit);
                    }
                }
            }

            vfd_gui_set_one_pattern(i, current);
        }
    }

    void random_fade_out_callback()
    {
        char buffer[7];
        strncpy(buffer, _text.c_str(), 6);
        buffer[6] = '\0';

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            u32 original = _originalPatterns[i];
            u32 current = 0;

            // Random fade out - gradually decrease probability of segment being shown
            float probability = 1.0 - (_index / (float)_length);

            for (uint8_t bit = 0; bit < 24; bit++)
            {
                if (original & (1UL << bit))
                {
                    // Use a random value compared against our probability threshold
                    float rnd = random(100) / 100.0;
                    if (rnd < probability)
                    {
                        current |= (1UL << bit);
                    }
                }
            }

            vfd_gui_set_one_pattern(i, current);
        }
    }

    void wave_effect_callback()
    {
        char displayBuffer[7];
        memset(displayBuffer, ' ', 6);
        displayBuffer[6] = '\0';

        // Get the current slice of text to display
        for (int i = 0; i < 6; i++)
        {
            if (_index + i < _text.length())
            {
                displayBuffer[i] = _text[_index + i];
            }
        }

        // Display the current slice of text first
        vfd_gui_set_text(displayBuffer);

        // Create wave effect by applying a phase-shifted sine wave to character brightness
        for (int i = 0; i < 6; i++)
        {
            if (displayBuffer[i] != ' ')
            {
                float phase = (_index + i) * 0.5; // Phase shift based on position
                float sineValue = sin(phase);

                // Map sine value (-1 to 1) to a brightness value (0 to 1)
                float brightness = (sineValue + 1) / 2.0;

                // Get the original pattern for this character
                u32 pattern = gui_get_font(displayBuffer[i]);

                // Apply brightness by masking segments
                u32 maskedPattern;
                if (brightness < 0.25)
                {
                    maskedPattern = pattern & FADE_PATTERN_25;
                }
                else if (brightness < 0.5)
                {
                    maskedPattern = pattern & FADE_PATTERN_50;
                }
                else if (brightness < 0.75)
                {
                    maskedPattern = pattern & FADE_PATTERN_75;
                }
                else
                {
                    maskedPattern = pattern; // Full brightness
                }

                vfd_gui_set_one_pattern(i, maskedPattern);
            }
        }
    }

    void typewriter_effect_callback()
    {
        // For the typewriter effect, we display characters one by one
        char displayBuffer[7];
        memset(displayBuffer, ' ', 6);
        displayBuffer[6] = '\0';

        // Fill the buffer with characters up to current index
        int displayChars = min(6, (int)_index);
        for (int i = 0; i < displayChars; i++)
        {
            // Don't exceed the text length
            if (i < _text.length())
            {
                displayBuffer[i] = _text[i];
            }
        }

        // Update the display
        vfd_gui_set_text(displayBuffer);

        // Add a cursor effect at the current position
        if (_index <= 6 && _index < _text.length())
        {
            // Make the cursor character blink
            u32 cursorPattern = 0;

            // Use a vertical line as cursor (modify this pattern as needed for your display)
            // This uses SEG_P3 and SEG_P7 for a simple vertical line
            if (_index % 2 == 0)
            { // Blink effect
                cursorPattern = SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19;
            }

            vfd_gui_set_one_pattern(_index, cursorPattern);
        }
    }

    void reveal_effect_callback()
    {
        char displayBuffer[7];
        strncpy(displayBuffer, _text.c_str(), 6);
        displayBuffer[6] = '\0';

        // First display the full text
        vfd_gui_set_text(displayBuffer);

        // Then mask out the characters not yet revealed
        for (int i = 0; i < 6; i++)
        {
            if (i >= _index && i < 6 && displayBuffer[i] != ' ')
            {
                // Hide this character
                vfd_gui_set_one_pattern(i, 0);
            }
        }
    }

void start(uint8_t cycles = 1, std::function<void()> callback = nullptr, unsigned long delayMs = 0)
{
    _index = 0;
    _cycles = cycles;
    _currentCycle = 1;
    _running = true;
    
    // Store the animation-specific callback, wrapping it with delay if needed
    if (callback && delayMs > 0) {
        // Create a new callback that includes the delay
        _currentAnimEndCallback = [this, callback, delayMs]() {
            // Schedule the actual callback after the delay
            _delayedCallbackTicker.once_ms(delayMs, callback);
        };
    } else {
        // No delay, just store the callback directly
        _currentAnimEndCallback = callback;
    }
    
    if (_startCallback)
        _startCallback();
        
    _ticker.attach_ms_scheduled_accurate(_frame, std::bind(&Animator::_static_callback, this));
}

public:
    Animator() {}

    void set_text_and_run(const char *text, uint8_t frame = 210, uint8_t cycles = 1, 
                         std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        set_text(text, frame);
        start(cycles, callback, delayMs);
    }

    void start_loading(uint8_t positions, std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        _index = 0;
        _length = SEGMENT_STEPS_COUNT - 1;
        _frame = 80;
        _positions = positions;
        _animCallback = std::bind(&Animator::loading_callback, this);
        _animType = ANIM_LOADING;
        start(255, callback, delayMs);
    }

    void set_text(const char *text, uint8_t frame = 210)
    {
        _text = "     ";
        _text += text;
        _frame = frame;
        _length = _text.length();
        _animCallback = std::bind(&Animator::text_callback, this);
        _animType = ANIM_TEXT;
    }

    void start_fade_in(const char *text, uint8_t frame = 120, 
                      std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        _text = text;
        _frame = frame;
        _index = 0;
        _length = FADE_SEGMENTS_COUNT - 1;
        _animCallback = std::bind(&Animator::fade_in_callback, this);
        _animType = ANIM_FADE_IN;
        start(1, callback, delayMs); // Run through the fade sequence once
    }

    void start_fade_out(const char *text, uint8_t frame = 120, 
                       std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        _text = text;
        vfd_gui_set_text(_text.c_str()); // First display the full text
        _frame = frame;
        _index = FADE_SEGMENTS_COUNT - 1; // Start from full visibility
        _length = 0;                      // End at 0 visibility
        _animCallback = std::bind(&Animator::fade_out_callback, this);
        _animType = ANIM_FADE_OUT;
        start(1, callback, delayMs); // Run through the fade sequence once
    }

    void start_advanced_fade_in(const char *text, uint8_t frame = 80, 
                              std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        _text = text;
        _frame = frame;

        // Store the original patterns for each character
        char buffer[7];
        strncpy(buffer, text, 6);
        buffer[6] = '\0';

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            _originalPatterns[i] = gui_get_font(buffer[i]);
        }

        // Start with all segments off
        vfd_gui_clear();

        // Calculate total segments to animate
        // For most characters there are about 21 segments
        _index = 0;
        _length = 20; // Maximum number of segments per character
        _animCallback = std::bind(&Animator::advanced_fade_in_callback, this);
        _animType = ANIM_ADVANCED_FADE_IN;
        start(1, callback, delayMs);
    }

    void start_advanced_fade_out(const char *text, uint8_t frame = 80, 
                               std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        _text = text;

        // First display the full text
        vfd_gui_set_text(_text.c_str());

        // Store the original patterns for each character
        char buffer[7];
        strncpy(buffer, text, 6);
        buffer[6] = '\0';

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            _originalPatterns[i] = gui_get_font(buffer[i]);
        }

        _frame = frame;
        _index = 0;
        _length = 20; // Maximum number of segments per character
        _animCallback = std::bind(&Animator::advanced_fade_out_callback, this);
        _animType = ANIM_ADVANCED_FADE_OUT;
        start(1, callback, delayMs);
    }

    void start_random_fade_in(const char *text, uint8_t frame = 50, 
                            std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        _text = text;
        _frame = frame;

        // Store the original patterns for each character
        char buffer[7];
        strncpy(buffer, text, 6);
        buffer[6] = '\0';

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            _originalPatterns[i] = gui_get_font(buffer[i]);
        }

        // Start with all segments off
        vfd_gui_clear();

        _index = 0;
        _length = 10; // 10 steps for random fade
        _animCallback = std::bind(&Animator::random_fade_in_callback, this);
        _animType = ANIM_RANDOM_FADE_IN;
        start(1, callback, delayMs);
    }

    void start_random_fade_out(const char *text, uint8_t frame = 50, 
                             std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        _text = text;
        _frame = frame;

        // First display the full text
        vfd_gui_set_text(_text.c_str());

        // Store the original patterns for each character
        char buffer[7];
        strncpy(buffer, text, 6);
        buffer[6] = '\0';

        for (uint8_t i = 0; i < strlen(buffer) && i < 6; i++)
        {
            _originalPatterns[i] = gui_get_font(buffer[i]);
        }

        _index = 0;
        _length = 10; // 10 steps for random fade
        _animCallback = std::bind(&Animator::random_fade_out_callback, this);
        _animType = ANIM_RANDOM_FADE_OUT;
        start(1, callback, delayMs);
    }

    void start_wave_effect(const char *text, uint8_t frame = 100, 
                         std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        _text = text;
        _frame = frame;

        // Pad the text with spaces to ensure all characters are properly shown
        String paddedText = "      ";
        paddedText += text;
        paddedText += "      ";
        _text = paddedText;

        // Clear display first
        vfd_gui_clear();

        _index = 0;
        _length = _text.length() - 6; // Length minus display width
        _animCallback = std::bind(&Animator::wave_effect_callback, this);
        _animType = ANIM_WAVE;
        start(1, callback, delayMs);
    }

    void start_typewriter_effect(const char *text, uint8_t frame = 200, 
                               std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        // Ensure text is always left-aligned for typewriter effect
        _text = text;
        _frame = frame;

        // Clear display first
        vfd_gui_clear();

        _index = 0;
        _length = strlen(text);
        _animCallback = std::bind(&Animator::typewriter_effect_callback, this);
        _animType = ANIM_TYPEWRITER;
        start(1, callback, delayMs);
    }

    void start_reveal_effect(const char *text, uint8_t frame = 150, 
                           std::function<void()> callback = nullptr, unsigned long delayMs = 0)
    {
        if (_running)
            stop();

        _text = text;
        _frame = frame;

        // Clear display first
        vfd_gui_clear();

        _index = 0;
        _length = 6; // 6 steps for the 6 characters
        _animCallback = std::bind(&Animator::reveal_effect_callback, this);
        _animType = ANIM_REVEAL;
        start(1, callback, delayMs);
    }

    void stop()
    {
        _running = false;
        _ticker.detach();
        _delayedCallbackTicker.detach(); // Ensure any pending delayed callbacks are cancelled
        
        // Execute animation-specific callback if there's one
        if (_currentAnimEndCallback) {
            auto callback = _currentAnimEndCallback;
            _currentAnimEndCallback = nullptr; // Clear it to prevent double execution
            callback(); // Execute the animation-specific callback
        }
        // Only call the global callback if there's no animation-specific callback
        // This indicates we've reached the end of a sequence
        else if (_globalEndCallback) {
            _globalEndCallback();
        }
    }

    void onEnd(void (*callback)())
    {
        _globalEndCallback = callback;
    }

    void onStart(void (*callback)())
    {
        _startCallback = callback;
    }

    void loop()
    {
        if (_running)
        {
            // Handle animations that go forward (index increases)
            if ((_animType == ANIM_TEXT ||
                 _animType == ANIM_LOADING ||
                 _animType == ANIM_FADE_IN ||
                 _animType == ANIM_ADVANCED_FADE_IN ||
                 _animType == ANIM_RANDOM_FADE_IN ||
                 _animType == ANIM_RANDOM_FADE_OUT || 
                 _animType == ANIM_WAVE ||
                 _animType == ANIM_TYPEWRITER ||
                 _animType == ANIM_REVEAL) &&
                _index > _length)
            {
                _currentCycle++;
                if (_currentCycle > _cycles)
                {
                    stop();
                    return;
                }
                _index = 0;
            }
            // Handle animations that go backward (index decreases)
            else if (_animType == ANIM_FADE_OUT && _index < _length)
            {
                stop();
                return;
            }

            // Call the animation callback
            _animCallback();

            // Update the index based on animation direction
            if (_animType == ANIM_FADE_OUT)
            {
                _index--;
            }
            else
            {
                _index++;
            }
        }
    }

    bool is_running()
    {
        return _running;
    }
};