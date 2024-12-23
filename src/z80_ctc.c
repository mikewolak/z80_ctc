#include "z80_ctc.h"
#include <string.h>
#include <stdio.h>

// Bitfield masks for control register
#define CTC_CTRL_INTERRUPT_MASK     0x80
#define CTC_CTRL_MODE_MASK          0x40
#define CTC_CTRL_PRESCALER_MASK     0x20
#define CTC_CTRL_TRIGGER_EDGE_MASK  0x10
#define CTC_CTRL_TRIGGER_MODE_MASK  0x08
#define CTC_CTRL_TIME_CONSTANT_MASK 0x04
#define CTC_CTRL_RESET_MASK         0x02
#define CTC_CTRL_CONTROL_VECTOR_MASK 0x01

// Default logging function (to stdout)
#ifdef CTC_DEBUG
static void default_log_function(const char* message) {
    fprintf(stderr, "%s", message);
}
#endif

// Interrupt-related functions
void z80_ctc_set_interrupt_vector(Z80_CTC* ctc, uint8_t channel_num, uint8_t vector) {
    if (channel_num > 3) return;
    
    ctc->channels[channel_num].interrupt_vector = vector;
}

void z80_ctc_set_global_interrupt_handler(Z80_CTC* ctc, 
                                           Z80_CTC_InterruptHandler handler, 
                                           void* context) {
    ctc->interrupt_handler = handler;
    ctc->interrupt_context = context;
}

uint8_t z80_ctc_get_interrupt_vector(Z80_CTC* ctc, uint8_t channel_num) {
    if (channel_num > 3) return 0;
    
    return ctc->channels[channel_num].interrupt_vector;
}

bool z80_ctc_has_pending_interrupt(Z80_CTC* ctc) {
    for (int i = 0; i < 4; i++) {
        if (ctc->channels[i].interrupt_enabled && 
            ctc->channels[i].interrupt_pending) {
            return true;
        }
    }
    return false;
}

uint8_t z80_ctc_acknowledge_interrupt(Z80_CTC* ctc) {
    // Check channels in priority order (0 is highest priority)
    for (int i = 0; i < 4; i++) {
        Z80_CTC_Channel* channel = &ctc->channels[i];
        
        // Check if this channel has a pending interrupt
        if (channel->interrupt_enabled && channel->interrupt_pending) {
            // Clear the pending interrupt
            channel->interrupt_pending = false;
            
            // If a global interrupt handler is set, call it
            if (ctc->interrupt_handler) {
                return ctc->interrupt_handler(i, ctc->interrupt_context);
            }
            
            // Otherwise, return the channel's interrupt vector
            return channel->interrupt_vector;
        }
    }
    
    // No pending interrupts
    return 0;
}

#ifdef CTC_DEBUG
void z80_ctc_set_log_function(Z80_CTC* ctc, Z80_CTC_LogFunction log_func) {
    ctc->log_function = log_func ? log_func : default_log_function;
}

// Internal debug logging function
static void debug_log_channel_state(Z80_CTC* ctc, uint8_t channel_num) {
    char buffer[512];
    Z80_CTC_Channel* channel = &ctc->channels[channel_num];
    
    snprintf(buffer, sizeof(buffer), 
        "CTC Channel %d State:\n"
        "  Mode: %s\n"
        "  Prescaler: %d\n"
        "  Trigger Edge: %s\n"
        "  Interrupt Enabled: %s\n"
        "  Interrupt Pending: %s\n"
        "  Running: %s\n"
        "  Control Register: 0x%02X\n"
        "  Time Constant: %d\n"
        "  Current Count: %d\n"
        "  Trigger Count: %d\n"
        "  Last Trigger State: %s\n"
        "  Interrupt Vector: 0x%02X\n\n",
        channel_num,
        channel->mode == CTC_MODE_TIMER ? "Timer" : "Counter",
        channel->prescaler,
        channel->trigger_edge == CTC_TRIGGER_RISING_EDGE ? "Rising" : "Falling",
        channel->interrupt_enabled ? "Yes" : "No",
        channel->interrupt_pending ? "Yes" : "No",
        channel->is_running ? "Yes" : "No",
        channel->control_register,
        channel->time_constant,
        channel->current_count,
        channel->trigger_count,
        channel->last_trigger_state ? "High" : "Low",
        channel->interrupt_vector
    );
    
    // Use the log function
    if (ctc->log_function) {
        ctc->log_function(buffer);
    }
}
#endif

// Initialize the entire CTC
void z80_ctc_init(Z80_CTC* ctc, uint32_t system_clock_frequency) {
    memset(ctc, 0, sizeof(Z80_CTC));
    ctc->system_clock_frequency = system_clock_frequency;
    
    // Initialize all channels to default state
    for (int i = 0; i < 4; i++) {
        ctc->channels[i].mode = CTC_MODE_TIMER;
        ctc->channels[i].prescaler = CTC_PRESCALER_16;
        ctc->channels[i].trigger_edge = CTC_TRIGGER_FALLING_EDGE;
        ctc->channels[i].is_running = false;
        ctc->channels[i].interrupt_enabled = false;
        ctc->channels[i].interrupt_pending = false;
    }

#ifdef CTC_DEBUG
    // Set default log function
    ctc->log_function = default_log_function;
#endif
}

