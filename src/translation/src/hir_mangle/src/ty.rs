use std::format;

use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use thin_vec::ThinVec;

use crate::string::{demangle_string, mangle_string};

/// Encodes a type into a compact, C99-safe, self-delimiting string.
///
/// The encoding uses single-character discriminants for primitive types and
/// length-prefixed, self-delimiting encodings for composite types. All output
/// characters are from the C99 identifier charset `[A-Za-z0-9_]`.
///
/// The encoding is **self-delimiting**: given a cursor into a byte stream,
/// `demangle_type` can determine exactly how many bytes belong to the type
/// encoding, allowing types to be safely nested and concatenated.
///
/// # Encoding Table
///
/// | Type            | Encoding                          |
/// |-----------------|-----------------------------------|
/// | `Never`         | `A`                               |
/// | `Unit`          | `B`                               |
/// | `Bool`          | `C`                               |
/// | `U8`            | `D`                               |
/// | `U16`           | `E`                               |
/// | `U32`           | `F`                               |
/// | `U64`           | `G`                               |
/// | `U128`          | `H`                               |
/// | `USize`         | `I`                               |
/// | `I8`            | `J`                               |
/// | `I16`           | `K`                               |
/// | `I32`           | `L`                               |
/// | `I64`           | `M`                               |
/// | `I128`          | `N`                               |
/// | `F32`           | `O`                               |
/// | `F64`           | `P`                               |
/// | `Array`         | `Q<element><len>_`                |
/// | `Tuple`         | `R<count>_<elements...>`          |
/// | `Struct`        | `S<name>`                         |
/// | `Enum`          | `T<name>`                         |
/// | `TypeAlias`     | `U<name>`                         |
/// | `Refine`        | `V<base>`                         |
/// | `Function`      | `W<param_count>_<params...><ret>` |
/// | `Reference`     | `X<flags><to>`                    |
/// | `SliceRef`      | `Y<flags><element>`               |
/// | `Pointer`       | `Z<flags><to>`                    |
/// | `SlicePtr`      | `a<flags><element>`               |
/// | `TraitObject`   | `b`                               |
/// | `Parameterized` | `c<base><arg_count>_<args...>`    |
/// | `GenericParam`  | `d<index>_`                       |
/// | `Inferred`      | `e<id>_`                          |
/// | `InferredFloat` | `f`                               |
/// | `InferredInt`   | `g`                               |
/// | `Range`         | `h`                               |
/// | `Str`           | `i`                               |
///
/// Where:
/// - `<name>` is a mangled string (self-delimiting, terminated with `_`)
/// - `<len>` is a decimal length followed by `_`
/// - `<count>` is a decimal count followed by `_`
/// - `<flags>` is a single char: `0`=shared-immutable, `1`=shared-mutable,
///   `2`=exclusive-immutable, `3`=exclusive-mutable
pub fn mangle_type(ty: &Type) -> String {
    match ty {
        Type::Never { .. } => "A".to_string(),
        Type::Unit { .. } => "B".to_string(),
        Type::Bool { .. } => "C".to_string(),
        Type::U8 { .. } => "D".to_string(),
        Type::U16 { .. } => "E".to_string(),
        Type::U32 { .. } => "F".to_string(),
        Type::U64 { .. } => "G".to_string(),
        Type::U128 { .. } => "H".to_string(),
        Type::USize { .. } => "I".to_string(),
        Type::I8 { .. } => "J".to_string(),
        Type::I16 { .. } => "K".to_string(),
        Type::I32 { .. } => "L".to_string(),
        Type::I64 { .. } => "M".to_string(),
        Type::I128 { .. } => "N".to_string(),
        Type::F32 { .. } => "O".to_string(),
        Type::F64 { .. } => "P".to_string(),

        Type::Array { element_type, len, .. } => {
            format!("Q{}{}_", mangle_type(element_type), len)
        }

        Type::Tuple { element_types, .. } => {
            let mut result = format!("R{}_", element_types.len());
            for element in element_types {
                result.push_str(&mangle_type(element));
            }
            result
        }

        Type::Struct { def, .. } => {
            let name = &def.borrow().name;
            format!("S{}", mangle_string(name))
        }

        Type::Enum { def, .. } => {
            let name = &def.borrow().name;
            format!("T{}", mangle_string(name))
        }

        Type::TypeAlias { def, .. } => {
            let name = &def.borrow().name;
            format!("U{}", mangle_string(name))
        }

        Type::Refine { base, .. } => {
            format!("V{}", mangle_type(base))
        }

        Type::UnresolvedArray { element_type, .. } => {
            format!("Q{}0_", mangle_type(element_type))
        }

        Type::UnresolvedRefine { base, .. } => {
            format!("V{}", mangle_type(base))
        }

        Type::Function { function_type, .. } => {
            let mut result = format!("W{}_", function_type.params.len());
            for (_, param) in function_type.params.iter() {
                result.push_str(&mangle_type(param));
            }
            result.push_str(&mangle_type(&function_type.return_type));
            result
        }

        Type::Reference {
            exclusive, mutable, to, ..
        } => {
            let flags = ref_flags(*exclusive, *mutable);
            format!("X{}{}", flags, mangle_type(to))
        }

        Type::SliceRef {
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let flags = ref_flags(*exclusive, *mutable);
            format!("Y{}{}", flags, mangle_type(element_type))
        }

        Type::Pointer {
            exclusive, mutable, to, ..
        } => {
            let flags = ref_flags(*exclusive, *mutable);
            format!("Z{}{}", flags, mangle_type(to))
        }

        Type::SlicePtr {
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let flags = ref_flags(*exclusive, *mutable);
            format!("a{}{}", flags, mangle_type(element_type))
        }

        Type::TraitObject { .. } => "b".to_string(),

        Type::Parameterized { base, args, .. } => {
            let mut result = format!("c{}{}_", mangle_type(base), args.positional.len() + args.named.len());
            for arg in args.positional.iter() {
                result.push_str(&mangle_type(arg));
            }
            for (_, arg) in args.named.iter() {
                result.push_str(&mangle_type(arg));
            }
            result
        }

        Type::GenericParam { index, .. } => format!("d{}_", index),

        Type::Inferred { id, .. } => format!("e{}_", id.get()),

        Type::InferredFloat { .. } => "f".to_string(),
        Type::InferredInteger { .. } => "g".to_string(),
        Type::Range { .. } => "h".to_string(),
        Type::Str { .. } => "i".to_string(),
    }
}

