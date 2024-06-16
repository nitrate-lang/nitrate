@use "v1.0";
@copyright "Wesley C. Jones";
@license "Unlicense";
@description "A simple cryptographic implementation of the ChaCha20 stream cipher.";

group chacha20 {
  cha:   [u32; 16],
  pos:   u8,
  key:   [u8; 32],
  nonce: [u8; 12],
  ctr:   u64,
  s:     [u32; 16],
};

fn pure pack4(a: [u8; 4]): u32 {
  let res: !u32 = 0;
  res |= a[0] as u32 << 00;
  res |= a[1] as u32 << 08;
  res |= a[2] as u32 << 16;
  res |= a[3] as u32 << 24;
  ret res;
}

fn pure chacha20_block_next(ctx: !chacha20) {
  foreach (i in 0..16) => ctx.cha[i] = ctx.s[i];

  fn pure quarterround(x: ![u32; 16], a: u32, b: u32, c: u32, d: u32) {
    x[a] += x[b];
    x[d] = (x[d] ^ x[a]) <<< 16;
    x[c] += x[d];
    x[b] = (x[b] ^ x[c]) <<< 12;
    x[a] += x[b];
    x[d] = (x[d] ^ x[a]) <<< 08;
    x[c] += x[d];
    x[b] = (x[b] ^ x[c]) <<< 07;
  };

  foreach (i in 0..10) {
    quarterround(ctx.cha, 00, 04, 08, 12);
    quarterround(ctx.cha, 01, 05, 09, 13);
    quarterround(ctx.cha, 02, 06, 10, 14);
    quarterround(ctx.cha, 03, 07, 11, 15);
    quarterround(ctx.cha, 00, 05, 10, 15);
    quarterround(ctx.cha, 01, 06, 11, 12);
    quarterround(ctx.cha, 02, 07, 08, 13);
    quarterround(ctx.cha, 03, 04, 09, 14);
  }

  foreach (i in 0..16) => ctx.cha[i] += ctx.s[i];

  ctx.s[12]++;
  if (ctx.s[12] == 0) {
    @invariant(ctx.s[13] != u32::max, "Nonce overflow");
    ctx.s[13]++;
  }
}

pub fn pure chacha20_init(ctx: !chacha20) => ctx = [...0];

pub fn pure chacha20_setup(ctx: !chacha20, key: [u8; 32], nonce: [u8; 12]) {
  const mag: [u8; 16] = "expand 32-byte k";

  ctx.key = key;
  ctx.nonce = nonce;
  ctx.pos = 64;

  ctx.s[00] = pack4(mag[00:04]);
  ctx.s[01] = pack4(mag[04:08]);
  ctx.s[02] = pack4(mag[08:12]);
  ctx.s[03] = pack4(mag[12:16]);
  ctx.s[04] = pack4(key[00:04]);
  ctx.s[05] = pack4(key[04:08]);
  ctx.s[06] = pack4(key[08:12]);
  ctx.s[07] = pack4(key[12:16]);
  ctx.s[08] = pack4(key[16:20]);
  ctx.s[09] = pack4(key[20:24]);
  ctx.s[10] = pack4(key[24:28]);
  ctx.s[11] = pack4(key[28:32]);
  ctx.s[12] = 00000000000000000;
  ctx.s[13] = pack4(nonce[0:4]);
  ctx.s[14] = pack4(nonce[4:8]);
  ctx.s[15] = pack4(nonce[8:12]);

  ctx.s[12] = ctx.ctr as u32;
  ctx.s[13] = pack4(ctx.nonce) + ((ctx.ctr >> 32) as u32);
}

pub fn pure chacha20_crypt(ctx: !chacha20, src: [u8], dst: ![u8]) {
  let cha = ctx.cha as [u8; 64];

  dst = src;
  foreach (i in 0..dst.len()) {
    if (ctx.pos == 64) {
      chacha20_block_next(ctx);
      ctx.pos = 0;
    }
    dst[i] ^= cha[ctx.pos++];
  }
}
