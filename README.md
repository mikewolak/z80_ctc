# Z80 Counter/Timer Circuit (CTC) Library

## Overview

This library provides a flexible, feature-complete emulation of the Zilog Z80 Counter/Timer Circuit (CTC). It supports both timer and counter modes, interrupt handling, and provides a close approximation of the original Z80 CTC's functionality.

## Features

- Four independent channels
- Timer and Counter modes
- Configurable prescaler (16/256)
- Interrupt handling
- Flexible trigger edge selection
- Callback support
- Interrupt vector configuration

## Building the Library

### Prerequisites

- GCC
- Make

### Compilation

```bash
# Build the library and examples
make

# Clean build artifacts
make clean
```

## Usage Example

```c
#include <z80_ctc.h>

int main() {
    // Create CTC instance with 4 MHz system clock
    Z80_CTC ctc;
    z80_ctc_init(&ctc, 4000000);
    
    // Configure Channel 0 as a timer with interrupts
    z80_ctc_write_control(&ctc, 0, 0x86);
    
    // Set interrupt vector
    z80_ctc_set_interrupt_vector(&ctc, 0, 0x10);
    
    // Set time constant
    z80_ctc_write_time_constant(&ctc, 0, 100);
    
    // Simulate clock ticks
    while (1) {
        z80_ctc_clock_tick(&ctc);
        
        // Handle interrupts
        if (z80_ctc_has_pending_interrupt(&ctc)) {
            z80_ctc_acknowledge_interrupt(&ctc);
        }
    }
    
    return 0;
}
```

## Configuration

The library supports various configuration options through its API:
- Channel mode selection (timer/counter)
- Prescaler configuration
- Trigger edge selection
- Interrupt handling
- Callback registration

## License

-- FREE AS IN BEER! --[

## Contributing

Contributions are welcome! Please submit pull requests or open issues on the project repository.

## Limitations

This is a software emulation and may not perfectly replicate hardware timing characteristics.
