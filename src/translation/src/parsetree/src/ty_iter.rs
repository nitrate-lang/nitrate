use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{
        ArrayType, Bool, Float8, Float16, Float32, Float64, Float128, FunctionType,
        FunctionTypeParameter, Int8, Int16, Int32, Int64, Int128, LatentType, Lifetime, OpaqueType,
        ReferenceType, RefinementType, SliceType, TupleType, Type, TypeName, TypeParentheses,
        TypeSyntaxError, UInt8, UInt16, UInt32, UInt64, UInt128,
    },
    ty::{GenericArgument, InferType, UnitType},
};

impl ParseTreeIterMut for TypeSyntaxError {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Bool {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for UInt8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for UInt16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for UInt32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for UInt64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for UInt128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Int8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Int16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Int32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Int64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Int128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Float8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Float16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Float32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Float64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Float128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for UnitType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for InferType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for TypeName {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for RefinementType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for TupleType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for ArrayType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for SliceType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for FunctionTypeParameter {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for FunctionType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Lifetime {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for ReferenceType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for OpaqueType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for LatentType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for GenericArgument {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for TypeParentheses {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Type {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        match self {
            Type::SyntaxError(ty) => ty.depth_first_iter_mut(f),
            Type::Bool(ty) => ty.depth_first_iter_mut(f),
            Type::UInt8(ty) => ty.depth_first_iter_mut(f),
            Type::UInt16(ty) => ty.depth_first_iter_mut(f),
            Type::UInt32(ty) => ty.depth_first_iter_mut(f),
            Type::UInt64(ty) => ty.depth_first_iter_mut(f),
            Type::UInt128(ty) => ty.depth_first_iter_mut(f),
            Type::Int8(ty) => ty.depth_first_iter_mut(f),
            Type::Int16(ty) => ty.depth_first_iter_mut(f),
            Type::Int32(ty) => ty.depth_first_iter_mut(f),
            Type::Int64(ty) => ty.depth_first_iter_mut(f),
            Type::Int128(ty) => ty.depth_first_iter_mut(f),
            Type::Float8(ty) => ty.depth_first_iter_mut(f),
            Type::Float16(ty) => ty.depth_first_iter_mut(f),
            Type::Float32(ty) => ty.depth_first_iter_mut(f),
            Type::Float64(ty) => ty.depth_first_iter_mut(f),
            Type::Float128(ty) => ty.depth_first_iter_mut(f),
            Type::UnitType(ty) => ty.depth_first_iter_mut(f),
            Type::InferType(ty) => ty.depth_first_iter_mut(f),
            Type::TypeName(ty) => ty.depth_first_iter_mut(f),
            Type::RefinementType(ty) => ty.depth_first_iter_mut(f),
            Type::TupleType(ty) => ty.depth_first_iter_mut(f),
            Type::ArrayType(ty) => ty.depth_first_iter_mut(f),
            Type::SliceType(ty) => ty.depth_first_iter_mut(f),
            Type::FunctionType(ty) => ty.depth_first_iter_mut(f),
            Type::ReferenceType(ty) => ty.depth_first_iter_mut(f),
            Type::OpaqueType(ty) => ty.depth_first_iter_mut(f),
            Type::LatentType(ty) => ty.depth_first_iter_mut(f),
            Type::Lifetime(ty) => ty.depth_first_iter_mut(f),
            Type::Parentheses(ty) => ty.depth_first_iter_mut(f),
        }
    }
}
