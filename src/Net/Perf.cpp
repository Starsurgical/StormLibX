#include "Perf.hpp"

#include <atomic>

typedef struct _PERFDATAREC {
  std::atomic_uint32_t value;
  uint32_t type;
  int32_t scale;
  bool providerspecific;
} PERFDATAREC, *PERFDATAPTR;

static PERFDATAREC s_perf_data[SNET_PERFIDNUM] = {
  {},
  { 0, SNET_PERFTYPE_RAWCOUNT, -1, false },
  {},
  {},
  { 0, SNET_PERFTYPE_COUNTER, -1, false },
  { 0, SNET_PERFTYPE_COUNTER, -1, false },
  { 0, SNET_PERFTYPE_COUNTER, -1, false },
  { 0, SNET_PERFTYPE_COUNTER, -1, false },
  { 0, SNET_PERFTYPE_COUNTER, -4, false },
  { 0, SNET_PERFTYPE_COUNTER, -4, false },
  { 0, SNET_PERFTYPE_COUNTER, -4, false },
  { 0, SNET_PERFTYPE_COUNTER, -4, false },
  { 0, SNET_PERFTYPE_COUNTER, -1, true },
  { 0, SNET_PERFTYPE_COUNTER, -1, true },
  { 0, SNET_PERFTYPE_COUNTER, -4, true },
  { 0, SNET_PERFTYPE_COUNTER, -4, true },
};

void PerfAdd(uint32_t id, int32_t value) {
  s_perf_data[id].value += value;
}

void PerfIncrement(uint32_t id) {
  s_perf_data[id].value++;
}

void PerfSet(uint32_t id, int32_t value) {
  s_perf_data[id].value = value;
}
