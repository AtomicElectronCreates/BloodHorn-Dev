/*
 * input.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_INPUT_H
#define BLOODHORN_INPUT_H

#include <bloodhorn/types.h>
#include <bloodhorn/status.h>

#ifdef __cplusplus
extern "C" {
#endif

// Key codes
typedef enum {
    BH_KEY_UNKNOWN = 0,
    
    // Printable keys
    BH_KEY_SPACE = 32,
    BH_KEY_APOSTROPHE = 39,  // '
    BH_KEY_COMMA = 44,       // ,
    BH_KEY_MINUS = 45,       // -
    BH_KEY_PERIOD = 46,      // .
    BH_KEY_SLASH = 47,       // /
    BH_KEY_0 = 48,
    BH_KEY_1 = 49,
    BH_KEY_2 = 50,
    BH_KEY_3 = 51,
    BH_KEY_4 = 52,
    BH_KEY_5 = 53,
    BH_KEY_6 = 54,
    BH_KEY_7 = 55,
    BH_KEY_8 = 56,
    BH_KEY_9 = 57,
    BH_KEY_SEMICOLON = 59,   // ;
    BH_KEY_EQUAL = 61,       // =
    BH_KEY_A = 65,
    BH_KEY_B = 66,
    BH_KEY_C = 67,
    BH_KEY_D = 68,
    BH_KEY_E = 69,
    BH_KEY_F = 70,
    BH_KEY_G = 71,
    BH_KEY_H = 72,
    BH_KEY_I = 73,
    BH_KEY_J = 74,
    BH_KEY_K = 75,
    BH_KEY_L = 76,
    BH_KEY_M = 77,
    BH_KEY_N = 78,
    BH_KEY_O = 79,
    BH_KEY_P = 80,
    BH_KEY_Q = 81,
    BH_KEY_R = 82,
    BH_KEY_S = 83,
    BH_KEY_T = 84,
    BH_KEY_U = 85,
    BH_KEY_V = 86,
    BH_KEY_W = 87,
    BH_KEY_X = 88,
    BH_KEY_Y = 89,
    BH_KEY_Z = 90,
    BH_KEY_LEFT_BRACKET = 91,  // [
    BH_KEY_BACKSLASH = 92,     // \
    BH_KEY_RIGHT_BRACKET = 93, // ]
    BH_KEY_GRAVE_ACCENT = 96,  // `
    BH_KEY_WORLD_1 = 161,      // non-US #1
    BH_KEY_WORLD_2 = 162,      // non-US #2

    // Function keys
    BH_KEY_ESCAPE = 256,
    BH_KEY_ENTER = 257,
    BH_KEY_TAB = 258,
    BH_KEY_BACKSPACE = 259,
    BH_KEY_INSERT = 260,
    BH_KEY_DELETE = 261,
    BH_KEY_RIGHT = 262,
    BH_KEY_LEFT = 263,
    BH_KEY_DOWN = 264,
    BH_KEY_UP = 265,
    BH_KEY_PAGE_UP = 266,
    BH_KEY_PAGE_DOWN = 267,
    BH_KEY_HOME = 268,
    BH_KEY_END = 269,
    BH_KEY_CAPS_LOCK = 280,
    BH_KEY_SCROLL_LOCK = 281,
    BH_KEY_NUM_LOCK = 282,
    BH_KEY_PRINT_SCREEN = 283,
    BH_KEY_PAUSE = 284,
    BH_KEY_F1 = 290,
    BH_KEY_F2 = 291,
    BH_KEY_F3 = 292,
    BH_KEY_F4 = 293,
    BH_KEY_F5 = 294,
    BH_KEY_F6 = 295,
    BH_KEY_F7 = 296,
    BH_KEY_F8 = 297,
    BH_KEY_F9 = 298,
    BH_KEY_F10 = 299,
    BH_KEY_F11 = 300,
    BH_KEY_F12 = 301,
    BH_KEY_F13 = 302,
    BH_KEY_F14 = 303,
    BH_KEY_F15 = 304,
    BH_KEY_F16 = 305,
    BH_KEY_F17 = 306,
    BH_KEY_F18 = 307,
    BH_KEY_F19 = 308,
    BH_KEY_F20 = 309,
    BH_KEY_F21 = 310,
    BH_KEY_F22 = 311,
    BH_KEY_F23 = 312,
    BH_KEY_F24 = 313,
    BH_KEY_F25 = 314,
    BH_KEY_KP_0 = 320,
    BH_KEY_KP_1 = 321,
    BH_KEY_KP_2 = 322,
    BH_KEY_KP_3 = 323,
    BH_KEY_KP_4 = 324,
    BH_KEY_KP_5 = 325,
    BH_KEY_KP_6 = 326,
    BH_KEY_KP_7 = 327,
    BH_KEY_KP_8 = 328,
    BH_KEY_KP_9 = 329,
    BH_KEY_KP_DECIMAL = 330,
    BH_KEY_KP_DIVIDE = 331,
    BH_KEY_KP_MULTIPLY = 332,
    BH_KEY_KP_SUBTRACT = 333,
    BH_KEY_KP_ADD = 334,
    BH_KEY_KP_ENTER = 335,
    BH_KEY_KP_EQUAL = 336,
    BH_KEY_LEFT_SHIFT = 340,
    BH_KEY_LEFT_CONTROL = 341,
    BH_KEY_LEFT_ALT = 342,
    BH_KEY_LEFT_SUPER = 343,
    BH_KEY_RIGHT_SHIFT = 344,
    BH_KEY_RIGHT_CONTROL = 345,
    BH_KEY_RIGHT_ALT = 346,
    BH_KEY_RIGHT_SUPER = 347,
    BH_KEY_MENU = 348,

    BH_KEY_LAST = BH_KEY_MENU
} bh_key_t;

// Modifier key flags
typedef enum {
    BH_MOD_NONE = 0x0000,
    BH_MOD_SHIFT = 0x0001,
    BH_MOD_CONTROL = 0x0002,
    BH_MOD_ALT = 0x0004,
    BH_MOD_SUPER = 0x0008,
    BH_MOD_CAPS_LOCK = 0x0010,
    BH_MOD_NUM_LOCK = 0x0020
} bh_modifier_t;

// Mouse buttons
typedef enum {
    BH_MOUSE_BUTTON_1 = 0,
    BH_MOUSE_BUTTON_2 = 1,
    BH_MOUSE_BUTTON_3 = 2,
    BH_MOUSE_BUTTON_4 = 3,
    BH_MOUSE_BUTTON_5 = 4,
    BH_MOUSE_BUTTON_6 = 5,
    BH_MOUSE_BUTTON_7 = 6,
    BH_MOUSE_BUTTON_8 = 7,
    BH_MOUSE_BUTTON_LAST = BH_MOUSE_BUTTON_8,
    BH_MOUSE_BUTTON_LEFT = BH_MOUSE_BUTTON_1,
    BH_MOUSE_BUTTON_RIGHT = BH_MOUSE_BUTTON_2,
    BH_MOUSE_BUTTON_MIDDLE = BH_MOUSE_BUTTON_3
} bh_mouse_button_t;

// Joystick IDs
typedef enum {
    BH_JOYSTICK_1 = 0,
    BH_JOYSTICK_2 = 1,
    BH_JOYSTICK_3 = 2,
    BH_JOYSTICK_4 = 3,
    BH_JOYSTICK_5 = 4,
    BH_JOYSTICK_6 = 5,
    BH_JOYSTICK_7 = 6,
    BH_JOYSTICK_8 = 7,
    BH_JOYSTICK_9 = 8,
    BH_JOYSTICK_10 = 9,
    BH_JOYSTICK_11 = 10,
    BH_JOYSTICK_12 = 11,
    BH_JOYSTICK_13 = 12,
    BH_JOYSTICK_14 = 13,
    BH_JOYSTICK_15 = 14,
    BH_JOYSTICK_16 = 15,
    BH_JOYSTICK_LAST = BH_JOYSTICK_16
} bh_joystick_id_t;

// Gamepad buttons
typedef enum {
    BH_GAMEPAD_BUTTON_A = 0,
    BH_GAMEPAD_BUTTON_B = 1,
    BH_GAMEPAD_BUTTON_X = 2,
    BH_GAMEPAD_BUTTON_Y = 3,
    BH_GAMEPAD_BUTTON_LEFT_BUMPER = 4,
    BH_GAMEPAD_BUTTON_RIGHT_BUMPER = 5,
    BH_GAMEPAD_BUTTON_BACK = 6,
    BH_GAMEPAD_BUTTON_START = 7,
    BH_GAMEPAD_BUTTON_GUIDE = 8,
    BH_GAMEPAD_BUTTON_LEFT_THUMB = 9,
    BH_GAMEPAD_BUTTON_RIGHT_THUMB = 10,
    BH_GAMEPAD_BUTTON_DPAD_UP = 11,
    BH_GAMEPAD_BUTTON_RIGHT = 12,
    BH_GAMEPAD_BUTTON_DPAD_DOWN = 13,
    BH_GAMEPAD_BUTTON_LEFT = 14,
    BH_GAMEPAD_BUTTON_LAST = BH_GAMEPAD_BUTTON_LEFT,
    BH_GAMEPAD_BUTTON_CROSS = BH_GAMEPAD_BUTTON_A,
    BH_GAMEPAD_BUTTON_CIRCLE = BH_GAMEPAD_BUTTON_B,
    BH_GAMEPAD_BUTTON_SQUARE = BH_GAMEPAD_BUTTON_X,
    BH_GAMEPAD_BUTTON_TRIANGLE = BH_GAMEPAD_BUTTON_Y
} bh_gamepad_button_t;

// Gamepad axes
typedef enum {
    BH_GAMEPAD_AXIS_LEFT_X = 0,
    BH_GAMEPAD_AXIS_LEFT_Y = 1,
    BH_GAMEPAD_AXIS_RIGHT_X = 2,
    BH_GAMEPAD_AXIS_RIGHT_Y = 3,
    BH_GAMEPAD_AXIS_LEFT_TRIGGER = 4,
    BH_GAMEPAD_AXIS_RIGHT_TRIGGER = 5,
    BH_GAMEPAD_AXIS_LAST = BH_GAMEPAD_AXIS_RIGHT_TRIGGER
} bh_gamepad_axis_t;

// Input action
typedef enum {
    BH_RELEASE = 0,
    BH_PRESS = 1,
    BH_REPEAT = 2
} bh_input_action_t;

// Input state
typedef struct {
    // Keyboard state
    uint8_t keys[BH_KEY_LAST + 1];
    
    // Mouse state
    double mouse_x;
    double mouse_y;
    double mouse_scroll_x;
    double mouse_scroll_y;
    uint8_t mouse_buttons[BH_MOUSE_BUTTON_LAST + 1];
    
    // Gamepad state
    struct {
        uint8_t buttons[BH_GAMEPAD_BUTTON_LAST + 1];
        float axes[BH_GAMEPAD_AXIS_LAST + 1];
        char name[128];
        bool connected;
    } gamepads[BH_JOYSTICK_LAST + 1];
    
    // Text input
    uint32_t codepoint;
    bool text_input_enabled;
} bh_input_state_t;

// Input callback function types
typedef void (*bh_key_callback_t)(bh_key_t key, int scancode, bh_input_action_t action, bh_modifier_t mods);
typedef void (*bh_char_callback_t)(uint32_t codepoint);
typedef void (*bh_cursor_pos_callback_t)(double xpos, double ypos);
typedef void (*bh_cursor_enter_callback_t)(bool entered);
typedef void (*bh_mouse_button_callback_t)(bh_mouse_button_t button, bh_input_action_t action, bh_modifier_t mods);
typedef void (*bh_scroll_callback_t)(double xoffset, double yoffset);
typedef void (*bh_joystick_callback_t)(bh_joystick_id_t jid, int event);

typedef struct {
    // Callbacks
    bh_key_callback_t key_callback;
    bh_char_callback_t char_callback;
    bh_cursor_pos_callback_t cursor_pos_callback;
    bh_cursor_enter_callback_t cursor_enter_callback;
    bh_mouse_button_callback_t mouse_button_callback;
    bh_scroll_callback_t scroll_callback;
    bh_joystick_callback_t joystick_callback;
    
    // State
    bh_input_state_t state;
    
    // Internal state
    bool initialized;
} bh_input_context_t;

/**
 * @brief Initialize the input system
 * 
 * @return bh_status_t Status code
 */
