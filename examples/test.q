@(fn test() {
  quix.api.set('def.macro_bestie', 'hello');
})

@(fn test2() {
  quix.api.set('def.macro_bestie2', 'world');
})

@test();
@test2();

macro_bestie ;macro_bestie2;