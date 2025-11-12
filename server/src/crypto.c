/*
 * OTT Streaming Server - Cryptography Module
 * Enhancement Phase 2: Password hashing with SHA-256
 *
 * Uses OpenSSL for SHA-256 hashing
 */

#include "../include/crypto.h"
#include <openssl/sha.h>
#include <string.h>
#include <stdio.h>

/**
 * Generate SHA-256 hash of input string
 */
void sha256_hash(const char* input, char* output_hex) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    // Compute SHA-256 hash
    SHA256((unsigned char*)input, strlen(input), hash);

    // Convert binary hash to hex string
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output_hex + (i * 2), "%02x", hash[i]);
    }
    output_hex[64] = '\0';
}

/**
 * Constant-time string comparison to prevent timing attacks
 *
 * Uses volatile variable to prevent compiler optimization that could
 * introduce timing variations based on input differences.
 *
 * @param a First string to compare
 * @param b Second string to compare
 * @param len Length to compare (should be SHA256_HEX_LENGTH for password hashes)
 * @return 1 if strings are equal, 0 if different
 *
 * Security note: This function takes the same amount of time regardless of
 * where the strings differ, preventing timing attacks that could be used
 * to progressively guess password hashes.
 */
static int constant_time_compare(const char* a, const char* b, size_t len) {
    volatile unsigned char result = 0;

    for (size_t i = 0; i < len; i++) {
        result |= (unsigned char)a[i] ^ (unsigned char)b[i];
    }

    return result == 0;
}

/**
 * Verify password against stored hash
 *
 * Uses constant-time comparison to prevent timing attacks.
 */
int verify_password(const char* input_password, const char* stored_hash) {
    char computed_hash[SHA256_HEX_LENGTH];

    // Hash the input password
    sha256_hash(input_password, computed_hash);

    // Use constant-time comparison to prevent timing attacks
    return constant_time_compare(computed_hash, stored_hash, SHA256_HEX_LENGTH - 1);
}
