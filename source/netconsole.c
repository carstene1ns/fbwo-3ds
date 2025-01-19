#include <3ds.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#define SOC_BUFFERSIZE 0x100000

static u32 *SOC_buffer = NULL;
static s32 sock = -1;

void netConsoleInit() {
	SOC_buffer = (u32*)memalign(0x1000, SOC_BUFFERSIZE);
	if(SOC_buffer)
		if(socInit(SOC_buffer, SOC_BUFFERSIZE) == 0)
			sock = link3dsStdio();
}

void netConsoleExit() {
	if(sock > -1) {
		close(sock);
		socExit();
	}
	if(SOC_buffer)
		free(SOC_buffer);
}
