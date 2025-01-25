#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "nitrate-rt.h"

#define EXPORT_API __attribute__((visibility("default")))

/* Demangled: {"name":"app::main","type":"fn (_0: u64, _1: **u8, _2: **u8):
 * i32"} */
extern _NLR_main_func_t _Q3app4mainFimPPhPPhE_0;  // NOLINT

extern struct _NLR_symbol_t* _NLR_symbols_start;  // NOLINT
extern struct _NLR_symbol_t* _NLR_symbols_end;    // NOLINT

static _NLR_uword_t _NLR_csprng_next(void);
static _NLR_argv_pp_t _NLR_get_argv(_NLR_uword_t* argc);
static _NLR_env_pp_t _NLR_get_envp(_NLR_uword_t* envc);

static void stdio_write(const char* str) {
  size_t len = 0;
  for (const char* ptr = str; *ptr; ++ptr) {
    ++len;
  }

  syscall(1, 1, str, len);
}

EXPORT_API void _NLR_panic(_NLR_uword_t cstr_ptr) {  // NOLINT
  if (cstr_ptr == 0) {
    stdio_write("[nlr/panic]: Aborting...\n");
  } else {
    stdio_write("\n");
    stdio_write("\n");
    stdio_write("\n");
    stdio_write(
        "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
        "▚▚▚▚▚▚▚▚▚▚\n");
    stdio_write(
        "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
        "▚▚▚▚▚▚▚▚▚▚\n");
    stdio_write("\n");
    stdio_write("\x1b[31;1m┏━━━━━━┫ APPLICATION PANIC ┣━━\x1b[0m\n");
    stdio_write("\x1b[31;1m┃\x1b[0m\n");
    stdio_write("\x1b[31;1m┣╸╸\x1b[0m \x1b[37;1mPanic message:\x1b[0m \t");
    stdio_write((const char*)cstr_ptr);
    stdio_write("\n");
    stdio_write("\x1b[31;1m┃\x1b[0m\n");
    stdio_write("\x1b[31;1m┗━━━━━━┫ END APPLICATION PANIC ┣━━\x1b[0m\n");
    stdio_write("\n");
    stdio_write(
        "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
        "▚▚▚▚▚▚▚▚▚▚\n");
    stdio_write(
        "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
        "▚▚▚▚▚▚▚▚▚▚\n");
    stdio_write("\n");
    stdio_write("\n");
  }

  __builtin_trap();
}

EXPORT_API _NLR_uword_t _NLR_malloc(_NLR_uword_t size) {  // NOLINT
  uint8_t* map;

  size += sizeof(_NLR_uword_t);

  map = (uint8_t*)syscall(SYS_mmap, NULL, size, PROT_READ | PROT_WRITE,
                          MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  if (map == MAP_FAILED) {
    return 0;
  }

  *(_NLR_uword_t*)(map) = size;

  return (_NLR_uword_t)(map + 8);
}

EXPORT_API void _NLR_free(_NLR_uword_t ptr) {  // NOLINT
  uint8_t* addr;
  _NLR_uword_t mmap_size;

  addr = (uint8_t*)ptr;
  if (addr == NULL) {
    return;
  }

  addr -= 8;
  mmap_size = *(_NLR_uword_t*)addr;

  if (syscall(SYS_munmap, addr, mmap_size) != 0) {
    _NLR_panic((_NLR_uword_t) "[nlr/mem]: double free or corruption");
  }
}

EXPORT_API _NLR_uword_t _NLR_signal_oom(_NLR_uword_t req_size) {  // NOLINT
  (void)req_size;  // Nothing to do here

  return 0;
}

EXPORT_API _NLR_uword_t _NLR_csprng(_NLR_uword_t buf_ptr,
                                    _NLR_uword_t max_size) {  // NOLINT
  uint8_t* buf;
  _NLR_uword_t remaining, i, rand;

  buf = (uint8_t*)buf_ptr;
  remaining = max_size % sizeof(_NLR_uword_t);

  for (i = 0; i < max_size / sizeof(_NLR_uword_t); ++i) {
    *(_NLR_uword_t*)(buf + i * sizeof(_NLR_uword_t)) = _NLR_csprng_next();
  }

  if (remaining != 0) {
    rand = _NLR_csprng_next();

    for (i = 0; i < remaining; ++i) {
      buf[max_size - remaining + i] = (rand >> (i * 8)) & 0xFF;
    }
  }

  return max_size;
}

EXPORT_API void _start(void) {  // NOLINT
  struct _NLR_symbol_t* symbol;
  _NLR_uword_t argc, envc;
  _NLR_argv_pp_t argv;
  _NLR_env_pp_t envp;
  int rc;

  argv = _NLR_get_argv(&argc);
  envp = _NLR_get_envp(&envc);

  symbol = _NLR_symbols_start;
  while (symbol != _NLR_symbols_end) {
    if (symbol->m_constructor != NULL) {
      if (symbol->m_constructor(symbol->m_spot) != 0) {
        _NLR_panic(
            (_NLR_uword_t) "[nlr/init]: global object construction failed");
      }
    }
    symbol++;
  }

  rc = _Q3app4mainFimPPhPPhE_0(argc, argv, envp);

  symbol = _NLR_symbols_end;
  while (symbol-- != _NLR_symbols_start) {
    if (symbol->m_destructor != NULL) {
      if (symbol->m_destructor(symbol->m_spot) != 0) {
        _NLR_panic(
            (_NLR_uword_t) "[nlr/init]: global object destruction failed");
      }
    }
  }

  syscall(SYS_exit_group, rc);
}

static _NLR_uword_t _NLR_csprng_next(void) {
  union {
    _NLR_uword_t value;
    uint8_t bytes[sizeof(_NLR_uword_t)];
  } result;
  int rc;

  rc = syscall(SYS_getrandom, result.bytes, sizeof(result.bytes), 0);
  if (rc != sizeof(result.bytes)) {
    _NLR_panic((_NLR_uword_t) "[nlr/csprng]: SYS_getrandom failed");
  }

  return result.value;
}

static _NLR_argv_pp_t _NLR_get_argv(_NLR_uword_t* argc) {
  static char* argv[] = {NULL};

  /// TODO: Implement this function
  *argc = 0;

  return (_NLR_argv_pp_t)argv;
}

static _NLR_env_pp_t _NLR_get_envp(_NLR_uword_t* envc) {
  static char* envp[] = {NULL};

  /// TODO: Implement this function
  *envc = 0;

  return (_NLR_env_pp_t)envp;
}
