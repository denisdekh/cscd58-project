#include "encrypt.h"
#ifndef NET_UTILS_H
    #include "netutils.h"
#endif 


bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat(filename, &buffer) == 0);
}

void log_ssl_err(const char *mes)
{
    unsigned long err, found;
    char errstr[1000];

    found = 0;
    while ((err = ERR_get_error())) {
        ERR_error_string(err, errstr);
        printf("%s: %s", mes, errstr);
        found = 1;
    }
    if (!found) {
        printf("%s", mes);
    }
}

// get the public key values to be sent to another user
int get_public(RSA *rsa, char *e, char *n) {
    strncpy(e, BN_bn2hex(RSA_get0_e(rsa)), MAX_REQUEST);
    strncpy(n, BN_bn2hex(RSA_get0_n(rsa)), MAX_REQUEST);
    printf("Copied keys: e = %s and n = %s\n", e, n);
    return 0;
}

// set the public key from another user
int set_public(RSA *rsa, char *e, char *n) {
    BIGNUM *ebn = BN_new();
    BIGNUM *nbn = BN_new();
    BN_hex2bn(&ebn, e);
    BN_hex2bn(&nbn, n);
    RSA_set0_key(rsa, n, e, NULL);
    return 0;
}

// takes an RSA struct pointer where the keys will be stored
int get_keys(RSA **keypair) {
    fprintf(stderr, "Generating RSA (%d bits) keypair...", KEY_LENGTH);
    // Generate key pair
    BIGNUM *e;
    uint32_t exponent_bin, exponent_num;
    exponent_num = PUB_EXP;
    exponent_bin = htonl(exponent_num);
    e = BN_bin2bn((const unsigned char *)&exponent_bin, 4, NULL);
    if (e == NULL) {
        log_ssl_err("BN_bin2bn failed for e");
        return 1;
    }

    if ((*keypair = RSA_new()) == NULL) {
        log_ssl_err("RSA_new failed");
        BN_free(e);
        return 1;
    }
    
    if(!RSA_generate_key_ex(*keypair, KEY_LENGTH, e, NULL)) {
        log_ssl_err("couldn't generate rsa key");
        BN_free(e);
        return 1;
    }
    /* FILE *keys = fopen("keys", "w");
    fwrite(keypair, RSA_size(keypair), 1, keys); */
    return 0;
}

//msg is the pointer to the original message, ciphertext is where the encrypted message will be stored
int encrypt_message(RSA *rsa, char msg[], unsigned char *ciphertext, int msg_len, int *cipher_len) {
    // Encrypt the message
    unsigned char *plaintext = (unsigned char*) msg;

    if ((*cipher_len = RSA_public_encrypt(msg_len, plaintext, ciphertext, 
                                        rsa, RSA_PKCS1_OAEP_PADDING)) == -1) {
        log_ssl_err("RSA_public_encrypt failed");
        exit(1);
    }
    
    return 0;
}

int decrypt_message(RSA *rsa, char ciphertext[], unsigned char *decrypted, int msg_len) {

    unsigned char *encrypted = (unsigned char *) ciphertext;
    int decrypted_len;

    if ((decrypted_len = RSA_private_decrypt(msg_len, encrypted, decrypted, 
                                            rsa, RSA_PKCS1_OAEP_PADDING)) == -1) {
        log_ssl_err("RSA_private_decrypt failed");
        return -1;
    }
    return decrypted_len;
}
