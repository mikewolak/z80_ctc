#include <stdio.h>
#include "z80_ctc.h"

// Global interrupt count tracker
static int interrupt_count = 0;

// Example callback function for timer expiration
static void timer_expired_callback(void* context) {
    int* timer_id = (int*)context;
    printf("Timer %d expired (callback)!\n", *timer_id);
}

// Global interrupt handler
static uint8_t global_interrupt_handler(uint8_t channel_num, void* context) {
    Z80_CTC* ctc = (Z80_CTC*)context;
    
    // Increment global interrupt count
    interrupt_count++;
    
    printf("Interrupt on channel %d (vector: 0x%02X, count: %d)\n", 
           channel_num, 
           z80_ctc_get_interrupt_vector(ctc, channel_num),
           interrupt_count);
    
    // Return the interrupt vector for the channel
    return z80_ctc_get_interrupt_vector(ctc, channel_num);
}

int main() {
    // Create CTC instance with 4 MHz system clock
    Z80_CTC ctc;
    z80_ctc_init(&ctc, 4000000);
    
    // Set global interrupt handler
    z80_ctc_set_global_interrupt_handler(&ctc, global_interrupt_handler, &ctc);
    
    // Timer context for callbacks
    int timer0_id = 0;
    int timer1_id = 1;
    
    // Configure Channel 0 as a timer with interrupts
    // Set control word: interrupt enabled, timer mode, 16 prescaler, falling edge
    z80_ctc_write_control(&ctc, 0, 0x86);  // Binary: 10000110
    
    // Set interrupt vector for channel 0
    z80_ctc_set_interrupt_vector(&ctc, 0, 0x10);
    
    // Set time constant to 100 (will trigger every 100 clock ticks)
    z80_ctc_write_time_constant(&ctc, 0, 100);
    
    // Set callback for timer 0
    z80_ctc_set_callback(&ctc, 0, timer_expired_callback, &timer0_id);
    
    // Configure Channel 1 as a counter with interrupts
    // Set control word: interrupt enabled, counter mode, rising edge
    z80_ctc_write_control(&ctc, 1, 0xC4);  // Binary: 11000100
    
    // Set interrupt vector for channel 1
    z80_ctc_set_interrupt_vector(&ctc, 1, 0x20);
    
    // Set time constant to 10 (will trigger every 10 external trigger events)
    z80_ctc_write_time_constant(&ctc, 1, 10);
    
    // Set callback for timer 1
    z80_ctc_set_callback(&ctc, 1, timer_expired_callback, &timer1_id);
    
    // Simulate clock ticks for timer mode channel (Channel 0)
    printf("Simulating clock ticks for Timer mode...\n");
    for (int i = 0; i < 500; i++) {
        z80_ctc_clock_tick(&ctc);
        
        // Simulate interrupt handling periodically
        if (z80_ctc_has_pending_interrupt(&ctc)) {
            z80_ctc_acknowledge_interrupt(&ctc);
        }
    }
    
    // Simulate external trigger events for counter mode channel (Channel 1)
    printf("\nSimulating external trigger events for Counter mode...\n");
    for (int i = 0; i < 50; i++) {
        z80_ctc_trigger_input(&ctc, 1, false);  // Falling edge
        z80_ctc_trigger_input(&ctc, 1, true);   // Rising edge
        
        // Simulate interrupt handling periodically
        if (z80_ctc_has_pending_interrupt(&ctc)) {
            z80_ctc_acknowledge_interrupt(&ctc);
        }
    }
    
    printf("\nTotal interrupt count: %d\n", interrupt_count);
    
    return 0;
}
