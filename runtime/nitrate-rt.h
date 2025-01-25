#ifndef __NITRATE_RT_API_H__
#define __NITRATE_RT_API_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t _NLR_uword_t;

typedef _NLR_uword_t _NLR_argc_t;
typedef _NLR_uword_t _NLR_argv_pp_t;
typedef _NLR_uword_t _NLR_env_pp_t;

typedef int(_NLR_main_func_t)(_NLR_argc_t, _NLR_argv_pp_t, _NLR_env_pp_t);

struct _NLR_symbol_t {
  void* m_spot;
  int (*m_constructor)(void*);
  int (*m_destructor)(void*);
};

void _NLR_panic(_NLR_uword_t cstr_ptr) __attribute__((noreturn));
_NLR_uword_t _NLR_malloc(_NLR_uword_t size);
void _NLR_free(_NLR_uword_t ptr);
_NLR_uword_t _NLR_signal_oom(_NLR_uword_t req_size);
_NLR_uword_t _NLR_csprng(_NLR_uword_t buf_ptr, _NLR_uword_t max_size);

#ifdef __cplusplus
}
#endif

#endif
