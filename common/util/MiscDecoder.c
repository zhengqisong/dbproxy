/********************************************************
*
*********************************************************
*		  BASE_64					 		
*		2000.03.03						
*		2000.04.23 (整理注释)
*		2001.06.30 (加test_base64)
*		2001.07.02 (加iso_8859)
*	2003.12.16, ww, 添加Q解码功能，test_base64改名为QpBase64_header_decode)
*	2003.12.21, ww, 添加函数unicode2x(), 用来处理uncode的解码问题
*********************************************************
*		RFC-1521						*
*		MIME (Multipurpose Internet Mail Extensions) Part One: 	*
*		Mechanisms for Specifying and Describing		*
*		the Format of Internet Message Bodies			*
*																		 *
*		 (5.2. Base64 Content-Transfer-Encoding)		*
********************************************************/
//#include "/usr/local/include/iconv.h"
#include <stdio.h>
#include <iconv.h>
#include <error.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "MiscDecoder.h"
#include "g_string.h"
#include "trace.h"

#define T_BASE64 0
#define T_QUOTED_PRINTABLE 1 
#define T_NULL 2 

char base64_code[128];
char base64_code1[32] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char base64_code2[32] = "abcdefghijklmnopqrstuvwxyz";
char base64_code3[32] = "0123456789+/ =";

//lyq
unsigned int m_LineWidth = 76;

int atohex(char c1,char c2);

/************************************************
*		BASE_64  初始化程序		*
*************************************************/
int base64_init()
{
	 int i;

	 for (i=0;i<26;i++) {
		  base64_code[i]	 = base64_code1[i];
		  base64_code[i+26] = base64_code2[i];
		  base64_code[i+52] = base64_code3[i];
	 }
	 return 1;
}

void base64_clean(void)
{
}


/************************************************
*		BASE_64 译码程序		*
*************************************************/
int base64_encode(unsigned char *aBuf1,char *aBuf2)
{
	 int i,i1,i2,code,len;

	 len = strlen(aBuf1);

	 i1 = 0;
	 i2 = 0;
	 for (i=0;i<(len/3);i++)
	 {
		  code = aBuf1[i1] >> 2;
		  aBuf2[i2++] = base64_code[code & 0x3f];

		  code = (aBuf1[i1] << 4) + (aBuf1[i1+1] >> 4);
		  aBuf2[i2++] = base64_code[code & 0x3f];

		  code = (aBuf1[i1+1] << 2) + (aBuf1[i1+2] >> 6);
		  aBuf2[i2++] = base64_code[code & 0x3f];

		  code = aBuf1[i1+2];
		  aBuf2[i2++] = base64_code[code & 0x3f];

		  i1 += 3;
	 }

	 if  ((len%3) == 1)
	 {
		  code = aBuf1[i1] >> 2;
		  aBuf2[i2++] = base64_code[code & 0x3f];

		  code = aBuf1[i1] << 4;
		  aBuf2[i2++] = base64_code[code & 0x3f];

		  aBuf2[i2++] = '=';
		  aBuf2[i2++] = '=';
	 }

	 if  ((len%3) == 2)
	 {
		  code = aBuf1[i1] >> 2;
		  aBuf2[i2++] = base64_code[code & 0x3f];

		  code = (aBuf1[i1] << 4) + (aBuf1[i1+1] >> 4);
		  aBuf2[i2++] = base64_code[code & 0x3f];

		  code = aBuf1[i1+1] << 2;
		  aBuf2[i2++] = base64_code[code & 0x3f];

		  aBuf2[i2++] = '=';
	 }

	 aBuf2[i2] = 0;

	 return 1;
}