bh_status_t bh_input_initialize(void);

/**
 * @brief Shutdown the input system
 */
void bh_input_shutdown(void);

/**
 * @brief Poll for input events
 * 
 * This should be called once per frame to update input state
 */
void bh_input_poll_events(void);

/**
 * @brief Wait for input events with timeout
 * 
 * @param timeout Timeout in seconds
 */
void bh_input_wait_events(double timeout);

/**
 * @brief Post an empty event to wake up the event loop
 */
void bh_input_post_empty_event(void);

/**
 * @brief Get the current input state
 * 
 * @return const bh_input_state_t* Current input state
 */
const bh_input_state_t* bh_input_get_state(void);

/**
 * @brief Get the key state
 * 
 * @param key Key to check
 * @return bh_input_action_t Key state (RELEASE, PRESS, or REPEAT)
 */
bh_input_action_t bh_input_get_key(bh_key_t key);

/**
 * @brief Get the mouse button state
 * 
 * @param button Button to check
 * @return bh_input_action_t Button state (RELEASE or PRESS)
 */
bh_input_action_t bh_input_get_mouse_button(bh_mouse_button_t button);

/**
 * @brief Get the cursor position
 * 
 * @param xpos [out] X position
 * @param ypos [out] Y position
 */
void bh_input_get_cursor_pos(double* xpos, double* ypos);

