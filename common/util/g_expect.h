#ifndef _G_EXPECT_H
#define _G_EXPECT_H
#include <sys/types.h>

int expect(int in, char *checklist[], int list_len, int timeout, char *buf, int *buf_len);

#endif
