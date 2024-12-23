#ifndef Z80_CTC_H
#define Z80_CTC_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Compile-time debug flag
// Uncomment or add -DCTC_DEBUG to compiler flags to enable verbose logging
// #define CTC_DEBUG

// Prescaler options
typedef enum {
    CTC_PRESCALER_16 = 16,
    CTC_PRESCALER_256 = 256
} Z80_CTC_Prescaler;

// Operation modes
typedef enum {
    CTC_MODE_TIMER,
    CTC_MODE_COUNTER
} Z80_CTC_Mode;

// Channel trigger edges
typedef enum {
    CTC_TRIGGER_FALLING_EDGE,
    CTC_TRIGGER_RISING_EDGE
} Z80_CTC_TriggerEdge;

// Logging function type for debug output
typedef void (*Z80_CTC_LogFunction)(const char* message);

// Interrupt handling type
typedef uint8_t (*Z80_CTC_InterruptHandler)(uint8_t channel_num, void* context);

// Callback type for timer expiration or event
typedef void (*Z80_CTC_Callback)(void* context);

// Channel configuration structure
typedef struct {
    // Configuration parameters
    Z80_CTC_Mode mode;
    Z80_CTC_Prescaler prescaler;
    Z80_CTC_TriggerEdge trigger_edge;
    bool interrupt_enabled;
    
    // Interrupt vector
    uint8_t interrupt_vector;
    
    // Internal state
    uint8_t control_register;
    uint8_t time_constant;  // Stored as 0-255, representing 1-256
    uint16_t current_count; // Allows full range of 1-256
    bool is_running;
    bool interrupt_pending;
    
    // Trigger and event handling
    bool last_trigger_state;
    uint8_t trigger_count;
    
    // Callback for timer expiration
    Z80_CTC_Callback expiration_callback;
    void* callback_context;
} Z80_CTC_Channel;

// Main CTC structure representing all 4 channels
typedef struct {
    Z80_CTC_Channel channels[4];
    uint32_t system_clock_frequency; // Base system clock in Hz
    
    // Global interrupt handler
    Z80_CTC_InterruptHandler interrupt_handler;
    void* interrupt_context;

    // Debug logging
#ifdef CTC_DEBUG
    Z80_CTC_LogFunction log_function;
#endif
} Z80_CTC;

// Function prototypes
void z80_ctc_init(Z80_CTC* ctc, uint32_t system_clock_frequency);
void z80_ctc_write_control(Z80_CTC* ctc, uint8_t channel_num, uint8_t control_word);
void z80_ctc_write_time_constant(Z80_CTC* ctc, uint8_t channel_num, uint8_t constant);
void z80_ctc_set_callback(Z80_CTC* ctc, uint8_t channel_num, 
                           Z80_CTC_Callback callback, void* context);
void z80_ctc_trigger_input(Z80_CTC* ctc, uint8_t channel_num, bool trigger_state);
void z80_ctc_clock_tick(Z80_CTC* ctc);

// Interrupt-related functions
void z80_ctc_set_interrupt_vector(Z80_CTC* ctc, uint8_t channel_num, uint8_t vector);
void z80_ctc_set_global_interrupt_handler(Z80_CTC* ctc, 
                                           Z80_CTC_InterruptHandler handler, 
                                           void* context);
uint8_t z80_ctc_get_interrupt_vector(Z80_CTC* ctc, uint8_t channel_num);
bool z80_ctc_has_pending_interrupt(Z80_CTC* ctc);
uint8_t z80_ctc_acknowledge_interrupt(Z80_CTC* ctc);

// Debug logging function
#ifdef CTC_DEBUG
void z80_ctc_set_log_function(Z80_CTC* ctc, Z80_CTC_LogFunction log_func);
#endif

#ifdef __cplusplus
}
#endif

#endif // Z80_CTC_H
#ifndef Z80_CTC_H
#define Z80_CTC_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Prescaler options
typedef enum {
    CTC_PRESCALER_16 = 16,
    CTC_PRESCALER_256 = 256
} Z80_CTC_Prescaler;

// Operation modes
typedef enum {
    CTC_MODE_TIMER,
    CTC_MODE_COUNTER
} Z80_CTC_Mode;

// Channel trigger edges
typedef enum {
    CTC_TRIGGER_FALLING_EDGE,
    CTC_TRIGGER_RISING_EDGE
} Z80_CTC_TriggerEdge;

// Interrupt handling type
typedef uint8_t (*Z80_CTC_InterruptHandler)(uint8_t channel_num, void* context);

// Callback type for timer expiration or event
typedef void (*Z80_CTC_Callback)(void* context);

// Channel configuration structure
typedef struct {
    // Configuration parameters
    Z80_CTC_Mode mode;
    Z80_CTC_Prescaler prescaler;
    Z80_CTC_TriggerEdge trigger_edge;
    bool interrupt_enabled;
    
    // Interrupt vector
    uint8_t interrupt_vector;
    
    // Internal state
    uint8_t control_register;
    uint8_t time_constant;  // Stored as 0-255, representing 1-256
    uint16_t current_count; // Allows full range of 1-256
    bool is_running;
    bool interrupt_pending;
    
    // Trigger and event handling
    bool last_trigger_state;
    uint8_t trigger_count;
    
    // Callback for timer expiration
    Z80_CTC_Callback expiration_callback;
    void* callback_context;
} Z80_CTC_Channel;

// Main CTC structure representing all 4 channels
typedef struct {
    Z80_CTC_Channel channels[4];
    uint32_t system_clock_frequency; // Base system clock in Hz
    
    // Global interrupt handler
    Z80_CTC_InterruptHandler interrupt_handler;
    void* interrupt_context;
} Z80_CTC;

// Function prototypes
void z80_ctc_init(Z80_CTC* ctc, uint32_t system_clock_frequency);
void z80_ctc_write_control(Z80_CTC* ctc, uint8_t channel_num, uint8_t control_word);
void z80_ctc_write_time_constant(Z80_CTC* ctc, uint8_t channel_num, uint8_t constant);
void z80_ctc_set_callback(Z80_CTC* ctc, uint8_t channel_num, 
                           Z80_CTC_Callback callback, void* context);
void z80_ctc_trigger_input(Z80_CTC* ctc, uint8_t channel_num, bool trigger_state);
void z80_ctc_clock_tick(Z80_CTC* ctc);

// Interrupt-related functions
void z80_ctc_set_interrupt_vector(Z80_CTC* ctc, uint8_t channel_num, uint8_t vector);
void z80_ctc_set_global_interrupt_handler(Z80_CTC* ctc, 
                                           Z80_CTC_InterruptHandler handler, 
                                           void* context);
uint8_t z80_ctc_get_interrupt_vector(Z80_CTC* ctc, uint8_t channel_num);
bool z80_ctc_has_pending_interrupt(Z80_CTC* ctc);
uint8_t z80_ctc_acknowledge_interrupt(Z80_CTC* ctc);

#ifdef __cplusplus
}
#endif

#endif // Z80_CTC_H
