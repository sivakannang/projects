#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include "rsa.h"

EVP_PKEY *rsa_generate_key(int bits)
{
	return EVP_RSA_gen(bits);
}

// Writes a NUL-terminated PEM. Returns its length, or -1 if it doesn't fit.
int rsa_public_key_pem(EVP_PKEY *key, char *pem, int size)
{
	char *data = NULL;
	long len;
	BIO *bio = BIO_new(BIO_s_mem());

	if ( bio == NULL || PEM_write_bio_PUBKEY(bio, key) != 1 )
	{
		BIO_free(bio);
		return -1;
	}

	len = BIO_get_mem_data(bio, &data);
	if ( len >= size )
	{
		BIO_free(bio);
		return -1;
	}

	memcpy(pem, data, len);
	pem[len] = '\0';

	BIO_free(bio);

	return (int)len;
}

// OAEP, because the older PKCS#1 v1.5 padding allows padding-oracle attacks on encryption
static int use_oaep(EVP_PKEY_CTX *ctx)
{
	return EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) > 0
		&& EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) > 0
		&& EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) > 0;
}

int rsa_public_encrypt(const unsigned char *data, int len, const char *pem, int pemLen, unsigned char *out, int outSize)
{
	int result = -1;
	size_t outLen = outSize;
	EVP_PKEY *key = NULL;
	EVP_PKEY_CTX *ctx = NULL;

	// The explicit length matters: the PEM came from the network, so it may have no NUL
	BIO *bio = BIO_new_mem_buf(pem, pemLen);
	if ( bio == NULL )
		return -1;

	key = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
	BIO_free(bio);

	if ( key == NULL )
		return -1;

	if ( !EVP_PKEY_is_a(key, "RSA") || EVP_PKEY_get_bits(key) < MIN_RSA_BITS )
	{
		fprintf(stderr, "Rejected key: must be RSA of at least %d bits\n", MIN_RSA_BITS);
		EVP_PKEY_free(key);
		return -1;
	}

	ctx = EVP_PKEY_CTX_new(key, NULL);

	if ( ctx != NULL && EVP_PKEY_encrypt_init(ctx) > 0 && use_oaep(ctx) && EVP_PKEY_encrypt(ctx, out, &outLen, data, len) > 0 )
		result = (int)outLen;

	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(key);

	return result;
}

int rsa_private_decrypt(const unsigned char *data, int len, EVP_PKEY *key, unsigned char *out, int outSize)
{
	int result = -1;
	size_t outLen = outSize;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key, NULL);

	if ( ctx != NULL && EVP_PKEY_decrypt_init(ctx) > 0 && use_oaep(ctx) && EVP_PKEY_decrypt(ctx, out, &outLen, data, len) > 0 )
		result = (int)outLen;

	EVP_PKEY_CTX_free(ctx);

	return result;
}

void printLastError(const char *msg)
{
	char err[256];

	ERR_error_string_n(ERR_get_error(), err, sizeof(err));
	fprintf(stderr, "%s ERROR: %s\n", msg, err);
}


// TCP has no message boundaries, so one recv may return part of a message
static int read_full(int fd, unsigned char *buf, int len)
{
	int total = 0;

	while ( total < len )
	{
		ssize_t n = recv(fd, buf + total, len - total, 0);

		if ( n > 0 )
			total += n;
		else if ( n == 0 || errno != EINTR )
			return -1;
	}

	return total;
}

static int write_full(int fd, const unsigned char *buf, int len)
{
	int total = 0;

	while ( total < len )
	{
		ssize_t n = send(fd, buf + total, len - total, 0);

		if ( n > 0 )
			total += n;
		else if ( n == 0 || errno != EINTR )
			return -1;
	}

	return total;
}

int read_frame(int fd, unsigned char *payload, int size)
{
	unsigned char header[2];
	int len;

	if ( read_full(fd, header, sizeof(header)) == -1 )
		return -1;

	len = (header[0] << 8) | header[1];
	if ( len > size )
		return -1;

	return read_full(fd, payload, len);
}

int write_frame(int fd, const unsigned char *payload, int len)
{
	unsigned char header[2];

	if ( len > 0xFFFF )
		return -1;

	header[0] = (unsigned char)(len >> 8);
	header[1] = (unsigned char)(len & 0xFF);

	if ( write_full(fd, header, sizeof(header)) == -1 )
		return -1;

	return write_full(fd, payload, len);
}