/**
 * @brief Set the cursor position
 * 
 * @param xpos X position
 * @param ypos Y position
 */
void bh_input_set_cursor_pos(double xpos, double ypos);

/**
 * @brief Set the cursor mode
 * 
 * @param mode Cursor mode (0 = normal, 1 = hidden, 2 = disabled)
 */
void bh_input_set_cursor_mode(int mode);

/**
 * @brief Check if a joystick is connected
 * 
 * @param jid Joystick ID
 * @return true if connected, false otherwise
 */
bool bh_input_joystick_present(bh_joystick_id_t jid);

/**
 * @brief Get the joystick name
 * 
 * @param jid Joystick ID
 * @return const char* Joystick name, or NULL if not connected
 */
const char* bh_input_get_joystick_name(bh_joystick_id_t jid);

/**
 * @brief Get the joystick axes
 * 
 * @param jid Joystick ID
 * @param count [out] Number of axes
 * @return const float* Array of axis values, or NULL if not connected
 */
const float* bh_input_get_joystick_axes(bh_joystick_id_t jid, int* count);

/**
 * @brief Get the joystick buttons
 * 
 * @param jid Joystick ID
 * @param count [out] Number of buttons
 * @return const uint8_t* Array of button states, or NULL if not connected
 */
const uint8_t* bh_input_get_joystick_buttons(bh_joystick_id_t jid, int* count);

