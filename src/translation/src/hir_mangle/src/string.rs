use flate2::Compression;
use flate2::read::DeflateDecoder;
use flate2::write::DeflateEncoder;
use std::{
    format,
    io::{Read, Write},
};

/// The C99 identifier charset: `[A-Za-z0-9_]` (63 characters).
/// We use a base-63 encoding so that compressed payloads can be encoded
/// losslessly using only C99-safe chars.
const C99_ALPHABET: &[u8; 63] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";

/// Threshold (in bytes) above which a string is DEFLATE-compressed.
const COMPRESS_THRESHOLD: usize = 64;

/// The separator appended after the segment count prefix in a mangled
/// string. It allows the decoder to read the count, then read exactly that
/// many self-delimiting segments.
const SEGMENT_COUNT_SEPARATOR: u8 = b'_';

/// Returns `true` if `s` is a valid C99 identifier (starts with a letter or
/// underscore, and contains only alphanumerics or underscores).
fn is_c99_identifier(s: &str) -> bool {
    let mut chars = s.chars();
    match chars.next() {
        Some(c) if c.is_ascii_alphabetic() || c == '_' => {}
        _ => return false,
    }
    chars.all(|c| c.is_ascii_alphanumeric() || c == '_')
}

/// Encodes a byte slice using a C99-safe base-63 encoding.
///
/// The input is treated as a big-endian number and converted to base 63
/// using the C99 identifier alphabet. This is lossless and produces output
/// containing only `[A-Za-z0-9_]`.
fn encode_c99_base63(data: &[u8]) -> String {
    if data.is_empty() {
        return String::new();
    }

    // Convert to a big-endian base-63 representation.
    let mut digits: Vec<u8> = vec![0];
    for &byte in data {
        let mut carry = byte as u32;
        for digit in digits.iter_mut() {
            let value = (*digit as u32) * 256 + carry;
            *digit = (value % 63) as u8;
            carry = value / 63;
        }
        while carry > 0 {
            digits.push((carry % 63) as u8);
            carry /= 63;
        }
    }

    // Reverse to get big-endian order.
    digits.reverse();
    digits.into_iter().map(|d| C99_ALPHABET[d as usize] as char).collect()
}

/// Decodes a C99-safe base-63 string back into bytes.
fn decode_c99_base63(s: &str) -> Result<Vec<u8>, ()> {
    if s.is_empty() {
        return Ok(Vec::new());
    }

    let mut bytes: Vec<u8> = vec![0];
    for c in s.bytes() {
        let val = match c {
            b'A'..=b'Z' => c - b'A',
            b'a'..=b'z' => c - b'a' + 26,
            b'0'..=b'9' => c - b'0' + 52,
            b'_' => 62,
            _ => return Err(()),
        };
        let mut carry = val as u32;
        for byte in bytes.iter_mut() {
            let value = (*byte as u32) * 63 + carry;
            *byte = (value % 256) as u8;
            carry = value / 256;
        }
        while carry > 0 {
            bytes.push((carry % 256) as u8);
            carry /= 256;
        }
    }

    // Reverse to get big-endian order.
    bytes.reverse();
    Ok(bytes)
}

/// Compresses a byte slice using raw DEFLATE.
fn deflate_compress(data: &[u8]) -> Vec<u8> {
    let mut encoder = DeflateEncoder::new(Vec::new(), Compression::default());
    encoder.write_all(data).expect("infallible");
    encoder.finish().expect("infallible")
}

/// Decompresses a raw DEFLATE stream.
fn deflate_decompress(data: &[u8]) -> Result<Vec<u8>, ()> {
    let mut decoder = DeflateDecoder::new(data);
    let mut out = Vec::new();
    decoder.read_to_end(&mut out).map_err(|_| ())?;
    Ok(out)
}

