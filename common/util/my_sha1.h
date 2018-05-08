#ifndef MY_SHA1_H_
#define MY_SHA1_H_

enum sha_result_codes
{
  SHA_SUCCESS = 0,
  SHA_NULL,		/* Null pointer parameter */
  SHA_INPUT_TOO_LONG,	/* input data too long */
  SHA_STATE_ERROR	/* called Input after Result */
};

#define SHA1_HASH_SIZE 20 /* Hash size in bytes */

/*
  This structure will hold context information for the SHA-1
  hashing operation
*/

typedef struct SHA1_CONTEXT
{
  ulonglong  Length;		/* Message length in bits      */
  uint32 Intermediate_Hash[SHA1_HASH_SIZE/4]; /* Message Digest  */
  int Computed;			/* Is the digest computed?	   */
  int Corrupted;		/* Is the message digest corrupted? */
  int16 Message_Block_Index;	/* Index into message block array   */
  uint8 Message_Block[64];	/* 512-bit message blocks      */
} SHA1_CONTEXT;

/*
  Function Prototypes
*/

int mysql_sha1_reset(SHA1_CONTEXT*);
int mysql_sha1_input(SHA1_CONTEXT*, const uint8 *, unsigned int);
int mysql_sha1_result(SHA1_CONTEXT* , uint8 Message_Digest[SHA1_HASH_SIZE]);

void scramble(char *to, const char *message, const char *password);
#endif

