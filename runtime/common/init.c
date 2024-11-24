#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t (*nlr_alloc_fn)(uint64_t size);
typedef void (*nlr_dealloc_fn)(uint64_t ptr);
typedef uint64_t (*nlr_nmem_fn)(uint64_t req_size);
typedef void (*nlr_panic_fn)(uint64_t code);
typedef uint64_t (*nlr_csprng_fn)(uint64_t buf_ptr, uint64_t size);

extern void (**nlr_ctors_start)();
extern void (**nlr_ctors_end)();
extern void runo3_pre_ctors();
extern void runo3_post_ctors();
extern int runo3_main(int argc, char** argv, char** envp);

static nlr_alloc_fn g_alloc;
static nlr_dealloc_fn g_dealloc;
static nlr_nmem_fn g_nmem;
static nlr_panic_fn g_panic;
static nlr_csprng_fn g_csprng;

#define NLR_RESERVOIR_SIZE 0xffff
static struct reservoir_t {
  uint8_t* ptr;
  size_t size;
} g_reservoir;

void* nlr_alloc(size_t size) { return (void*)g_alloc(size); }
void nlr_dealloc(void* ptr) { g_dealloc((uint64_t)ptr); }
void* nlr_nmem(size_t req_size) { return (void*)g_nmem(req_size); }
void nlr_panic(uint64_t code) { g_panic(code); }
size_t nlr_csprng(void* buf, size_t size) {
  return (size_t)g_csprng((uint64_t)buf, size);
}

void* nlr_reservoir_alloc(size_t size) {
  if (size > g_reservoir.size) {
    return NULL;
  }

  g_reservoir.size -= size;
  return g_reservoir.ptr -= size;
}

void nlr_reservoir_reset() {
  g_reservoir.size = NLR_RESERVOIR_SIZE;
  g_reservoir.ptr += NLR_RESERVOIR_SIZE;
}

void nlr_initialize(nlr_alloc_fn _alloc, nlr_dealloc_fn _dealloc,
                    nlr_nmem_fn _nmem, nlr_panic_fn _panic,
                    nlr_csprng_fn _csprng) {
  g_alloc = _alloc;
  g_dealloc = _dealloc;
  g_nmem = _nmem;
  g_panic = _panic;
  g_csprng = _csprng;

  if (!g_alloc || !g_dealloc || !g_nmem || !g_panic || !g_csprng) {
    nlr_panic(0);
  }

  // Reserve a reservoir of memory for emergency use
  if ((g_reservoir.ptr = (uint8_t*)nlr_alloc(NLR_RESERVOIR_SIZE)) == NULL) {
    nlr_panic(0);
  }
  g_reservoir.size = NLR_RESERVOIR_SIZE;
}

int nlr_start(int argc, char** argv, char** envp) {
  // Call the pre-constructor function
  runo3_pre_ctors();

  // Call the constructors
  for (void (**ctor)() = nlr_ctors_start; ctor < nlr_ctors_end; ++ctor) {
    (*ctor)();
  }

  // Call the post-constructor function
  runo3_post_ctors();

  // Call the main function
  return runo3_main(argc, argv, envp);
}
