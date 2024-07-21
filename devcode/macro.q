@(fn timestamp() {
  return quix.api.time('false')
})

fn main() {
  let compiled_at = @timestamp();
  let message = 'Hello, Quix!';

  printn('Compiled at: ' + compiled_at);
  printn(message);
}