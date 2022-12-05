#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <sys/stat.h>   // stat
#include <stdbool.h>    // bool type
#include <arpa/inet.h>
#include <openssl/evp.h>

#ifndef ENCRYPT_H
#define ENCRYPT_H

#define KEY_LENGTH  2048
#define PUB_EXP     65537   

bool file_exists (char *filename);
void log_ssl_err(const char *mes);
int get_keys(RSA **keypair);
int encrypt_message(RSA *rsa, char msg[], unsigned char ciphertext[], int msg_len, int *cipher_len);
int decrypt_message(RSA *rsa, char ciphertext[], unsigned char *decrypted, int msg_len);
int get_public(RSA *rsa, char *e, char *n);
int set_public(RSA *rsa, char *e, char *n);

#endif