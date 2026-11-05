#ifndef RSA_DEMO_H
#define RSA_DEMO_H

#include <openssl/evp.h>

/*
 * Message format, used in both directions:
 *   2 bytes   length of everything after these 2 bytes (big endian)
 *   1 byte    message type: MSG_IPEK_REQUEST or MSG_IPEK_RESPONSE
 *   1 byte    version
 *   request:  5 bytes key set id, then the client's public key in PEM format
 *   response: 1 byte ACK, then the encrypted IPEK
 */
#define MSG_IPEK_REQUEST  0x01
#define MSG_IPEK_RESPONSE 0x02
#define MSG_VERSION       0x01
#define MSG_ACK           0x06
#define KEY_SET_ID_LEN    5
#define MAX_PAYLOAD       4096
#define MIN_RSA_BITS      2048
#define IPEK_LEN          16

EVP_PKEY *rsa_generate_key(int bits);
int rsa_public_key_pem(EVP_PKEY *key, char *pem, int size);

// Both return the output length, or -1 on error. Padding is OAEP with SHA-256.
int rsa_public_encrypt(const unsigned char *data, int len, const char *pem, int pemLen, unsigned char *out, int outSize);
int rsa_private_decrypt(const unsigned char *data, int len, EVP_PKEY *key, unsigned char *out, int outSize);

void printLastError(const char *msg);

// Return the payload length, or -1 on error or disconnect
int read_frame(int fd, unsigned char *payload, int size);
int write_frame(int fd, const unsigned char *payload, int len);

#endif
