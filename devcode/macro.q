@(fn write(msg: text) { quix.api.write(msg, 0, 1) })

@(fn printn(line: text) { 
  quix.api.write(line, 0, 1);
  quix.api.write("\n", 0, 1);
})


@(fn exec(cmd: text) {})

@exec(quix.api.clear_term())
@printn("Hello, world!")