/// Encodes a single name segment.
///
/// - If the segment is a valid C99 identifier, it is encoded as `<len><segment>`.
/// - Otherwise, it is hex-encoded as `x<hex_len>_<hex_bytes>`.
/// - If the resulting encoding is longer than `COMPRESS_THRESHOLD`, it is
///   DEFLATE-compressed and base63-encoded as `z<base63_len>_<base63>`.
///
/// The lowercase `x` and `z` prefixes are used (rather than uppercase) to
/// avoid ambiguity with composite type encoding prefixes (`X` = TypeAlias,
/// `Z` = SliceRef) when a string is concatenated with a type encoding.
fn mangle_segment(segment: &str) -> String {
    if segment.is_empty() {
        return "0".to_string();
    }

    let encoded = if is_c99_identifier(segment) {
        format!("{}{}", segment.len(), segment)
    } else {
        let mut hex = String::with_capacity(segment.len() * 2);
        for byte in segment.as_bytes() {
            hex.push_str(&format!("{:02x}", byte));
        }
        format!("x{}_{}", hex.len(), hex)
    };

    // Compress if the encoding is too long.
    if encoded.len() > COMPRESS_THRESHOLD {
        let compressed = deflate_compress(encoded.as_bytes());
        // Only use compression if it actually helps.
        if compressed.len() < encoded.len() {
            let base63 = encode_c99_base63(&compressed);
            // Length-prefix the base63 payload so it is self-delimiting
            // when concatenated with other segments.
            return format!("z{}_{}", base63.len(), base63);
        }
    }

    encoded
}

/// Decodes a single name segment from a cursor.
fn demangle_segment(input: &mut &[u8]) -> Result<String, ()> {
    if input.is_empty() {
        return Err(());
    }

    match input[0] {
        b'z' => {
            // Compressed segment: `z<base63_len>_<base63>`
            *input = &input[1..];
            let len_end = input.iter().position(|&b| b == b'_').ok_or(())?;
            let len_str = std::str::from_utf8(&input[..len_end]).map_err(|_| ())?;
            let base63_len: usize = len_str.parse().map_err(|_| ())?;
            *input = &input[len_end + 1..];

            if input.len() < base63_len {
                return Err(());
            }
            let base63 = std::str::from_utf8(&input[..base63_len]).map_err(|_| ())?;
            *input = &input[base63_len..];

            let compressed = decode_c99_base63(base63)?;
            let encoded = deflate_decompress(&compressed)?;
            let encoded = std::str::from_utf8(&encoded).map_err(|_| ())?;
            let mut encoded_bytes = encoded.as_bytes();
            demangle_segment(&mut encoded_bytes)
        }
        b'x' => {
            // Hex-encoded segment: `x<hex_len>_<hex_bytes>`
            *input = &input[1..];
            let len_end = input.iter().position(|&b| b == b'_').ok_or(())?;
            let len_str = std::str::from_utf8(&input[..len_end]).map_err(|_| ())?;
            let hex_len: usize = len_str.parse().map_err(|_| ())?;
            *input = &input[len_end + 1..];

            if input.len() < hex_len {
                return Err(());
            }
            let hex = &input[..hex_len];
            if hex_len % 2 != 0 {
                return Err(());
            }
            let mut bytes = Vec::with_capacity(hex_len / 2);
            for pair in hex.chunks(2) {
                let hi = (pair[0] as char).to_digit(16).ok_or(())? as u8;
                let lo = (pair[1] as char).to_digit(16).ok_or(())? as u8;
                bytes.push((hi << 4) | lo);
            }
            *input = &input[hex_len..];
            String::from_utf8(bytes).map_err(|_| ())
        }
        b'0'..=b'9' => {
            // Length-prefixed C99 identifier: `<len><segment>`
            let len_end = input.iter().position(|&b| !b.is_ascii_digit()).ok_or(())?;
            let len_str = std::str::from_utf8(&input[..len_end]).map_err(|_| ())?;
            let len: usize = len_str.parse().map_err(|_| ())?;
            *input = &input[len_end..];

            if input.len() < len {
                return Err(());
            }
            let segment = std::str::from_utf8(&input[..len]).map_err(|_| ())?;
            *input = &input[len..];
            Ok(segment.to_string())
        }
        _ => Err(()),
    }
}

/// Encodes a full string (which may contain `::` path separators) into a
/// mangled form. The encoding is prefixed with a segment count so that it is
/// fully self-delimiting when concatenated with other data (e.g. a type
/// encoding in a full symbol name).
///
/// Format: `<segment_count>_<segment_1>...<segment_n>`
///
/// where each segment is independently self-delimiting (length-prefixed,
/// hex-encoded, or compressed).
pub fn mangle_string(s: &str) -> String {
    let segments: Vec<&str> = s.split("::").collect();
    let mut out = format!("{}{}", segments.len(), SEGMENT_COUNT_SEPARATOR as char);
    for segment in segments {
        out.push_str(&mangle_segment(segment));
    }
    out
}

