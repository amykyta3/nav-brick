#ifndef AVR_ATOMIC_H
#define AVR_ATOMIC_H

// Define a passthrough for atomic blocks
#define ATOMIC_BLOCK(type) for(int i=0; i<1; i++)

#endif
