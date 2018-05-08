#ifndef _G_IO_H
#define _G_IO_H

typedef struct header
{
	int stream_dir;
	struct timeval tv;
	int len;
} Header;

int read_header(FILE *, Header *);
int write_header(FILE *, Header *);

#endif
