use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{
        ArrayType, Bool, Float8, Float16, Float32, Float64, Float128, FunctionType,
        FunctionTypeParameter, Int8, Int16, Int32, Int64, Int128, LatentType, Lifetime, OpaqueType,
        ReferenceType, RefinementType, SliceType, TupleType, Type, TypeName, TypeParentheses,
        TypeSyntaxError, UInt8, UInt16, UInt32, UInt64, UInt128,
    },
    ty::{InferType, UnitType},
};

impl ParseTreeIterMut for TypeSyntaxError {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeSyntaxError);
        f(Order::Post, RefNodeMut::TypeSyntaxError);
    }
}

impl ParseTreeIterMut for Bool {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeBool);
        f(Order::Post, RefNodeMut::TypeBool);
    }
}

impl ParseTreeIterMut for UInt8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeUInt8);
        f(Order::Post, RefNodeMut::TypeUInt8);
    }
}

impl ParseTreeIterMut for UInt16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeUInt16);
        f(Order::Post, RefNodeMut::TypeUInt16);
    }
}

impl ParseTreeIterMut for UInt32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeUInt32);
        f(Order::Post, RefNodeMut::TypeUInt32);
    }
}

impl ParseTreeIterMut for UInt64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeUInt64);
        f(Order::Post, RefNodeMut::TypeUInt64);
    }
}

impl ParseTreeIterMut for UInt128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeUInt128);
        f(Order::Post, RefNodeMut::TypeUInt128);
    }
}

impl ParseTreeIterMut for Int8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeInt8);
        f(Order::Post, RefNodeMut::TypeInt8);
    }
}

impl ParseTreeIterMut for Int16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeInt16);
        f(Order::Post, RefNodeMut::TypeInt16);
    }
}

impl ParseTreeIterMut for Int32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeInt32);
        f(Order::Post, RefNodeMut::TypeInt32);
    }
}

impl ParseTreeIterMut for Int64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeInt64);
        f(Order::Post, RefNodeMut::TypeInt64);
    }
}

impl ParseTreeIterMut for Int128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeInt128);
        f(Order::Post, RefNodeMut::TypeInt128);
    }
}

impl ParseTreeIterMut for Float8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeFloat8);
        f(Order::Post, RefNodeMut::TypeFloat8);
    }
}

impl ParseTreeIterMut for Float16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeFloat16);
        f(Order::Post, RefNodeMut::TypeFloat16);
    }
}

impl ParseTreeIterMut for Float32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeFloat32);
        f(Order::Post, RefNodeMut::TypeFloat32);
    }
}

impl ParseTreeIterMut for Float64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeFloat64);
        f(Order::Post, RefNodeMut::TypeFloat64);
    }
}

impl ParseTreeIterMut for Float128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeFloat128);
        f(Order::Post, RefNodeMut::TypeFloat128);
    }
}

impl ParseTreeIterMut for UnitType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeUnitType);
        f(Order::Post, RefNodeMut::TypeUnitType);
    }
}

impl ParseTreeIterMut for InferType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeInferType);
        f(Order::Post, RefNodeMut::TypeInferType);
    }
}

impl ParseTreeIterMut for TypeName {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeTypeName(&mut self.name));
        f(Order::Post, RefNodeMut::TypeTypeName(&mut self.name));
    }
}

impl ParseTreeIterMut for RefinementType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeRefinementType(self));

        self.basis_type.depth_first_iter_mut(f);
        self.width.as_mut().map(|w| w.depth_first_iter_mut(f));
        self.minimum.as_mut().map(|m| m.depth_first_iter_mut(f));
        self.maximum.as_mut().map(|m| m.depth_first_iter_mut(f));

        f(Order::Post, RefNodeMut::TypeRefinementType(self));
    }
}

impl ParseTreeIterMut for TupleType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeTupleType(self));

        for elem in &mut self.element_types {
            elem.depth_first_iter_mut(f);
        }

        f(Order::Post, RefNodeMut::TypeTupleType(self));
    }
}

impl ParseTreeIterMut for ArrayType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeArrayType(self));

        self.element_type.depth_first_iter_mut(f);
        self.len.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::TypeArrayType(self));
    }
}

impl ParseTreeIterMut for SliceType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeSliceType(self));

        self.element_type.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::TypeSliceType(self));
    }
}

impl ParseTreeIterMut for FunctionTypeParameter {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeFunctionTypeParameter(self));

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        self.param_type.depth_first_iter_mut(f);
        self.default.as_mut().map(|d| d.depth_first_iter_mut(f));

        f(Order::Post, RefNodeMut::TypeFunctionTypeParameter(self));
    }
}

impl ParseTreeIterMut for FunctionType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeFunctionType(self));

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        for param in &mut self.parameters {
            param.depth_first_iter_mut(f);
        }

        if let Some(ret_ty) = &mut self.return_type {
            ret_ty.depth_first_iter_mut(f);
        }

        f(Order::Post, RefNodeMut::TypeFunctionType(self));
    }
}

impl ParseTreeIterMut for Lifetime {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeLifetime(self));
        f(Order::Post, RefNodeMut::TypeLifetime(self));
    }
}

impl ParseTreeIterMut for ReferenceType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeReferenceType(self));

        self.lifetime.as_mut().map(|lt| lt.depth_first_iter_mut(f));
        self.to.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::TypeReferenceType(self));
    }
}

impl ParseTreeIterMut for OpaqueType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeOpaqueType(&mut self.name));
        f(Order::Post, RefNodeMut::TypeOpaqueType(&mut self.name));
    }
}

impl ParseTreeIterMut for LatentType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeLatentType(&mut self.body));

        self.body.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::TypeLatentType(&mut self.body));
    }
}

impl ParseTreeIterMut for TypeParentheses {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::TypeParentheses(&mut self.inner));

        self.inner.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::TypeParentheses(&mut self.inner));
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
