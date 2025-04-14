#ifndef RING_H_
#define RING_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX_CHARS 1024

struct ring
{
    volatile uint8_t buffer[MAX_CHARS];
    volatile uint32_t head;
    volatile uint32_t tail;
};

void ring_init(struct ring *ring);
bool_t ring_empty(const struct ring *ring);
bool_t ring_full(const struct ring *ring);
void ring_put(struct ring *ring, uint8_t bits);
uint8_t ring_get(struct ring *ring);

#endif // RING_H_