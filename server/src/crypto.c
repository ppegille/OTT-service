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
 * Verify password against stored hash
 */
int verify_password(const char* input_password, const char* stored_hash) {
    char computed_hash[SHA256_HEX_LENGTH];

    // Hash the input password
    sha256_hash(input_password, computed_hash);

    // Compare with stored hash (constant-time comparison would be better for production)
    return (strcmp(computed_hash, stored_hash) == 0) ? 1 : 0;
}
