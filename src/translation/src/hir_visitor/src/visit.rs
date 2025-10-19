use nitrate_hir::prelude::*;
use std::num::NonZero;

pub trait HirTypeVisitor<T> {
    fn visit_never(&mut self) -> T;
    fn visit_unit(&mut self) -> T;
    fn visit_bool(&mut self) -> T;
    fn visit_u8(&mut self) -> T;
    fn visit_u16(&mut self) -> T;
    fn visit_u32(&mut self) -> T;
    fn visit_u64(&mut self) -> T;
    fn visit_u128(&mut self) -> T;
    fn visit_usize(&mut self) -> T;
    fn visit_i8(&mut self) -> T;
    fn visit_i16(&mut self) -> T;
    fn visit_i32(&mut self) -> T;
    fn visit_i64(&mut self) -> T;
    fn visit_i128(&mut self) -> T;
    fn visit_f32(&mut self) -> T;
    fn visit_f64(&mut self) -> T;
    fn visit_opaque(&mut self, name: &str) -> T;
    fn visit_array(&mut self, element_type: &TypeId, len: u64) -> T;
    fn visit_tuple(&mut self, element_types: &[TypeId]) -> T;
    fn visit_slice(&mut self, element_type: &TypeId) -> T;
    fn visit_struct(&mut self, struct_type: &StructType) -> T;
    fn visit_enum(&mut self, enum_type: &EnumType) -> T;
    fn visit_refine(&mut self, base: &TypeId, min: &LiteralId, max: &LiteralId) -> T;
    fn visit_bitfield(&mut self, base: &TypeId, len: u8) -> T;
    fn visit_function(&mut self, function_type: &FunctionType) -> T;
    fn visit_reference(&mut self, life: &Lifetime, excl: bool, mutable: bool, to: &TypeId) -> T;
    fn visit_pointer(&mut self, excl: bool, mutable: bool, to: &TypeId) -> T;
    fn visit_symbol(&mut self, path: &str, link: &Option<TypeId>) -> T;
    fn visit_inferred_float(&mut self) -> T;
    fn visit_inferred_integer(&mut self) -> T;
    fn visit_inferred(&mut self, id: NonZero<u32>) -> T;

    fn visit_type(&mut self, ty: &Type, store: &Store) -> T {
        match ty {
            Type::Never => self.visit_never(),
            Type::Unit => self.visit_unit(),
            Type::Bool => self.visit_bool(),
            Type::U8 => self.visit_u8(),
            Type::U16 => self.visit_u16(),
            Type::U32 => self.visit_u32(),
            Type::U64 => self.visit_u64(),
            Type::U128 => self.visit_u128(),
            Type::USize => self.visit_usize(),
            Type::I8 => self.visit_i8(),
            Type::I16 => self.visit_i16(),
            Type::I32 => self.visit_i32(),
            Type::I64 => self.visit_i64(),
            Type::I128 => self.visit_i128(),
            Type::F32 => self.visit_f32(),
            Type::F64 => self.visit_f64(),
            Type::Opaque { name } => self.visit_opaque(name),
            Type::Array { element_type, len } => self.visit_array(element_type, *len),
            Type::Tuple { element_types } => self.visit_tuple(element_types),
            Type::Slice { element_type } => self.visit_slice(element_type),
            Type::Struct { struct_type } => self.visit_struct(&store[struct_type]),
            Type::Enum { enum_type } => self.visit_enum(&store[enum_type]),
            Type::Refine { base, min, max } => self.visit_refine(base, min, max),
            Type::Bitfield { base, bits } => self.visit_bitfield(base, *bits),
            Type::Function { function_type } => self.visit_function(&store[function_type]),

            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => self.visit_reference(lifetime, *exclusive, *mutable, to),

            Type::Pointer {
                exclusive,
                mutable,
                to,
            } => self.visit_pointer(*exclusive, *mutable, to),

            Type::Symbol { path, link } => self.visit_symbol(path, link),
            Type::InferredFloat => self.visit_inferred_float(),
            Type::InferredInteger => self.visit_inferred_integer(),
            Type::Inferred { id } => self.visit_inferred(*id),
        }
    }
}
