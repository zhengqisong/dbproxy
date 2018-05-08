#ifndef _STREAM_H
#define _STREAM_H

enum {
	DIR_REQUEST,
	DIR_RESPONSE
};

#define MAX_SQL_LEN	1024

typedef struct _stream_t{
	char* p;
	char* end;
	char* data;
	int size;
}stream_t;

#define make_stream(s) \
{ \
  (s) = (struct _stream_t*)malloc(sizeof(struct _stream_t)); \
  if ((s)) \
  { \
    memset((s), 0, sizeof(struct _stream_t)); \
  } \
}

#define init_stream(s, v) \
{ \
  if ((v) > (s)->size) \
  { \
    free((s)->data); \
	memset((s), 0, sizeof(struct _stream_t)); \
    (s)->data = (char*)malloc((v)); \
	(s)->size = ((s)->data ? (v) : 0); \
  } \
  (s)->p = (s)->data; \
  (s)->end = (s)->data; \
}

#define resize_stream(s, v) \
{ \
	char* new_s; \
	int p_len; \
	int end_len; \
	if((v) > (s)->size) \
	{ \
		(v) /= 1024; \
		(v)++; \
		(v) *= 1024; \
		new_s = (char *)malloc(v); \
		memcpy(new_s, (s)->data, (s)->end - (s)->data); \
		\
		p_len = (s)->p - (s)->data; \
		end_len = (s)->end - (s)->data; \
		\
		free((s)->data);\
		\
		(s)->data = new_s; \
		(s)->p = (s)->data + p_len; \
		(s)->end = (s)->data + end_len; \
		(s)->size = ((s)->data ? (v) : 0); \
	} \
}

#define free_stream(s) \
{ \
  if ((s)) \
  { \
    free((s)->data); \
  } \
  free((s)); \
} \

#define stream_in_sint8(s, v) \
{ \
  (v) = *((signed char*)((s)->p)); \
  (s)->p++; \
}

#define stream_in_uint8(s, v) \
{ \
  (v) = *((unsigned char*)((s)->p)); \
  (s)->p++; \
}

#define stream_in_sint16_le(s, v) \
{ \
  (v) = (signed short) \
    ( \
      (*((unsigned char*)((s)->p + 0)) << 0) | \
      (*((unsigned char*)((s)->p + 1)) << 8) \
    ); \
  (s)->p += 2; \
}

#define stream_in_uint16_le(s, v) \
{ \
  (v) = (unsigned short) \
    ( \
      (*((unsigned char*)((s)->p + 0)) << 0) | \
      (*((unsigned char*)((s)->p + 1)) << 8) \
    ); \
  (s)->p += 2; \
}

#define stream_in_uint16_be(s, v) \
{ \
  (v) = *((unsigned char*)((s)->p)); \
  (s)->p++; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)((s)->p)); \
  (s)->p++; \
}

#define stream_in_uint32_le(s, v) \
{ \
  (v) = (unsigned int) \
    ( \
      (*((unsigned char*)((s)->p + 0)) << 0) | \
      (*((unsigned char*)((s)->p + 1)) << 8) | \
      (*((unsigned char*)((s)->p + 2)) << 16) | \
      (*((unsigned char*)((s)->p + 3)) << 24) \
    ); \
  (s)->p += 4; \
}

#define stream_in_uint32_be(s, v) \
{ \
  (v) = *((unsigned char*)((s)->p)); \
  (s)->p++; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)((s)->p)); \
  (s)->p++; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)((s)->p)); \
  (s)->p++; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)((s)->p)); \
  (s)->p++; \
}

#define stream_in_uint24_le(s, v) \
{ \
  (v) = (unsigned int) \
    ( \
      (*((unsigned char*)((s)->p + 0)) << 0) | \
      (*((unsigned char*)((s)->p + 1)) << 8) | \
      (*((unsigned char*)((s)->p + 2)) << 16)  \
    ); \
  (s)->p += 3; \
}

#define stream_out_uint8(s, v) \
{ \
  *((s)->p) = (unsigned char)(v); \
  (s)->p++; \
}

#define stream_out_uint16_le(s, v) \
{ \
  *((s)->p) = (unsigned char)((v) >> 0); \
  (s)->p++; \
  *((s)->p) = (unsigned char)((v) >> 8); \
  (s)->p++; \
}

#define stream_out_uint16_be(s, v) \
{ \
  *((s)->p) = (unsigned char)((v) >> 8); \
  (s)->p++; \
  *((s)->p) = (unsigned char)((v) >> 0); \
  (s)->p++; \
}

#define stream_out_uint32_le(s, v) \
{ \
  *((s)->p) = (unsigned char)((v) >> 0); \
  (s)->p++; \
  *((s)->p) = (unsigned char)((v) >> 8); \
  (s)->p++; \
  *((s)->p) = (unsigned char)((v) >> 16); \
  (s)->p++; \
  *((s)->p) = (unsigned char)((v) >> 24); \
  (s)->p++; \
}

#define stream_out_uint32_be(s, v) \
{ \
  *((s)->p) = (unsigned char)((v) >> 24); \
  s->p++; \
  *((s)->p) = (unsigned char)((v) >> 16); \
  s->p++; \
  *((s)->p) = (unsigned char)((v) >> 8); \
  s->p++; \
  *((s)->p) = (unsigned char)(v); \
  (s)->p++; \
}

#define stream_in_uint8p(s, v, n) \
{ \
  (v) = (s)->p; \
  (s)->p += (n); \
}

#define stream_in_uint8a(s, v, n) \
{ \
  g_memcpy((v), (s)->p, (n)); \
  (s)->p += (n); \
}

#define stream_in_uint8s(s, n) \
{ \
  (s)->p += (n); \
}

#define stream_out_uint8p(s, v, n) \
{ \
  g_memcpy((s)->p, (v), (n)); \
  (s)->p += (n); \
}

#define stream_out_uint8a(s, v, n) \
{ \
  out_uint8p((s), (v), (n)); \
}

#define stream_out_uint8s(s, n) \
{ \
  g_memset((s)->p, 0, (n)); \
  (s)->p += (n); \
}

#endif
