#ifndef _MISCDECODER_H
#define _MISCDECODER_H

#include <iconv.h>
#include <sys/types.h>

int base64_init();
void base64_clean(void);
int base64_encode(unsigned char *aBuf1,char *aBuf2);
int Base64EncodeSize(int iSize);
int base64_decode(char *pDest, unsigned int outlen, char *pSrc, unsigned int nSize);
int QpBase64_header_decode(char *dst, int max_dstlen, char *src, int srclen);
int Qprintable_decode(char *dst, int max_dstlen, char *src, int srclen);

int unicode2x(char *src, int *p_srclen,  char *dest_charset);

int32_t init_encode_converter();

int32_t convert_gb2312_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len);
int32_t convert_utf16_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len);
int32_t convert_unicode_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len);
int32_t convert_unicode_swapped_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len);
int32_t convert_utf8_to_unicode(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len);
int32_t convert_utf8_to_utf16(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len);
int32_t convert_utf8_to_gb2312(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len);
int32_t convert_ucs2le_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len);

int32_t simple_convert_unicode_to_utf8(char *in_out_buf, int32_t in_len, 
			int32_t total_out_len);
int32_t simple_convert_gb2312_to_utf8(char *in_out_buf, int32_t in_len, 
			int32_t total_out_len);
int32_t simple_convert_utf16_to_utf8(char *in_out_buf, int32_t in_len, 
			int32_t total_out_len);

int32_t convert_utf16_to_utf8_or_copy(char *outbuf, int32_t total_out_len, 
			char *inbuf, int32_t in_len);
int32_t convert_unicode_to_utf8_or_copy(char *outbuf, int32_t total_out_len, 
			char *inbuf, int32_t in_len);
int32_t convert_gb2312_to_utf8_or_copy(char *outbuf, int32_t total_out_len, 
			char *inbuf, int32_t in_len);
int32_t onestep_convert_gb2312_to_utf8(char *outbuf, int32_t total_out_len, 
			char *inbuf, int32_t in_len);
void clean_encode_converter();
#endif
