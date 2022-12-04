#include "encrypt.h"


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

// takes an RSA struct pointer where the keys will be stored
int get_keys(RSA *keypair) {
    /*
    if(file_exists("keys")) {
        FILE *keys = fopen("keys", "r");
        fread(*keypair, RSA_size(*keypair), 1, keys);
        return 0;
    } else {
    
    printf("Generating RSA (%d bits) keypair...", KEY_LENGTH);
    fflush(stdout);*/

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

    if ((keypair = RSA_new()) == NULL) {
        log_ssl_err("RSA_new failed");
        BN_free(e);
        return 1;
    }
    
    if(!RSA_generate_key_ex(keypair, KEY_LENGTH, e, NULL)) {
        log_ssl_err("couldn't generate rsa key");
        BN_free(e);
        return 1;
    }

    /* FILE *keys = fopen("keys", "w");
    fwrite(keypair, RSA_size(keypair), 1, keys); */
    return 0;
}

//msg is the pointer to the original message, ciphertext is where the encrypted message will be stored
int encrypt_message(RSA *rsa, char msg[],unsigned char *ciphertext, int msg_len) {
    unsigned char   *encrypt = NULL;    // Encrypted message
    unsigned char   *decrypt = NULL;    // Decrypted message
    char   *err;               // Buffer for any error messages


    // Encrypt the message
    unsigned char *plaintext = (unsigned char*) msg;
    unsigned char *decrypted;
    int cipher_len, decrypted_len;

    ciphertext = malloc(RSA_size(rsa));
    if ((cipher_len = RSA_public_encrypt(strlen(plaintext), plaintext, ciphertext, 
                                        rsa, RSA_PKCS1_OAEP_PADDING)) == -1) {
        log_ssl_err("RSA_public_encrypt failed");
        exit(1);
    }
    /* 
    #ifdef WRITE_TO_FILE
    // Write the encrypted message to a file
        FILE *out = fopen("out.bin", "w");
        fwrite(encrypt, sizeof(*encrypt),  RSA_size(keypair), out);
        fclose(out);
        printf("Encrypted message written to file.\n");
        free(encrypt);
        encrypt = NULL;

        // Read it back
        printf("Reading back encrypted message and attempting decryption...\n");
        encrypt = malloc(RSA_size(keypair));
        out = fopen("out.bin", "r");
        fread(encrypt, sizeof(*encrypt), RSA_size(keypair), out);
        fclose(out);
    #endif

    // Decrypt it
    decrypt = malloc(encrypt_len);
    if(RSA_private_decrypt(encrypt_len, (unsigned char*)encrypt, (unsigned char*)decrypt,
                           keypair, RSA_PKCS1_OAEP_PADDING) == -1) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error decrypting message: %s\n", err);
        goto free_stuff;
    }
    printf("Decrypted message: %s\n", decrypt);

    free_stuff:
    RSA_free(keypair);
    BIO_free_all(pub);
    BIO_free_all(pri);
    free(pri_key);
    free(pub_key);
    free(encrypt);
    free(decrypt);
    free(err);
 */
    return 0;
}

int decrypt_message(RSA *rsa, char ciphertext[], unsigned char *decrypted, int msg_len) {
    char   *err;               // Buffer for any error messages

    unsigned char *encrypted = (unsigned char *) ciphertext;
    int decrypted_len;

    decrypted = malloc(RSA_size(rsa));
    if ((decrypted_len = RSA_private_decrypt(msg_len, encrypted, decrypted, 
                                            rsa, RSA_PKCS1_OAEP_PADDING)) == -1) {
        log_ssl_err("RSA_private_decrypt failed");
        return 0;
    }
}
