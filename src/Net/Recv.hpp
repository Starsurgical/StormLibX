#ifndef __STORMLIBX_NET_RECV_H__
#define __STORMLIBX_NET_RECV_H__

#include <cstdint>

extern HANDLE s_recv_event;  // FIXME: move off Windows

void RecvProcessExternalMessages();
void RecvThreadProc();
BOOL RecvInitialize(HANDLE* eventptr);
void RecvDestroy();

#endif
