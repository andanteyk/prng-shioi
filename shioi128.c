/* Shioi128 v1.0 - pseudorandom number generator

To the extent possible under law, the author has waived all copyright 
and related or neighboring rights to this software.
See: https://creativecommons.org/publicdomain/zero/1.0/
*/

#include <inttypes.h>

// Rotate left (ROL) instruction
static inline uint64_t rotl(uint64_t x, int k)
{
    return (x << k) | (x >> (-k & 0x3f));
}

static uint64_t s[2];

// Returns 64-bit pseudorandom value.
uint64_t next(void)
{
    uint64_t s0 = s[0], s1 = s[1];
    uint64_t result = rotl(s0 * 0xD2B74407B1CE6E93, 29) + s1;

    s[0] = s1;
    s[1] = (s0 << 2) ^ ((int64_t)s0 >> 19) ^ s1;

    return result;
}

// Initialize state.
void init(uint64_t seed)
{
    // You may initialize state by any method, unless state will be {0, 0}.
    for (int i = 0; i < 2; i++)
    {
        s[i] = seed = seed * 6364136223846793005 + 1442695040888963407;
    }
}

static inline void jump(const uint64_t jumppoly[])
{
    uint64_t t[2] = {0, 0};

    for (int i = 0; i < 2; i++)
    {
        for (int b = 0; b < 64; b++)
        {
            if ((jumppoly[i] >> b) & 1)
            {
                t[0] ^= s[0];
                t[1] ^= s[1];
            }
            next();
        }
    }

    s[0] = t[0];
    s[1] = t[1];
}

// It is equivalent to 2^32 calls of next().
void jump32(void)
{
    const uint64_t jumppoly_2_32[] = {0x8003A4B944F009D0, 0x7FFE925EEBD5615B};
    jump(jumppoly_2_32);
}

// It is equivalent to 2^64 calls of next().
void jump64(void)
{
    // It is equivalent to jump({ 0x3, 0 })
    uint64_t s0 = s[0], s1 = s[1];

    s[0] = s0 ^ s1;
    s[1] = (s0 << 2) ^ ((int64_t)s0 >> 19);
}

// It is equivalent to 2^96 calls of next().
void jump96(void)
{
    const uint64_t jumppoly_2_96[] = {0x8003A4B944F009D1, 0x7FFE925EEBD5615B};
    jump(jumppoly_2_96);
}

//////// The test code is shown below. ////////

#include <stdio.h>

#define ASSERT(pred)               \
    if (!(pred))                   \
    {                              \
        puts("assertion failed."); \
        return -1;                 \
    }
#define PRINTSTATE(title) printf(title "%016" PRIx64 " %016" PRIx64 "\n", s[0], s[1])

int main(void)
{
    init(401);
    PRINTSTATE("init: ");
    ASSERT(s[0] == 0x6C64F673ED93B6CC && s[1] == 0x97C703D5F6C9D72B);

    printf("next: ");
    {
        const uint64_t answers[] = {
            0xF8D7B7BA91C4D17A, 0xB053788D02AE0471,
            0xF6F7467B5C631C8A, 0x8F109E92A5905420};
        for (int i = 0; i < 4; i++)
        {
            uint64_t value = next();
            printf("%016" PRIx64 " ", value);
            ASSERT(value == answers[i]);
        }
    }
    printf("\n");

    PRINTSTATE("jp 0: ");
    ASSERT(s[0] == 0x1FE470A806C38EB1 && s[1] == 0xFAC7289977D6FD63);

    jump32();
    PRINTSTATE("jp32: ");
    ASSERT(s[0] == 0x985B17ADA536684C && s[1] == 0x2CECBEFC3FB03DF8);

    jump64();
    PRINTSTATE("jp64: ");
    ASSERT(s[0] == 0xB4B7A9519A8655B4 && s[1] == 0x9E93ADBDF62C1596);

    jump96();
    PRINTSTATE("jp96: ");
    ASSERT(s[0] == 0x67EA4FFD18216615 && s[1] == 0x696B13B974BFBFF7);

    return 0;
}
