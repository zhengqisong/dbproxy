#ifndef _G_TRACE_MYH_E1897E85_F5B9_4FC6_A1B1_83E29D1DE3EB__INCLUDED_
#define _G_TRACE_MYH_E1897E85_F5B9_4FC6_A1B1_83E29D1DE3EB__INCLUDED_

#include <errno.h>

int init_trace_ex(const char *profile_path, const char *main_key);
void errmsg_ex(int trace_id, long status, int err, 
		const char *code_file, const int code_line,
		const char *fmt, ...);
void fatal_quit_f(const char *fmt, ...);
void fatal_quit_thread_f(const char *fmt, ...);
void fatalmsg_f(const char *fmt, ...);
void dbgmsg_f(const char *fmt, ...);
void enable_dbgmsg();
void mem_dump(u_int8_t *ucp, int32_t len);
void _mem_dump(u_int8_t *ucp, int32_t len, int32_t summary, u_int8_t *info);

#define fatal_quit(fmt, vargs...) \
	errmsg_ex(-1, -1, errno, __FILE__, __LINE__, fmt, ##vargs)
#define fatalmsg(fmt, vargs...) \
	errmsg_ex(-1, 0, errno, __FILE__, __LINE__, fmt, ##vargs)

#define dbgmsg(fmt, vargs...) \
	errmsg_ex_(MY_TRACE_ID, 0, 0, __FILE__, __LINE__, fmt, ##vargs)
#define dbgmsg_ex(trace_id, fmt, vargs...) \
	errmsg_ex_(trace_id, 0, 0, __FILE__, __LINE__, fmt, ##vargs)

#ifndef MY_TRACE_ID
extern int MY_TRACE_ID;
#endif

#define TRACE_ID_1   0x00000001
#define TRACE_ID_2   0x00000002
#define TRACE_ID_3   0x00000004
#define TRACE_ID_4   0x00000008
#define TRACE_ID_5   0x00000010
#define TRACE_ID_6   0x00000020
#define TRACE_ID_7   0x00000040
#define TRACE_ID_8   0x00000080
#define TRACE_ID_9   0x00000100
#define TRACE_ID_10  0x00000200
#define TRACE_ID_11  0x00000400
#define TRACE_ID_12  0x00000800
#define TRACE_ID_13  0x00001000
#define TRACE_ID_14  0x00002000
#define TRACE_ID_15  0x00004000
#define TRACE_ID_16  0x00008000
#define TRACE_ID_17  0x00010000
#define TRACE_ID_18  0x00020000
#define TRACE_ID_19  0x00040000
#define TRACE_ID_20  0x00080000
#define TRACE_ID_21  0x00100000
#define TRACE_ID_22  0x00200000
#define TRACE_ID_23  0x00400000
#define TRACE_ID_24  0x00800000
#define TRACE_ID_25  0x01000000
#define TRACE_ID_26  0x02000000
#define TRACE_ID_27  0x04000000
#define TRACE_ID_28  0x08000000
#define TRACE_ID_29  0x10000000
#define TRACE_ID_30  0x20000000
#define TRACE_ID_31  0x40000000
#define TRACE_ID_32  0x80000000

#define errmsg_ex_MY_TRACE_ID  errmsg_ex
#define errmsg_ex_0x00000001   errmsg_ex
#define errmsg_ex_0x00000002   errmsg_ex
#define errmsg_ex_0x00000004   errmsg_ex
#define errmsg_ex_0x00000008   errmsg_ex
#define errmsg_ex_0x00000010   errmsg_ex
#define errmsg_ex_0x00000020   errmsg_ex
#define errmsg_ex_0x00000040   errmsg_ex
#define errmsg_ex_0x00000080   errmsg_ex
#define errmsg_ex_0x00000100   errmsg_ex
#define errmsg_ex_0x00000200   errmsg_ex
#define errmsg_ex_0x00000400   errmsg_ex
#define errmsg_ex_0x00000800   errmsg_ex
#define errmsg_ex_0x00001000   errmsg_ex
#define errmsg_ex_0x00002000   errmsg_ex
#define errmsg_ex_0x00004000   errmsg_ex
#define errmsg_ex_0x00008000   errmsg_ex
#define errmsg_ex_0x00010000   errmsg_ex
#define errmsg_ex_0x00020000   errmsg_ex
#define errmsg_ex_0x00040000   errmsg_ex
#define errmsg_ex_0x00080000   errmsg_ex
#define errmsg_ex_0x00100000   errmsg_ex
#define errmsg_ex_0x00200000   errmsg_ex
#define errmsg_ex_0x00400000   errmsg_ex
#define errmsg_ex_0x00800000   errmsg_ex
#define errmsg_ex_0x01000000   errmsg_ex
#define errmsg_ex_0x02000000   errmsg_ex
#define errmsg_ex_0x04000000   errmsg_ex
#define errmsg_ex_0x08000000   errmsg_ex
#define errmsg_ex_0x10000000   errmsg_ex
#define errmsg_ex_0x20000000   errmsg_ex
#define errmsg_ex_0x40000000   errmsg_ex
#define errmsg_ex_0x80000000   errmsg_ex

#ifdef INVALID_TRACE_ALL
#define INVALID_MY_TRACE_ID
#define INVALID_TRACE_ID_1
#define INVALID_TRACE_ID_2
#define INVALID_TRACE_ID_3
#define INVALID_TRACE_ID_4
#define INVALID_TRACE_ID_5
#define INVALID_TRACE_ID_6
#define INVALID_TRACE_ID_7
#define INVALID_TRACE_ID_8
#define INVALID_TRACE_ID_9
#define INVALID_TRACE_ID_10
#define INVALID_TRACE_ID_11
#define INVALID_TRACE_ID_12
#define INVALID_TRACE_ID_13
#define INVALID_TRACE_ID_14
#define INVALID_TRACE_ID_15
#define INVALID_TRACE_ID_16
#define INVALID_TRACE_ID_17
#define INVALID_TRACE_ID_18
#define INVALID_TRACE_ID_19
#define INVALID_TRACE_ID_20
#define INVALID_TRACE_ID_21
#define INVALID_TRACE_ID_22
#define INVALID_TRACE_ID_23
#define INVALID_TRACE_ID_24
#define INVALID_TRACE_ID_25
#define INVALID_TRACE_ID_26
#define INVALID_TRACE_ID_27
#define INVALID_TRACE_ID_28
#define INVALID_TRACE_ID_29
#define INVALID_TRACE_ID_30
#define INVALID_TRACE_ID_31
#define INVALID_TRACE_ID_32
#endif

#ifdef INVALID_MY_TRACE_ID
#undef errmsg_ex_MY_TRACE_ID
#define errmsg_ex_MY_TRACE_ID(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_1
#undef errmsg_ex_0x00000001
#define errmsg_ex_0x00000001(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_2
#undef errmsg_ex_0x00000002
#define errmsg_ex_0x00000002(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_3
#undef errmsg_ex_0x00000004
#define errmsg_ex_0x00000004(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_4
#undef errmsg_ex_0x00000008
#define errmsg_ex_0x00000008(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_5
#undef errmsg_ex_0x00000010
#define errmsg_ex_0x00000010(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_6
#undef errmsg_ex_0x00000020
#define errmsg_ex_0x00000020(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_7
#undef errmsg_ex_0x00000040
#define errmsg_ex_0x00000040(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_8
#undef errmsg_ex_0x00000080
#define errmsg_ex_0x00000080(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_9
#undef errmsg_ex_0x00000100
#define errmsg_ex_0x00000100(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_10
#undef errmsg_ex_0x00000200
#define errmsg_ex_0x00000200(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_11
#undef errmsg_ex_0x00000400
#define errmsg_ex_0x00000400(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_12
#undef errmsg_ex_0x00000800
#define errmsg_ex_0x00000800(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_13
#undef errmsg_ex_0x00001000
#define errmsg_ex_0x00001000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_14
#undef errmsg_ex_0x00002000
#define errmsg_ex_0x00002000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_15
#undef errmsg_ex_0x00004000
#define errmsg_ex_0x00004000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_16
#undef errmsg_ex_0x00008000
#define errmsg_ex_0x00008000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_17
#undef errmsg_ex_0x00010000
#define errmsg_ex_0x00010000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_18
#undef errmsg_ex_0x00020000
#define errmsg_ex_0x00020000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_19
#undef errmsg_ex_0x00040000
#define errmsg_ex_0x00040000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_20
#undef errmsg_ex_0x00080000
#define errmsg_ex_0x00080000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_21
#undef errmsg_ex_0x00100000
#define errmsg_ex_0x00100000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_22
#undef errmsg_ex_0x00200000
#define errmsg_ex_0x00200000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_23
#undef errmsg_ex_0x00400000
#define errmsg_ex_0x00400000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_24
#undef errmsg_ex_0x00800000
#define errmsg_ex_0x00800000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_25
#undef errmsg_ex_0x01000000
#define errmsg_ex_0x01000000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_26
#undef errmsg_ex_0x02000000
#define errmsg_ex_0x02000000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_27
#undef errmsg_ex_0x04000000
#define errmsg_ex_0x04000000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_28
#undef errmsg_ex_0x08000000
#define errmsg_ex_0x08000000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_29
#undef errmsg_ex_0x10000000
#define errmsg_ex_0x10000000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_30
#undef errmsg_ex_0x20000000
#define errmsg_ex_0x20000000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_31
#undef errmsg_ex_0x40000000
#define errmsg_ex_0x40000000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif
#ifdef INVALID_TRACE_ID_32
#undef errmsg_ex_0x80000000
#define errmsg_ex_0x80000000(trace_id, status, err, code_file, code_line, fmt, vargs...)
#endif



#define errmsg_ex__(trace_id, status, err, code_file, code_line, fmt, vargs...) \
	errmsg_ex_##trace_id(trace_id, status, err, code_file, code_line, fmt, ##vargs)
#define errmsg_ex_(trace_id, status, err, code_file, code_line, fmt, vargs...) \
	errmsg_ex__(trace_id, status, err, code_file, code_line, fmt, ##vargs)

#endif