/// Decodes a type from a cursor, consuming exactly the bytes that belong to
/// the type encoding. The cursor is left positioned at the start of any data
/// that follows the type.
pub fn demangle_type(input: &mut &[u8]) -> Result<Type, ()> {
    if input.is_empty() {
        return Err(());
    }

    let discriminant = input[0];
    *input = &input[1..];

    match discriminant {
        b'A' => Ok(Type::Never {
            span: ByteSpan::default(),
        }),
        b'B' => Ok(Type::Unit {
            span: ByteSpan::default(),
        }),
        b'C' => Ok(Type::Bool {
            span: ByteSpan::default(),
        }),
        b'D' => Ok(Type::U8 {
            span: ByteSpan::default(),
        }),
        b'E' => Ok(Type::U16 {
            span: ByteSpan::default(),
        }),
        b'F' => Ok(Type::U32 {
            span: ByteSpan::default(),
        }),
        b'G' => Ok(Type::U64 {
            span: ByteSpan::default(),
        }),
        b'H' => Ok(Type::U128 {
            span: ByteSpan::default(),
        }),
        b'I' => Ok(Type::USize {
            span: ByteSpan::default(),
        }),
        b'J' => Ok(Type::I8 {
            span: ByteSpan::default(),
        }),
        b'K' => Ok(Type::I16 {
            span: ByteSpan::default(),
        }),
        b'L' => Ok(Type::I32 {
            span: ByteSpan::default(),
        }),
        b'M' => Ok(Type::I64 {
            span: ByteSpan::default(),
        }),
        b'N' => Ok(Type::I128 {
            span: ByteSpan::default(),
        }),
        b'O' => Ok(Type::F32 {
            span: ByteSpan::default(),
        }),
        b'P' => Ok(Type::F64 {
            span: ByteSpan::default(),
        }),

        b'Q' => {
            // Array: `Q<element><len>_`
            let element_type = demangle_type(input)?;
            let len = read_decimal(input)?;
            Ok(Type::Array {
                span: ByteSpan::default(),
                element_type: element_type.into(),
                len,
            })
        }

        b'R' => {
            // Tuple: `R<count>_<elements...>`
            let count = read_decimal(input)?;
            let mut element_types = ThinVec::with_capacity(count as usize);
            for _ in 0..count {
                element_types.push(demangle_type(input)?.into());
            }
            Ok(Type::Tuple {
                span: ByteSpan::default(),
                element_types,
            })
        }

        b'S' => {
            // Struct: `S<name>`
            let name = demangle_string(input)?;
            Ok(Type::Struct {
                span: ByteSpan::default(),
                def: StructDefId::from(StructDef {
                    span: ByteSpan::default(),
                    visibility: Visibility::Sec,
                    name: NString::from(name),
                    attributes: std::collections::BTreeSet::new(),
                    fields: std::collections::BTreeMap::new(),
                    generics: None,
                    layout: ThinVec::new(),
                }),
            })
        }

        b'T' => {
            // Enum: `T<name>`
            let name = demangle_string(input)?;
            Ok(Type::Enum {
                span: ByteSpan::default(),
                def: EnumDefId::from(EnumDef {
                    span: ByteSpan::default(),
                    visibility: Visibility::Sec,
                    name: NString::from(name),
                    attributes: std::collections::BTreeSet::new(),
                    generics: None,
                    variants: ThinVec::new(),
                }),
            })
        }

        b'U' => {
            // TypeAlias: `U<name>`
            let name = demangle_string(input)?;
            Ok(Type::TypeAlias {
                span: ByteSpan::default(),
                def: TypeAliasDefId::from(TypeAliasDef {
                    span: ByteSpan::default(),
                    visibility: Visibility::Sec,
                    name: NString::from(name),
                    generics: None,
                    type_id: Type::Unit {
                        span: ByteSpan::default(),
                    }
                    .into(),
                }),
            })
        }

        b'V' => {
            // Refine: `V<base>`
            let base = demangle_type(input)?;
            Ok(Type::Refine {
                span: ByteSpan::default(),
                base: base.into(),
                min: LiteralId::from(Lit::I64(0)),
                max: LiteralId::from(Lit::I64(0)),
            })
        }

        b'W' => {
            // Function: `W<param_count>_<params...><ret>`
            let param_count = read_decimal(input)?;
            let mut params = ThinVec::with_capacity(param_count as usize);
            for _ in 0..param_count {
                let param_ty = demangle_type(input)?;
                params.push((NString::from(""), param_ty.into()));
            }
            let return_type = demangle_type(input)?;
            Ok(Type::Function {
                span: ByteSpan::default(),
                function_type: FunctionType {
                    attributes: std::collections::BTreeSet::new(),
                    params,
                    return_type: return_type.into(),
                }
                .into(),
            })
        }

        b'X' => {
            // Reference: `X<flags><to>`
            let (exclusive, mutable) = read_ref_flags(input)?;
            let to = demangle_type(input)?;
            Ok(Type::Reference {
                span: ByteSpan::default(),
                lifetime: Lifetime::Inferred,
                exclusive,
                mutable,
                to: to.into(),
            })
        }

        b'Y' => {
            // SliceRef: `Y<flags><element>`
            let (exclusive, mutable) = read_ref_flags(input)?;
            let element_type = demangle_type(input)?;
            Ok(Type::SliceRef {
                span: ByteSpan::default(),
                lifetime: Lifetime::Inferred,
                exclusive,
                mutable,
                element_type: element_type.into(),
            })
        }

        b'Z' => {
            // Pointer: `Z<flags><to>`
            let (exclusive, mutable) = read_ref_flags(input)?;
            let to = demangle_type(input)?;
            Ok(Type::Pointer {
                span: ByteSpan::default(),
                lifetime: Lifetime::Inferred,
                exclusive,
                mutable,
                to: to.into(),
            })
        }

        b'a' => {
            // SlicePtr: `a<flags><element>`
            let (exclusive, mutable) = read_ref_flags(input)?;
            let element_type = demangle_type(input)?;
            Ok(Type::SlicePtr {
                span: ByteSpan::default(),
                lifetime: Lifetime::Inferred,
                exclusive,
                mutable,
                element_type: element_type.into(),
            })
        }

        b'b' => Ok(Type::TraitObject {
            span: ByteSpan::default(),
            bounds: Vec::new(),
        }),

        b'c' => {
            // Parameterized: `c<base><arg_count>_<args...>`
            let base = demangle_type(input)?;
            let arg_count = read_decimal(input)?;
            let mut positional = ThinVec::with_capacity(arg_count as usize);
            let named = ThinVec::new();
            for _ in 0..arg_count {
                positional.push(demangle_type(input)?.into());
            }
            Ok(Type::Parameterized {
                span: ByteSpan::default(),
                base: base.into(),
                args: Arguments { positional, named },
            })
        }

        b'd' => {
            // GenericParam: `d<index>_`
            let index = read_decimal(input)?;
            Ok(Type::GenericParam {
                span: ByteSpan::default(),
                index,
                name: NString::from(""),
            })
        }

        b'e' => {
            // Inferred: `e<id>_`
            let id = read_decimal(input)?;
            Ok(Type::Inferred {
                span: ByteSpan::default(),
                id: std::num::NonZeroU32::new(id).ok_or(())?,
                name: None,
            })
        }

        b'f' => Ok(Type::InferredFloat {
            span: ByteSpan::default(),
        }),
        b'g' => Ok(Type::InferredInteger {
            span: ByteSpan::default(),
        }),
        b'h' => Ok(Type::Range {
            span: ByteSpan::default(),
        }),

        _ => Err(()),
    }
}

