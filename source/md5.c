/*
 * The MD5 algorithm
 *
 * Credit: https://en.wikipedia.org/wiki/MD5
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define LENDIAN(WORD) ((((WORD) << 24) & 0xff000000) | \
                       (((WORD) <<  8) & 0x00ff0000) | \
                       (((WORD) >>  8) & 0x0000ff00) | \
                       (((WORD) >> 24) & 0x000000ff))

/*
 * Create a MD5 hash from the inputted abcd-values
 *
 * PARAMS
 * - char hash[32]        | A pointer to the "will be created"-hash
 * - const uint32 abcd[4] | The abcd-values which to create the hash from
 *
 * RETURN (char* hash)
 */
static char* abcd_hash(char hash[32], const uint32_t abcd[4])
{
  for(uint8_t index = 0; index < 4; index++)
  {
    // hash + (index * 8) points to the current 8 characters
    sprintf(hash + (index * 8), "%08x", LENDIAN(abcd[index]));
  }
  return hash;
}

#define LSHIFT(a, b) ((a) << (b))
#define RSHIFT(a, b) ((a) >> (b))

#define LROTATE(a, b) (LSHIFT(a, b) | RSHIFT(a, 32 - (b)))

static const uint32_t k[64] = {
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
  0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
  0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
  0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
  0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
  0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
  0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
  0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
  0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
  0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
  0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
  0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

const uint8_t s[16] = {
  7, 12, 17, 22,
  5,  9, 14, 20,
  4, 11, 16, 23,
  6, 10, 15, 21
};

#define F(B, C, D) (((B) & (C)) | (~(B) & (D)))
#define G(B, C, D) (((B) & (D)) | ((C) & ~(D)))
#define H(B, C, D) ((B) ^ (C) ^ (D))
#define I(B, C, D) ((C) ^ ((B) | ~(D)))

/*
 * Update the abcd-values with the bits in the inputted chunk
 *
 * PARAMS
 * - uint32_t abcd[4]         | The "will be updated" abcd-values
 * - const uint32_t chunk[16] | The current chunk from the block
 */
static void abcd_chunk_update(uint32_t abcd[4], const uint32_t chunk[16])
{
  // Initialize hash value for this chunk:
  uint32_t a = abcd[0];
  uint32_t b = abcd[1];
  uint32_t c = abcd[2];
  uint32_t d = abcd[3];

  for(uint8_t i = 0; i < 64; i++)
  {
    uint32_t f;
    uint8_t g;

    if(i >= 0 && i < 16)
    {
      f = F(b, c, d);
      g = i;
    }
    else if(i >= 16 && i < 32)
    {
      f = G(b, c, d);
      g = ((5 * i) + 1) & 0b1111;
    }
    else if(i >= 32 && i < 48)
    {
      f = H(b, c, d);
      g = ((3 * i) + 5) & 0b1111;
    }
    else // if(i >= 48 && i < 63)
    {
      f = I(b, c, d);
      g = (7 * i) & 0b1111;
    }

    uint8_t si = ((i >> 4) << 2) + (i & 0b11);

    f += a + k[i] + chunk[g];

    a = d;
    d = c;
    c = b;
    b += LROTATE(f, s[si]);
  }
  // Add this chunk's hash to result so far:
  abcd[0] += a;
  abcd[1] += b;
  abcd[2] += c;
  abcd[3] += d;
}

/*
 * These bit manipulation macros is ment for 32-bit words
 *
 * BIT refers to the bit in the words at a specific index
 */
// Note: (BIT >> 5) is equivalent to (BIT / 32)
#define BIT_WORD(WORDS, BIT) (WORDS)[(BIT) >> 5]
// Note: (BIT & 0b11111) is equivalent to (BIT % 32)
#define WORD_BIT(BIT) (31 - ((BIT) & 0b11111))

#define BIT_SET(WORDS, BIT) (BIT_WORD(WORDS, BIT) |=  LSHIFT(1, WORD_BIT(BIT)))
#define BIT_OFF(WORDS, BIT) (BIT_WORD(WORDS, BIT) &= ~LSHIFT(1, WORD_BIT(BIT)))

/*
 * Prepend the binary representation of the message to the message block
 *
 * PARAMS
 * - uint32_t* block     | The message block to prepend the message to
 * - const void* message | The message to prepend
 * - size_t size         | The amount of bytes (8 bits)
 */
static void block_message_prepend(uint32_t* block, const void* message, size_t size)
{
  for(size_t index = 0; index < size; index++)
  {
    for(uint8_t bit = 0; bit < 8; bit++)
    {
      char word = ((char*) message)[index];

      if(word & LSHIFT(1, (7 - bit)))
      {
        BIT_SET(block, (index * 8) + bit);
      }
      else BIT_OFF(block, (index * 8) + bit);
    }
  }
}

// This is the equivalent to ceil(size / 64)
#define INIT_CHUNKS(SIZE) (((SIZE) & 0b111111) ? ((SIZE) >> 6) + 1 : (SIZE) >> 6)

/*
 * Check if the size is between 56 and 64 bytes
 * That is when you have to add an extra chunk
 */
#define EXTRA_CHUNK(SIZE) (((SIZE) & 0b111000) == 0b111000)
/*
 * Calculates the amount of message bits in the last chunk
 */
#define MOD_BITS(SIZE) (((SIZE) & 0b111111) * 8)

#define CHUNKS(SIZE) (((SIZE) == 0) ? 1 : (EXTRA_CHUNK(SIZE) ? (INIT_CHUNKS(SIZE) + 1) : INIT_CHUNKS(SIZE)))
#define ZEROS(SIZE) (((SIZE) == 0) ? 448 : (EXTRA_CHUNK(SIZE) ? (959 - MOD_BITS(SIZE)) : (447 - MOD_BITS(SIZE))))

/*
 * Create the message block needed to generate the MD5 hash
 *
 * PARAMS
 * - uint32_t* block     | A pointer to the beginning of the block
 * - size_t chunks       | The amount of chunks in the block
 * - uint16_t zeros      | The amount of zeros between the message and the length
 * - const void* message | The message to hash
 * - size_t size         | The amount of bytes (8 bits)
 */
static void block_create(uint32_t* block, size_t chunks, uint16_t zeros, const void* message, size_t size)
{
  // 1. Copy the encoded message to the message block
  block_message_prepend(block, message, size);

  // 2. Append a single '1' to the encoded message
  BIT_SET(block, size * 8);

  // 3. Add zeros between the encoded message and the length integer
  for(size_t index = 0; index < zeros; index++)
  {
    BIT_OFF(block, size * 8 + 1 + index);
  }

  // 5. Convert 32-bit words to little endian 32-bit words
  for(size_t index = 0; index < (chunks * 16) - 2; index++)
  {
    block[index] = LENDIAN(block[index]);
  }

  // 6. Copy binary representation of length to end of block
  // The length is the amount of bits (1 byte = 8 bits)
  uint64_t length = (uint64_t) size * 8;

  block[(chunks * 16) - 1] = (uint32_t) (length >> 32);
  block[(chunks * 16) - 2] = (uint32_t) length;
}

/*
 * Create a MD5 hash of the inputted message block
 *
 * PARAMS
 * - char hash[64]         | A pointer to the "will be created"-hash
 * - const uint32_t* block | The block to hash 
 * - size_t chunks         | The amount of chunks (512 bits)
 *
 * RETURN (char* hash)
 */
static char* block_hash(char hash[32], const uint32_t* block, size_t chunks)
{
  uint32_t abcd[4] = {
    0x67452301,
    0xefcdab89,
    0x98badcfe,
    0x10325476
  };

  for(size_t index = 0; index < chunks; index++)
  {
    // block + (index * 16) points to the current chunk
    abcd_chunk_update(abcd, block + (index * 16));
  }

  return abcd_hash(hash, abcd);
}

extern char* md5(char hash[32], const void* message, size_t size)
{
  size_t chunks = CHUNKS(size);
  uint16_t zeros = ZEROS(size);

  uint32_t block[chunks * 16];

  block_create(block, chunks, zeros, message, size);

  return block_hash(hash, block, chunks);
}
