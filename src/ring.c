
#include <stdint.h>
#include "main.h"
#include "ring.h"

bool_t ring_empty(const struct ring *ring)
{
    return (ring->head == ring->tail);
}

bool_t ring_full(const struct ring *ring)
{
    int next = (ring->head + 1) % MAX_CHARS;
    return (next == ring->tail);
}

void ring_put(struct ring *ring, uint8_t bits)
{
    uint32_t next = (ring->head + 1) % MAX_CHARS;
    ring->buffer[ring->head] = bits;
    ring->head = next;
}

uint8_t ring_get(struct ring *ring)
{
    uint8_t bits;
    uint32_t next = (ring->tail + 1) % MAX_CHARS;
    bits = ring->buffer[ring->tail];
    ring->tail = next;
    return bits;
}

void ring_init(struct ring *ring)
{
    ring->head = 0;
    ring->tail = 0;
}