//lyq
unsigned int BASE64_DECODE_TABLE[256] = {
	255, 255, 255, 255, 255, 255, 255, 255, //  00 -  07
	255, 255, 255, 255, 255, 255, 255, 255, //  08 -  15
	255, 255, 255, 255, 255, 255, 255, 255, //  16 -  23
	255, 255, 255, 255, 255, 255, 255, 255, //  24 -  31
	255, 255, 255, 255, 255, 255, 255, 255, //  32 -  39
	255, 255, 255,  62, 255, 255, 255,  63, //  40 -  47
	 52,  53,  54,  55,  56,  57,  58,  59, //  48 -  55
	 60,  61, 255, 255, 255, 255, 255, 255, //  56 -  63
	255,	0,	1,	2,	3,	4,	5,	6, //  64 -  71
	  7,	8,	9,  10,  11,  12,  13,  14, //  72 -  79
	 15,  16,  17,  18,  19,  20,  21,  22, //  80 -  87
	 23,  24,  25, 255, 255, 255, 255, 255, //  88 -  95
	255,  26,  27,  28,  29,  30,  31,  32, //  96 - 103
	 33,  34,  35,  36,  37,  38,  39,  40, // 104 - 111
	 41,  42,  43,  44,  45,  46,  47,  48, // 112 - 119
	 49,  50,  51, 255, 255, 255, 255, 255, // 120 - 127
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255 
};

//lyq
int Base64EncodeSize(int iSize)
{
	int nSize, nCR;

	nSize = (iSize + 2) / 3 * 4 ;
	nCR = nSize / m_LineWidth; //计算回车数量
	nSize+= nCR * 2;
	return nSize;
}

//lyq
int base64_decode(char *pDest, unsigned int outlen, char *pSrc, unsigned int nSize)
{
	unsigned int lByteBuffer, lByteBufferSpace;
	unsigned int C; 
	int reallen;
	char *InPtr, *InLimitPtr, *OutLimitPtr;
	char *OutPtr;

	 if ((pSrc == NULL) || (pDest == NULL) || (nSize <= 0))
		  return 0;

	lByteBuffer = 0;
	lByteBufferSpace = 4;
				
	InPtr = pSrc;
	InLimitPtr= InPtr + nSize;
	OutLimitPtr = InPtr + outlen;
	OutPtr = pDest;
				
	while ((InPtr != InLimitPtr) && (InPtr != OutLimitPtr))
	{
		C = BASE64_DECODE_TABLE[(int)*InPtr]; 
		InPtr++;
		if (C == 0xFF)
			continue; 			
		lByteBuffer = lByteBuffer << 6 ;
		lByteBuffer = lByteBuffer | C ;
		lByteBufferSpace--;
		if (lByteBufferSpace != 0)
			continue; 
		OutPtr[2] = lByteBuffer;
		lByteBuffer = lByteBuffer >> 8;
		OutPtr[1] = lByteBuffer;
		lByteBuffer = lByteBuffer >> 8;
		OutPtr[0] = lByteBuffer;
		
		OutPtr += 3; lByteBuffer = 0; lByteBufferSpace = 4;
	}
	reallen = (unsigned int)OutPtr - (unsigned int)pDest;
	
	switch (lByteBufferSpace)
	{
		case 1:
			lByteBuffer = lByteBuffer >> 2;
			OutPtr[1] = lByteBuffer;
			lByteBuffer = lByteBuffer >> 8;
			OutPtr[0] = lByteBuffer;
			OutPtr[2] = 0;
			return reallen + 2;
		case 2:
			lByteBuffer = lByteBuffer >> 4;
			OutPtr[0] = lByteBuffer;
			OutPtr[1] = 0;
			return reallen + 1;
		default:
			OutPtr[0] = 0;
			return reallen;
	}
}


/* 
 * Description:
 * 	Decode string which is encoded as Qprintable or base64.
 * 	The format goes like this: =?charset?encoding?encoded_txt?=
 * 
 * Return val: 	
 *		0	: failed
 *		otherwise: the dest string len which was decoded.
 */ 
