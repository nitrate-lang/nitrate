subsystem priv {
  @(fn qsys_define(name, id, func) {
    return 'm_qsyscalls.push(["' .. name .. '",' .. id .. ',' .. func .. ']);';
  });
}

type Engine = opaque(qprep_impl_t);

fn main(): i32 {
  const qsys_0_fn = fn (ctx: *Engine, args: [string]) => libs::verof("quix-lexer");;
  const qsys_1_fn = fn (ctx: *Engine, args: [string]) => libs::verof("quix-preprocessor");;

  @qsys_define('lexer_version', 0x0000, 'qsys_0_fn');
  @qsys_define('preprocessor_version', 0x0001, 'qsys_1_fn');

  ret 0;
}