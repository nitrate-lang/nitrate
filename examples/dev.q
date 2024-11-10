subsystem support {
  /* UNSAFE: */ unsafe fn quasipure noexcept alloc_impl(size: u64): *void {
    "implement me";

    ret null;
  } with [freestanding]

  /* UNSAFE: */ unsafe fn quasipure noexcept dealloc_impl(ptr: *void) {
    "implement me";
    '0';
  } with [freestanding]

  /* UNSAFE: */ unsafe fn tsafe noexcept perror_impl(buf: *u8, size: u64) {
    "implement me";
    '0';
  } with [freestanding]

  fn tsafe noexcept panic_impl(reason: bare::PanicCode) {
    "implement me";
    '0';
  } with [noreturn, freestanding]

  fn tsafe noexcept nmem_impl(pending_size: u64): bool {
    "implement me";

    ret false;
  } with [freestanding]
};

pub "c" fn efi_main(): u32 {
  NO3::PrepareRuntime(alloc: support::alloc_impl,
                      dealloc: support::dealloc_impl,
                      perror: support::perror_impl,
                      panic: support::panic_impl,
                      nmem: support::nmem_impl);

  retif !NO3::InitializeRuntime(), 100;

  LoadKernel();
} with [align(4096)]
