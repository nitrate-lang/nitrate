@use "v1.0";

group TypeInfo {
  name:  string,
  size:  usize,
  align: usize,

  fn TypeInfo(T: [any]) {
    name =  T[0];
    size =  T[1];
    align = T[2];
  }

  fn name(): string;
  fn size(): usize;
  fn align(): usize;
  fn hash(): usize;

  fn eq(t: TypeInfo): bool;
  fn cast(t: TypeInfo): string;
};

@(fn typename(t: any): string => ret @qsys(200, t); );
@(fn sizeof(t: any): usize => ret @qsys(201, t); );
@(fn alignof(t: any): usize => ret @qsys(202, t); );

@(fn typeof(t: any): TypeInfo {
  ret TypeInfo(@typename(t), @sizeof(t), @alignof(t));
});

fn main() {
  // let name = @typeof(main).name().pout();
}