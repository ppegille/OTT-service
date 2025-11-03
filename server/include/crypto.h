#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>

// SHA-256 hash output is 64 hex characters + null terminator
#define SHA256_HEX_LENGTH 65

/**
 * Generate SHA-256 hash of input string
 *
 * @param input Input string to hash
 * @param output_hex Output buffer (must be at least 65 bytes)
 */
void sha256_hash(const char* input, char* output_hex);

/**
 * Verify password against stored hash
 *
 * @param input_password Plain text password
 * @param stored_hash SHA-256 hash to compare against
 * @return 1 if match, 0 if no match
 */
int verify_password(const char* input_password, const char* stored_hash);

#endif // CRYPTO_H
