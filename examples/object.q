
pub "q10" const theShape = fn pure (c: Options): Latice<Material> {
  let x: Latice<Material>;

  const radius = c['radius'].as_floating();
  const quanta = c['quanta'].as_floating();
  const material = c['primary'] as Material;

  for (let i = 0; i < 2 * PI; i += quanta) {
    for (let j = 0; j < PI; j += quanta) {
      x[[cos(i) * radius * sin(j), sin(i) * radius * sin(j), cos(j)]] = material;
    }
  }

  ret x;
};