/// Encodes reference flags as a single character.
fn ref_flags(exclusive: bool, mutable: bool) -> char {
    match (exclusive, mutable) {
        (false, false) => '0',
        (false, true) => '1',
        (true, false) => '2',
        (true, true) => '3',
    }
}

/// Reads reference flags from a cursor.
fn read_ref_flags(input: &mut &[u8]) -> Result<(bool, bool), ()> {
    if input.is_empty() {
        return Err(());
    }
    let flags = input[0];
    *input = &input[1..];
    match flags {
        b'0' => Ok((false, false)),
        b'1' => Ok((false, true)),
        b'2' => Ok((true, false)),
        b'3' => Ok((true, true)),
        _ => Err(()),
    }
}

/// Reads a decimal number followed by `_` from a cursor.
fn read_decimal(input: &mut &[u8]) -> Result<u32, ()> {
    let len_end = input.iter().position(|&b| b == b'_').ok_or(())?;
    let len_str = std::str::from_utf8(&input[..len_end]).map_err(|_| ())?;
    let value: u32 = len_str.parse().map_err(|_| ())?;
    *input = &input[len_end + 1..];
    Ok(value)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn with_store<R>(f: impl FnOnce() -> R) -> R {
        let store = Store::new();
        using_storage(&store, f)
    }

    fn test_type() -> Type {
        Type::Function {
            span: ByteSpan::default(),
            function_type: FunctionType {
                attributes: std::collections::BTreeSet::new(),
                params: ThinVec::new(),
                return_type: Type::Unit {
                    span: ByteSpan::default(),
                }
                .into(),
            }
            .into(),
        }
    }

    #[test]
    fn test_mangle_type_roundtrip() {
        with_store(|| {
            let cases = [
                Type::Never {
                    span: ByteSpan::default(),
                },
                Type::Unit {
                    span: ByteSpan::default(),
                },
                Type::Bool {
                    span: ByteSpan::default(),
                },
                Type::U8 {
                    span: ByteSpan::default(),
                },
                Type::U16 {
                    span: ByteSpan::default(),
                },
                Type::U32 {
                    span: ByteSpan::default(),
                },
                Type::U64 {
                    span: ByteSpan::default(),
                },
                Type::U128 {
                    span: ByteSpan::default(),
                },
                Type::USize {
                    span: ByteSpan::default(),
                },
                Type::I8 {
                    span: ByteSpan::default(),
                },
                Type::I16 {
                    span: ByteSpan::default(),
                },
                Type::I32 {
                    span: ByteSpan::default(),
                },
                Type::I64 {
                    span: ByteSpan::default(),
                },
                Type::I128 {
                    span: ByteSpan::default(),
                },
                Type::F32 {
                    span: ByteSpan::default(),
                },
                Type::F64 {
                    span: ByteSpan::default(),
                },
                Type::Array {
                    span: ByteSpan::default(),
                    element_type: Type::U8 {
                        span: ByteSpan::default(),
                    }
                    .into(),
                    len: 42,
                },
                Type::Tuple {
                    span: ByteSpan::default(),
                    element_types: ThinVec::from(vec![
                        Type::U8 {
                            span: ByteSpan::default(),
                        }
                        .into(),
                        Type::I32 {
                            span: ByteSpan::default(),
                        }
                        .into(),
                    ]),
                },
                Type::Function {
                    span: ByteSpan::default(),
                    function_type: FunctionType {
                        attributes: std::collections::BTreeSet::new(),
                        // Parameter names are deliberately not encoded in the
                        // mangled type signature (only types matter for symbol
                        // identity), so the roundtrip uses empty names.
                        params: ThinVec::from(vec![
                            (
                                NString::from(""),
                                Type::U8 {
                                    span: ByteSpan::default(),
                                }
                                .into(),
                            ),
                            (
                                NString::from(""),
                                Type::I32 {
                                    span: ByteSpan::default(),
                                }
                                .into(),
                            ),
                        ]),
                        return_type: Type::Bool {
                            span: ByteSpan::default(),
                        }
                        .into(),
                    }
                    .into(),
                },
                Type::Reference {
                    span: ByteSpan::default(),
                    lifetime: Lifetime::Inferred,
                    exclusive: true,
                    mutable: true,
                    to: Type::U8 {
                        span: ByteSpan::default(),
                    }
                    .into(),
                },
                Type::SliceRef {
                    span: ByteSpan::default(),
                    lifetime: Lifetime::Inferred,
                    exclusive: false,
                    mutable: true,
                    element_type: Type::U8 {
                        span: ByteSpan::default(),
                    }
                    .into(),
                },
                Type::Pointer {
                    span: ByteSpan::default(),
                    lifetime: Lifetime::Inferred,
                    exclusive: true,
                    mutable: false,
                    to: Type::U8 {
                        span: ByteSpan::default(),
                    }
                    .into(),
                },
                Type::SlicePtr {
                    span: ByteSpan::default(),
                    lifetime: Lifetime::Inferred,
                    exclusive: false,
                    mutable: false,
                    element_type: Type::U8 {
                        span: ByteSpan::default(),
                    }
                    .into(),
                },
                Type::TraitObject {
                    span: ByteSpan::default(),
                    bounds: Vec::new(),
                },
                Type::GenericParam {
                    span: ByteSpan::default(),
                    index: 3,
                    // The generic param name is deliberately not encoded in
                    // the mangled type signature (only the index matters for
                    // symbol identity).
                    name: NString::from(""),
                },
                Type::Inferred {
                    span: ByteSpan::default(),
                    id: std::num::NonZeroU32::new(7).unwrap(),
                    name: None,
                },
                Type::InferredFloat {
                    span: ByteSpan::default(),
                },
                Type::InferredInteger {
                    span: ByteSpan::default(),
                },
            ];

            for ty in cases {
                let mangled = mangle_type(&ty);
                let mut input = mangled.as_bytes();
                let demangled = demangle_type(&mut input).unwrap();
                assert_eq!(demangled, ty, "roundtrip failed for {:?}", ty);
                assert!(input.is_empty(), "trailing bytes for {:?}", ty);
            }
        })
    }

    #[test]
    fn test_mangle_type_c99_charset() {
        with_store(|| {
            let cases = [
                Type::Never {
                    span: ByteSpan::default(),
                },
                Type::Array {
                    span: ByteSpan::default(),
                    element_type: Type::U8 {
                        span: ByteSpan::default(),
                    }
                    .into(),
                    len: 42,
                },
                Type::Tuple {
                    span: ByteSpan::default(),
                    element_types: ThinVec::from(vec![
                        Type::U8 {
                            span: ByteSpan::default(),
                        }
                        .into(),
                        Type::I32 {
                            span: ByteSpan::default(),
                        }
                        .into(),
                    ]),
                },
                test_type(),
            ];

            for ty in cases {
                let mangled = mangle_type(&ty);
                assert!(
                    mangled.bytes().all(|b| b.is_ascii_alphanumeric() || b == b'_'),
                    "mangled type contains non-C99 chars: {}",
                    mangled
                );
            }
        })
    }

    #[test]
    fn test_mangle_type_self_delimiting() {
        with_store(|| {
            // Two types concatenated should be decodable independently.
            let ty1 = Type::Array {
                span: ByteSpan::default(),
                element_type: Type::U8 {
                    span: ByteSpan::default(),
                }
                .into(),
                len: 42,
            };
            let ty2 = Type::I32 {
                span: ByteSpan::default(),
            };

            let mangled = format!("{}{}", mangle_type(&ty1), mangle_type(&ty2));
            let mut input = mangled.as_bytes();
            let demangled1 = demangle_type(&mut input).unwrap();
            let demangled2 = demangle_type(&mut input).unwrap();
            assert_eq!(demangled1, ty1);
            assert_eq!(demangled2, ty2);
            assert!(input.is_empty());
        })
    }
}