// Write control word to a specific channel
void z80_ctc_write_control(Z80_CTC* ctc, uint8_t channel_num, uint8_t control_word) {
    if (channel_num > 3) return;
    
    Z80_CTC_Channel* channel = &ctc->channels[channel_num];
    
    // Store the control register
    channel->control_register = control_word;
    
    // Interrupt enable
    channel->interrupt_enabled = (control_word & CTC_CTRL_INTERRUPT_MASK) ? true : false;
    
    // Mode selection
    channel->mode = (control_word & CTC_CTRL_MODE_MASK) ? CTC_MODE_COUNTER : CTC_MODE_TIMER;
    
    // Prescaler selection
    channel->prescaler = (control_word & CTC_CTRL_PRESCALER_MASK) ? 
        CTC_PRESCALER_256 : CTC_PRESCALER_16;
    
    // Trigger edge selection
    channel->trigger_edge = (control_word & CTC_CTRL_TRIGGER_EDGE_MASK) ? 
        CTC_TRIGGER_RISING_EDGE : CTC_TRIGGER_FALLING_EDGE;
    
    // Reset handling
    if (control_word & CTC_CTRL_RESET_MASK) {
        channel->current_count = 0;
        channel->is_running = false;
        channel->interrupt_pending = false;
    }
    
    // Time constant follow flag
    if (control_word & CTC_CTRL_TIME_CONSTANT_MASK) {
        // Expect time constant to be written next
        channel->is_running = false;
    }
}

// Write time constant to a specific channel
void z80_ctc_write_time_constant(Z80_CTC* ctc, uint8_t channel_num, uint8_t constant) {
    if (channel_num > 3) return;
    
    Z80_CTC_Channel* channel = &ctc->channels[channel_num];
    
    // Store time constant
    channel->time_constant = constant;
    
    // Reset current count to actual value (256 if constant was 0)
    channel->current_count = (constant == 0) ? 256 : constant;
    channel->is_running = true;
    channel->interrupt_pending = false;
}

// Set callback for a specific channel
void z80_ctc_set_callback(Z80_CTC* ctc, uint8_t channel_num, 
                           Z80_CTC_Callback callback, void* context) {
    if (channel_num > 3) return;
    
    Z80_CTC_Channel* channel = &ctc->channels[channel_num];
    channel->expiration_callback = callback;
    channel->callback_context = context;
}

// Handle trigger input for a specific channel
void z80_ctc_trigger_input(Z80_CTC* ctc, uint8_t channel_num, bool trigger_state) {
    if (channel_num > 3) return;
    
    Z80_CTC_Channel* channel = &ctc->channels[channel_num];
    
    // Only process if in counter mode
    if (channel->mode != CTC_MODE_COUNTER) return;
    
    // Check trigger edge condition
    bool trigger_condition = 
        (channel->trigger_edge == CTC_TRIGGER_RISING_EDGE && 
         !channel->last_trigger_state && trigger_state) ||
        (channel->trigger_edge == CTC_TRIGGER_FALLING_EDGE && 
         channel->last_trigger_state && !trigger_state);
    
    // If trigger condition met and channel is running
    if (trigger_condition && channel->is_running) {
        // Decrement counter
        if (channel->current_count > 0) {
            channel->current_count--;
        }
        
        // Check for zero count
        if (channel->current_count == 0) {
            // Reload counter to actual value (256 if time_constant is 0)
            channel->current_count = (channel->time_constant == 0) ? 256 : channel->time_constant;
            
            // Set interrupt pending if interrupts are enabled
            if (channel->interrupt_enabled) {
                channel->interrupt_pending = true;
            }
            
            // Trigger callback if set
            if (channel->expiration_callback) {
                channel->expiration_callback(channel->callback_context);
            }
            
            // Debug logging
#ifdef CTC_DEBUG
            debug_log_channel_state(ctc, channel_num);
#endif
        }
    }
    
    // Update last trigger state
    channel->last_trigger_state = trigger_state;
}

// Simulate clock tick for timer mode channels
void z80_ctc_clock_tick(Z80_CTC* ctc) {
    for (int i = 0; i < 4; i++) {
        Z80_CTC_Channel* channel = &ctc->channels[i];
        
        // Skip if not in timer mode or not running
        if (channel->mode != CTC_MODE_TIMER || !channel->is_running) 
            continue;
        
        // Apply prescaler
        channel->trigger_count++;
        if (channel->trigger_count < channel->prescaler)
            continue;
        
        // Reset prescaler counter
        channel->trigger_count = 0;
        
        // Decrement counter
        if (channel->current_count > 0) {
            channel->current_count--;
        }
        
        // Check for zero count
        if (channel->current_count == 0) {
            // Reload counter to actual value (256 if time_constant is 0)
            channel->current_count = (channel->time_constant == 0) ? 256 : channel->time_constant;
            
            // Set interrupt pending if interrupts are enabled
            if (channel->interrupt_enabled) {
                channel->interrupt_pending = true;
            }
            
            // Trigger callback if set
            if (channel->expiration_callback) {
                channel->expiration_callback(channel->callback_context);
            }
            
            // Debug logging
#ifdef CTC_DEBUG
            debug_log_channel_state(ctc, i);
#endif
        }
    }
}