/**
 * @brief Get the joystick hats
 * 
 * @param jid Joystick ID
 * @param count [out] Number of hats
 * @return const uint8_t* Array of hat states, or NULL if not connected
 */
const uint8_t* bh_input_get_joystick_hats(bh_joystick_id_t jid, int* count);

/**
 * @brief Check if a gamepad is connected
 * 
 * @param jid Joystick ID
 * @return true if connected and has a gamepad mapping, false otherwise
 */
bool bh_input_gamepad_present(bh_joystick_id_t jid);

/**
 * @brief Get the gamepad state
 * 
 * @param jid Joystick ID
 * @param state [out] Gamepad state
 * @return true if successful, false otherwise
 */
bool bh_input_get_gamepad_state(bh_joystick_id_t jid, struct bh_gamepad_state* state);

/**
 * @brief Set the clipboard string
 * 
 * @param string String to set
 * @return bh_status_t Status code
 */
bh_status_t bh_input_set_clipboard_string(const char* string);

/**
 * @brief Get the clipboard string
 * 
 * @return const char* Clipboard string, or NULL on failure
 */
const char* bh_input_get_clipboard_string(void);

/**
 * @brief Set the input callbacks
 * 
 * @param callbacks Callback structure
 */
void bh_input_set_callbacks(const bh_input_context_t* callbacks);

/**
 * @brief Get the time in seconds
 * 
 * @return double Time in seconds
 */
double bh_input_get_time(void);

/**
 * @brief Set the time
 * 
 * @param time Time in seconds
 */
void bh_input_set_time(double time);

/**
 * @brief Get the timer value in seconds
 * 
 * @return double Timer value in seconds
 */
double bh_input_get_timer_value(void);

/**
 * @brief Get the timer frequency
 * 
 * @return uint64_t Timer frequency in Hz
 */
uint64_t bh_input_get_timer_frequency(void);

#ifdef __cplusplus
}
#endif

#endif // BLOODHORN_INPUT_H
