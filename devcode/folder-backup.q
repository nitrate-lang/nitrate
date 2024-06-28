@use "v1.0";

// @import filesystem as fs;
// @import sha3_256 from crypto;

fn main(args: [string]): i32 {
  if args.size() < 2 {
    eprintn(f"Usage: {args[0]} <directory>");
    ret 1;
  }

  let input = args[0];

  // retif !fs::dir_exists(input), eprintn(e), 2;

  let backup = "/tmp/" + sha3_256(input)[0: 16];

  if !fs::dir_copy(input, f"/tmp/{backup}.dir", overwrite: true) {
  	eprintn(e);
  	ret 3;
  }

  if !fs::gzip_dir(backup, lvl: 4, inplace: true) {
  	eprintn(e);
  	ret 4;
  }

  printn(f"Backup completed for {input} into {backup}");

  ret 0;
}