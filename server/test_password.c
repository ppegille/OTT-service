/*
 * Test program to generate correct SHA-256 hash for "password123"
 */

#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

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

int main() {
    char hash[65];
    const char* password = "password123";

    sha256_hash(password, hash);

    printf("Password: %s\n", password);
    printf("SHA-256 Hash: %s\n", hash);
    printf("\nSQL to insert user:\n");
    printf("INSERT INTO users (username, password_hash) VALUES ('alice', '%s');\n", hash);

    return 0;
}
