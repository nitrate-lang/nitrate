type image_handle_t = *void;
type system_table_t = *void;

pub "C" fn efi_main(image_handle: image_handle_t, system_table: system_table_t): i32 {
  ret "";
}