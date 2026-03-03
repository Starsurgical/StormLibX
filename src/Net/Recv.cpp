#include "Recv.hpp"

#include "Conn.hpp"
#include "Spi.hpp"
#include "Sys.hpp"

#include <Storm/String.hpp>
#include <SNet.h>

#include <atomic>
#include <mutex>
#include <thread>

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#define SCOPE_LOCK(x) std::lock_guard<decltype(x)> CONCAT(x, __COUNTER__)(x)

extern std::recursive_mutex s_api_critsect;

std::thread s_recv_thread;
std::atomic_bool s_recv_shutdown;
HANDLE s_recv_event;  // FIXME: move off Windows


void RecvProcessExternalMessages() {
  const char* senderpath = nullptr;
  const char* sendername = nullptr;
  const char* message = nullptr;
  while (s_spi->spiReceiveExternalMessage(&senderpath, &sendername, &message)) {
    if (!senderpath || !sendername || !message) break;

    if (!senderpath[0] && !sendername[0]) {
      SysQueueUserEvent(4, -1, message, SStrLen(message) + 1);
    }

    s_spi->spiFreeExternalMessage(senderpath, sendername, message);
    senderpath = nullptr;
    sendername = nullptr;
    message = nullptr;
  }
}

void RecvThreadProc() {
  int timeout = -1;
  while (!s_recv_shutdown) {
    BOOL v2 = !WaitForSingleObject(s_recv_event, timeout);  // FIXME: remove Windows dep
    if (s_recv_shutdown) break;

    SCOPE_LOCK(s_api_critsect);

    if (!s_spi) break;

    if (v2) {
      RecvProcessExternalMessages();
      //RecvProcessIncomingPackets();
    }

    timeout = ConnMaintainConnections();
  }
}

BOOL RecvInitialize(HANDLE* eventptr) {
  if (!s_recv_event) {
    s_recv_event = CreateEvent(nullptr, 0, 0, nullptr);
  }

  if (!s_recv_thread.joinable()) {
    s_recv_shutdown = false;
    s_recv_thread = std::thread(RecvThreadProc);
  }

  if (eventptr) *eventptr = s_recv_event;
  return s_recv_event && s_recv_thread.joinable();
}

void RecvDestroy() {
  if (s_recv_thread.joinable()) {
    s_recv_shutdown = true;
    SetEvent(s_recv_event);
    s_recv_thread.join();
    s_recv_shutdown = false;
  }
}
