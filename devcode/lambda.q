@use "v1.0";

@(fn(lua) std::core::publish_function(in: [code], out: []): void {
  res = qlua.qsys(0x78FF1D, code, 'private')

  if res != "OK" then
    qlua.abort("Failed to publish function")
  end

  qlua.done()
});

fn main() {   ~> Prints: "Hello, World!"
  const code = """(fn hello() {
    printn('Hello, World!');
  })""";

  @std::core::publish_function(code);

  hello();  ~> Defined with macros
}