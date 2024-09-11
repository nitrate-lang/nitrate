@(fn define() {
  local name = quix.api.next();
  local sepr = quix.api.next();
  local valu = quix.api.next();
  local semi = quix.api.next();

  if name.ty ~= 'name' then
    quix.api.abort('Expected name in @define macro');
  end
  name = name.v;

  if sepr.ty ~= 'op' or sepr.v ~= '=' then
    quix.api.abort('Expected = in @define macro');
  end

  if semi.ty ~= 'sym' or semi.v ~= ';' then
    quix.api.abort('Expected ; in @define macro');
  end

  quix.api.set('def.'.. name, valu.v);
})

@define hello = 0xcc36deef_f502_4af1_8060_9174dadafad7;

hello +

@(quix.api.set('def.hello', '123'))

hello;