int QpBase64_header_decode(char *dst, int max_dstlen, char *src, int srclen)
{
	char *encoded_txt_head = NULL;
	int i, ques_num, encoded_txt_len = 0;
	int encoding_t = T_NULL, final_stringlen = 0;
	char *charset_txt_head = NULL;
	int  charset_txt_len = 0;
	ques_num = 0;
	char buf[10000];
	int buf_len = 0;
	buf[0] = '\0';

	
	for(i = 0; i < srclen; i++) {
		if(src[i] == 0)
			break;
		if(src[i] == '?') {
			ques_num++;
	 		/* WW:	needn't care the 'charset' which is embraced 
			 * 		by the 1st and the 2nd '?' */
			if ((ques_num == 1) && (i < srclen - 1)) {
				charset_txt_head = src + i + 1;
			} else if ((ques_num == 2) && (i < srclen - 1)) {
				if (charset_txt_len == 0) {
					charset_txt_len = src + i - charset_txt_head;
				}

				if(src[i+1] == 'B' || src[i+1] == 'b')
					encoding_t = T_BASE64;
				else if(src[i+1] == 'Q' || src[i+1] == 'q')
					encoding_t = T_QUOTED_PRINTABLE;
				else
					encoding_t = T_NULL;
			} else if ((ques_num == 3) && (i < srclen - 1)) {
				encoded_txt_head = src + i + 1;
			} else if (ques_num == 4) {
				encoded_txt_len = src + i - encoded_txt_head;
				break;
		 	}
		}
 	}
	
	if (ques_num == 4) {
		switch(encoding_t) {
  		case T_BASE64:
			final_stringlen = 
				base64_decode(dst, max_dstlen - 1, encoded_txt_head, encoded_txt_len);
			break;
		case T_QUOTED_PRINTABLE:
			final_stringlen = 
				Qprintable_decode(dst, max_dstlen - 1, encoded_txt_head, encoded_txt_len);
			break;
		default://T_NULL
			final_stringlen = 0;
			break;
		}
	} else if (!ques_num) {//ASCII
	 	final_stringlen = v_strncpy(dst, max_dstlen, src, srclen);
	} else	//coding method that cannot be recognized
		final_stringlen = 0;

	if (charset_txt_len > 0 && final_stringlen > 0) {
		if (v_strncasecmp(charset_txt_head, charset_txt_len, "utf-8", strlen("utf-8")) == 0) {
			buf_len = v_strncpy(buf, sizeof(buf), dst, final_stringlen);
			final_stringlen = convert_utf8_to_gb2312(dst, max_dstlen, buf, buf_len);
			//printf("dst= %s\n", dst);
		}
	}
	
	dst[final_stringlen] = 0;
	return final_stringlen;
}


#if 0
int iso_8859(char *aBuf1,char *aBuf2)
/****************************************
*		 ISO-8859格式转换		*
*		 输入:	aBuf1:						*
*		 输出:	aBuf2:						*
*		 说明:	XX=XX=XX=..		*
*****************************************/
{
	 int i, j;

	 i = 0;
	 j = 0;
	 while (1)
	 {
	if  (aBuf1[i] == 0)
		 break;

	if  (aBuf1[i] == '=')
	{
		 aBuf2[j] = atohex(aBuf1[i+1],aBuf1[i+2]);
		 i += 3;
		 j++;
	}
	else
		aBuf2[j++] = aBuf1[i++];
	 }
	 aBuf2[j] = 0;
	 return j;
}

int atohex(char c1,char c2)
{
	 int n;

	 if  ( (c1 >= '0') && (c1 <= '9') )
	n = c1 - 0x30;
	 else
	 {
		c1 |= 0x20;
		if ( (c1 >= 'a') && (c1 <= 'z') )
		 	n = c1 - 'a' + 10;
		else
		 	n = 0;
	 }

	 n = n << 4;

	 if  ( (c2 >= '0') && (c2 <= '9') )
		  n += c2 - 0x30;
	 else
	 {
		  c2 |= 0x20;
		  if  ( (c2 >= 'a') && (c2 <= 'z') )
				n += c2 - 'a' + 10;
		  else
				n += 0;
	 }

	 return n;
}
#endif