/// Decodes a full mangled string back into its original form.
///
/// Reads the segment count prefix, then exactly that many self-delimiting
/// segments. The input cursor is left positioned at the start of any data
/// that follows the string.
pub fn demangle_string(input: &mut &[u8]) -> Result<String, ()> {
    let count_end = input.iter().position(|&b| b == SEGMENT_COUNT_SEPARATOR).ok_or(())?;
    let count_str = std::str::from_utf8(&input[..count_end]).map_err(|_| ())?;
    let count: usize = count_str.parse().map_err(|_| ())?;
    *input = &input[count_end + 1..];

    let mut segments = Vec::with_capacity(count);
    for _ in 0..count {
        segments.push(demangle_segment(input)?);
    }
    Ok(segments.join("::"))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_c99_identifier() {
        assert!(is_c99_identifier("main"));
        assert!(is_c99_identifier("_foo"));
        assert!(is_c99_identifier("foo_bar123"));
        assert!(!is_c99_identifier("123foo"));
        assert!(!is_c99_identifier("foo-bar"));
        assert!(!is_c99_identifier("foo.bar"));
        assert!(!is_c99_identifier(""));
    }

    #[test]
    fn test_mangle_simple() {
        assert_eq!(mangle_segment("main"), "4main");
        assert_eq!(mangle_segment("_foo"), "4_foo");
        assert_eq!(mangle_segment(""), "0");
    }

    #[test]
    fn test_mangle_hex() {
        // "λ" is U+03BB, UTF-8: CE BB (2 bytes = 4 hex chars)
        assert_eq!(mangle_segment("λ"), "x4_cebb");
        // "foo-bar" contains a hyphen (7 bytes = 14 hex chars)
        assert_eq!(mangle_segment("foo-bar"), "x14_666f6f2d626172");
    }

    #[test]
    fn test_mangle_roundtrip() {
        let cases = [
            "main",
            "_foo",
            "foo_bar123",
            "λ",
            "foo-bar",
            "foo.bar",
            "123foo",
            "a very long name that exceeds the compression threshold and should be compressed to save space in the symbol table",
        ];

        for case in cases {
            let mangled = mangle_segment(case);
            let mut input = mangled.as_bytes();
            let demangled = demangle_segment(&mut input).unwrap();
            assert_eq!(demangled, case, "roundtrip failed for {:?}", case);
            assert!(input.is_empty(), "trailing bytes for {:?}", case);
        }
    }

    #[test]
    fn test_mangle_string_roundtrip() {
        let cases = [
            "foo",
            "foo::bar",
            "my_package::my_module::my_function",
            "λ::foo-bar",
            "a::b::c::d::e::f::g::h::i::j::k::l::m::n::o::p::q::r::s::t::u::v::w::x::y::z",
        ];

        for case in cases {
            let mangled = mangle_string(case);
            let mut input = mangled.as_bytes();
            let demangled = demangle_string(&mut input).unwrap();
            assert_eq!(demangled, case, "roundtrip failed for {:?}", case);
            assert!(input.is_empty(), "trailing bytes for {:?}", case);
        }
    }

    #[test]
    fn test_mangle_string_self_delimiting() {
        // The string encoding must be self-delimiting: after decoding, the
        // cursor should be positioned at the start of the following data.
        let mangled = mangle_string("foo");
        let mut input = mangled.as_bytes();
        let demangled = demangle_string(&mut input).unwrap();
        assert_eq!(demangled, "foo");
        assert!(input.is_empty(), "expected all data to be consumed");

        // Concatenated with a type encoding, the decoder should leave the
        // type encoding untouched.
        let encoded_str = format!("{}d", mangle_string("foo"));
        let mut input = encoded_str.as_bytes();
        let demangled = demangle_string(&mut input).unwrap();
        assert_eq!(demangled, "foo");
        assert_eq!(input, b"d", "type encoding should be left untouched");
    }

    #[test]
    fn test_compression_used_for_long_names() {
        let long_name = "a".repeat(200);
        let mangled = mangle_segment(&long_name);
        assert!(mangled.starts_with('z'), "expected compressed output, got: {}", mangled);
        assert!(mangled.len() < long_name.len(), "compression should reduce size");
    }

    #[test]
    fn test_c99_charset_only() {
        let cases = [
            "main",
            "λ",
            "foo-bar",
            "foo.bar",
            "a very long name that exceeds the compression threshold and should be compressed to save space in the symbol table",
        ];

        for case in cases {
            let mangled = mangle_segment(case);
            assert!(
                mangled.bytes().all(|b| b.is_ascii_alphanumeric() || b == b'_'),
                "mangled name contains non-C99 chars: {}",
                mangled
            );
        }
    }
}
