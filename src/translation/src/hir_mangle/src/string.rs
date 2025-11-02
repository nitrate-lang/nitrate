fn mangle_segment(str: &str) -> String {
    if str.is_empty() {
        return "0".to_string();
    }

    let is_atypical = if str.as_bytes()[0].is_ascii_digit() {
        true
    } else {
        let is_regular = str
            .as_bytes()
            .iter()
            .all(|b| b.is_ascii_alphanumeric() || *b == b'_');

        !is_regular
    };

    if is_atypical {
        let mut mangled = String::new();
        for byte in str.as_bytes() {
            mangled.push_str(&format!("{:02x}", byte));
        }

        format!("X{}_{}", mangled.len(), mangled)
    } else {
        return format!("{}{}", str.len(), str);
    }
}

pub(crate) fn mangle_string(str: &str) -> String {
    let mut mangled = String::new();

    for name_segment in str.split("::") {
        let segment = mangle_segment(name_segment);
        mangled.push_str(&segment);
    }

    mangled
}

pub(crate) fn demangle_string(_mangled: &mut dyn std::io::Read) -> Result<String, ()> {
    // TODO: implement string demangling
    Err(())
}