/* This function is from Snort by CSC */
/*
 * Function: base64(char * xdata, int length)
 *
 * Purpose: Insert data into the database
 *
 * Arguments: xdata  => pointer to data to base64 encode
 *				length => how much data to encode 
 *
 * Make sure you allocate memory for the output before you pass
 * the output pointer into this function. You should allocate 
 * (1.5 * length) bytes to be safe.
 *
 * Returns: data base64 encoded as a char *
 *
 */
char * base64(u_char * xdata, int length)
{
	 int count, cols, bits, c, char_count;
	 unsigned char alpha[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	 char * payloadptr;
	 char * output;
	 char_count = 0;
	 bits = 0;
	 cols = 0;

	 output = (char *)malloc( (unsigned int) (length * 1.5 + 4) );

	 payloadptr = output;

	 for(count = 0; count < length; count++)
	 {
		  c = xdata[count];

		  if(c > 255)
		  {
				printf("plugbase.c->base64(): encountered char > 255 (decimal %d)\n If you see this error message a char is more than one byte on your machine\n This means your base64 results can not be trusted", c);
		  }

		  bits += c;
		  char_count++;

		  if(char_count == 3)
		  {
				*output = alpha[bits >> 18]; output++;
				*output = alpha[(bits >> 12) & 0x3f]; output++;
				*output = alpha[(bits >> 6) & 0x3f]; output++;
				*output = alpha[bits & 0x3f]; output++; 
				cols += 4;
				if(cols == 72)
				{
					 *output = '\n'; output++;
					 cols = 0;
				}
				bits = 0;
				char_count = 0;
		  }
		  else
		  {
				bits <<= 8;
		  }
	 }

	 if(char_count != 0)
	 {
		  bits <<= 16 - (8 * char_count);
		  *output = alpha[bits >> 18]; output++;
		  *output = alpha[(bits >> 12) & 0x3f]; output++;
		  if(char_count == 1)
		  {
				*output = '='; output++;
				*output = '='; output++;
		  }
		  else
		  {
				*output = alpha[(bits >> 6) & 0x3f]; 
				output++; *output = '='; 
				output++;
		  }
	 }
	 *output = '\0';
	 return payloadptr;
} 




/*
 * <Argument>
 * 	buf: original string.
 * 	len: original string length.
 * <Return Val> 
 * 	the length of the final string.
 * <Comment>
 * 	the final string is also filled at buf, that is buf is changed.
 */ 
int Qprintable_decode(char *dst, int max_dstlen, char *src, int srclen)
{
	char tmp = 0;
	char *src_tmp = src, *dst_tmp = dst;
	int p_counter = 0, i =0;

	/*add by rechard 2006-02-23 to avoid buffer overlap*/
	if ((!dst) || (!src) || (max_dstlen < 1) || (srclen < 1)) {
		goto rtn;                                                   
	}  
	/*add end*/
	while(p_counter < srclen) {
		if(*src_tmp == '=') {
			for (i=1; i >= 0; i --) {	
				src_tmp ++;
	//			printf("*src_p = %c\n", *src_p);
				if(*src_tmp == 0x0d && *(src_tmp + 1) == 0x0a) {
					/* in case of: "=<CRLF>E2", bypass the <CRLF> */
					src_tmp += 2;
					continue;
				} 
				else if(*src_tmp >= 0x61  && *src_tmp <= 0x66)
					tmp |= (*src_tmp - 0x57) & 0xff;
				else if(*src_tmp >= 0x41 && *src_tmp <= 0x46)
					tmp |= (*src_tmp - 0x37) & 0xff;
				else if(*src_tmp >= 0x30 && *src_tmp <= 0x39)
					tmp = tmp | ((*src_tmp - 0x30) & 0xff);
				tmp = tmp << 4 * i ;
		//		printf("tmp = %x\n", tmp);
			}			
			*dst_tmp = tmp;
			tmp = 0;
			p_counter += 3;//for example: =E2
		} else {
			*dst_tmp = *src_tmp;
			p_counter ++;
		}
		dst_tmp ++;
		if(dst_tmp - dst >= max_dstlen - 1)
			goto rtn;
		src_tmp ++;
	}//end while
rtn:
	*dst_tmp = 0; //terminate the str.
	return dst_tmp - dst;
}

int UCS2_to_UTF8(unsigned short *ucs2, int ucs2_len, char *utf8, int utf8_len)
{
    unsigned short unicode;
    unsigned char bytes[4] = {0};
    int nbytes = 0;
    int i = 0, j = 0;
    int len=0;

	if ((ucs2 != NULL) && (utf8 != NULL)) {
		if (ucs2_len == 0) {
			len = 0;
		} else {
			for (i = 0; i < ucs2_len/2; i++) {
				unicode = ucs2[i];

				if (unicode < 0x80) {
					nbytes = 1;
					bytes[0] = unicode;
				} else if (unicode < 0x800) {
					nbytes = 2;
					bytes[1] = ((unicode & 0x3f) | 0x80);
					bytes[0] = (((unicode << 2) & 0x1f00) | 0xc000) >> 8;
				} else {
					nbytes = 3;
					bytes[2] = ((unicode & 0x3f) | 0x80);
					bytes[1] = (((unicode << 2) & 0x3f00) | 0x8000) >> 8;
					bytes[0] = (((unicode << 4) & 0x0f0000) | 0xe00000) >> 16;
				}
				if(len + nbytes >= utf8_len){
					goto error;
				}
				for (j = 0; j < nbytes; j++) {
					utf8[len] = bytes[j];
					len++;
				}
			}
		}
error:
		utf8[len] = '\0';
	}
	return len;
}

iconv_t unicode_to_utf8_converter = (iconv_t)-1;
iconv_t unicode_swapped_to_utf8_converter = (iconv_t)-1;
iconv_t gb2312_to_utf8_converter = (iconv_t)-1;
iconv_t utf16_to_utf8_converter = (iconv_t)-1;
iconv_t utf8_to_utf16_converter = (iconv_t)-1;
iconv_t utf8_to_gb2312_converter = (iconv_t)-1;
iconv_t utf8_to_unicode_converter = (iconv_t)-1;
iconv_t ucs2le_to_utf8_converter = (iconv_t)-1;

int32_t init_encode_converter()
{
	ucs2le_to_utf8_converter = iconv_open("utf-8", "ucs-2le");
	unicode_to_utf8_converter = iconv_open("utf-8", "UCS-2-INTERNAl");
	unicode_swapped_to_utf8_converter = iconv_open("utf-8", "UCS-2-SWAPPED");
	gb2312_to_utf8_converter = iconv_open("utf-8", "gb18030");
	utf16_to_utf8_converter = iconv_open("utf-8", "utf-16");
	utf8_to_utf16_converter = iconv_open("utf-16", "utf-8");
	utf8_to_gb2312_converter = iconv_open("gb18030", "utf-8");
	utf8_to_unicode_converter = iconv_open("UCS-2-INTERNAl", "utf-8");

	return 0;
}

static int32_t simple_base_convert(iconv_t cd, char *in_out_buf, int32_t in_len, int32_t total_out_len)
{
	static char inner_outbuf[10000];

	int32_t ret = 0;
	char *outbuf = inner_outbuf;
	int32_t left_out_len = 0; 
	char *initial_in_out_buf = in_out_buf;

	if (NULL == in_out_buf || total_out_len < 0 || in_len < 0) {
		goto err;
	}

	if ((iconv_t)-1 == cd) {
		goto err;
	}

	total_out_len = total_out_len > 10000 ? 10000 : total_out_len;
	left_out_len = total_out_len;

	ret = iconv(cd, &in_out_buf, &in_len, &outbuf, &left_out_len);
	if (-1 == ret) {
//		if (E2BIG != errno) {
//			goto err;
//		}
	}

	if (left_out_len < 0 || left_out_len > total_out_len) {
		goto err;
	}

	return v_strncpy(initial_in_out_buf, total_out_len, inner_outbuf, total_out_len - left_out_len);

err:
	if (in_out_buf) {
		in_out_buf[0] = 0;
	}
	return 0;
}

int32_t simple_convert_gb2312_to_utf8(char *in_out_buf, int32_t in_len, int32_t total_out_len)
{
	return simple_base_convert(gb2312_to_utf8_converter, in_out_buf, in_len, total_out_len);
}

int32_t simple_convert_unicode_to_utf8(char *in_out_buf, int32_t in_len, int32_t total_out_len)
{
	return simple_base_convert(unicode_to_utf8_converter, in_out_buf, in_len, total_out_len);
}

int32_t simple_convert_utf16_to_utf8(char *in_out_buf, int32_t in_len, int32_t total_out_len)
{
	return simple_base_convert(utf16_to_utf8_converter, in_out_buf, in_len, total_out_len);
}

static int32_t base_convert(iconv_t cd, char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{
	int32_t ret = 0;
	int32_t left_out_len = total_out_len;
	char *initial_outbuf = outbuf;
	int32_t out_len = 0;

	if (NULL == inbuf || NULL == outbuf || total_out_len <= 0 || in_len < 0) {
		goto err;
	}

	if ((iconv_t)-1 == cd) {
		goto err;
	}

	ret = iconv(cd, &inbuf, &in_len, &outbuf, &left_out_len);
	if (-1 == ret) {
//		if (E2BIG != errno) {
//			goto err;
//		}
	}

	if (left_out_len < 0 || left_out_len > total_out_len) {
		goto err;
	}

	if (left_out_len != 0) {
		out_len = total_out_len - left_out_len;
	} else {
		out_len = total_out_len - 1;
	} 

	initial_outbuf[out_len] = '\0';
	return out_len;

err:
	if (outbuf) {
		outbuf[0] = '\0';
	}
	return 0;
}

int32_t convert_ucs2le_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{

	//return UCS2_to_UTF8((unsigned short*)inbuf, in_len, outbuf, total_out_len);//
	return base_convert(ucs2le_to_utf8_converter, outbuf, total_out_len, inbuf, in_len);
}

int32_t convert_unicode_swapped_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{
	return base_convert(unicode_swapped_to_utf8_converter, outbuf, total_out_len, inbuf, in_len);
}

int32_t convert_unicode_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{
	return base_convert(unicode_to_utf8_converter, outbuf, total_out_len, inbuf, in_len);
}

int32_t convert_utf16_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{
	return base_convert(utf16_to_utf8_converter, outbuf, total_out_len, inbuf, in_len);
}

int32_t convert_gb2312_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{
	return base_convert(gb2312_to_utf8_converter, outbuf, total_out_len, inbuf, in_len);
}

int32_t convert_utf8_to_unicode(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{
	return base_convert(utf8_to_unicode_converter, outbuf, total_out_len, inbuf, in_len);
}

int32_t convert_utf8_to_gb2312(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{
	return base_convert(utf8_to_gb2312_converter, outbuf, total_out_len, inbuf, in_len);
}

int32_t convert_utf8_to_utf16(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{
	return base_convert(utf8_to_utf16_converter, outbuf, total_out_len, inbuf, in_len);
}

int32_t onestep_convert_gb2312_to_utf8(char *outbuf, int32_t total_out_len, char *inbuf, int32_t in_len)
{
	iconv_t converter = (iconv_t)-1;
	int32_t ret = 0;
	int32_t left_out_len = total_out_len;
	char *initial_outbuf = outbuf;
	int32_t out_len = 0;

	if (NULL == inbuf || NULL == outbuf || total_out_len <= 0 || in_len < 0) {
		goto err;
	}

	converter = iconv_open("utf-8", "gb18030");
	if ((iconv_t)-1 == converter) {
		goto err;
	}

	ret = iconv(converter, &inbuf, &in_len, &outbuf, &left_out_len);
	if (-1 == ret) {
//		if (E2BIG != errno) {
//			goto err;
//		}
	}

	if (left_out_len < 0 || left_out_len > total_out_len) {
		goto err;
	}

	if (left_out_len != 0) {
		out_len = total_out_len - left_out_len;
	} else {
		out_len = total_out_len - 1;
	} 

	initial_outbuf[out_len] = '\0';
	iconv_close(converter);

	return out_len;

err:
	if (outbuf) {
		outbuf[0] = '\0';
	}
	return 0;
}

int32_t convert_utf16_to_utf8_or_copy(char *outbuf, int32_t total_out_len, 
			char *inbuf, int32_t in_len)
{
	int32_t ret = 0;

	ret = convert_utf16_to_utf8(outbuf, total_out_len, inbuf, in_len);
	if (0 == ret) {
		ret = v_strncpy(outbuf, total_out_len, inbuf, in_len);
	}

	return ret;
}

int32_t convert_unicode_to_utf8_or_copy(char *outbuf, int32_t total_out_len, 
			char *inbuf, int32_t in_len)
{
	int32_t ret = 0;

	ret = convert_unicode_to_utf8(outbuf, total_out_len, inbuf, in_len);
	if (0 == ret) {
		ret = v_strncpy(outbuf, total_out_len, inbuf, in_len);
	}

	return ret;
}

int32_t convert_gb2312_to_utf8_or_copy(char *outbuf, int32_t total_out_len, 
			char *inbuf, int32_t in_len)
{
	int32_t ret = 0;

	ret = convert_gb2312_to_utf8(outbuf, total_out_len, inbuf, in_len);
	if (0 == ret) {
		ret = v_strncpy(outbuf, total_out_len, inbuf, in_len);
	}

	return ret;
}

void clean_encode_converter()
{
	if ((iconv_t)-1 != unicode_to_utf8_converter) {
		iconv_close(unicode_to_utf8_converter);
		unicode_to_utf8_converter = (iconv_t)-1;
	}

	if ((iconv_t)-1 != unicode_swapped_to_utf8_converter) {
		iconv_close(unicode_swapped_to_utf8_converter);
		unicode_swapped_to_utf8_converter = (iconv_t)-1;
	}

	if ((iconv_t)-1 != gb2312_to_utf8_converter) {
		iconv_close(gb2312_to_utf8_converter);
		gb2312_to_utf8_converter = (iconv_t)-1;
	}

	if ((iconv_t)-1 != utf16_to_utf8_converter) {
		iconv_close(utf16_to_utf8_converter);
		utf16_to_utf8_converter = (iconv_t)-1;
	}

	if ((iconv_t)-1 != utf8_to_utf16_converter) {
		iconv_close(utf8_to_utf16_converter);
		utf8_to_utf16_converter = (iconv_t)-1;
	}

	if ((iconv_t)-1 != utf8_to_gb2312_converter) {
		iconv_close(utf8_to_gb2312_converter);
		utf8_to_gb2312_converter = (iconv_t)-1;
	}

	if ((iconv_t)-1 != utf8_to_unicode_converter) {
		iconv_close(utf8_to_unicode_converter);
		utf8_to_unicode_converter = (iconv_t)-1;
	}

	if ((iconv_t)-1 != ucs2le_to_utf8_converter) {
		iconv_close(ucs2le_to_utf8_converter);
		ucs2le_to_utf8_converter = (iconv_t)-1;
	}
}


