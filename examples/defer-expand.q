@(fn force_all_strlit_uppercase() {
  force_all_strlit_uppercase_toggle = false;
  quix.defer(function (T)
    if (force_all_strlit_uppercase_toggle) then
      force_all_strlit_uppercase_toggle = false
      return true
    end

    if (T.ty == 'str') then
      quix.emit(quix.enstr(string.upper(T.v)))
      force_all_strlit_uppercase_toggle = true
      return false
    else
      return true
    end
  end)
})

@force_all_strlit_uppercase();

fn main() {
  print("hail me mortal. for i have sinned."); // prints "HAIL ME MORTAL. FOR I HAVE SINNED."
}
