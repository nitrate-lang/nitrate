use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    ast::{
        ArrayType, Bool, Float32, Float64, FunctionType, Int8, Int16, Int32, Int64, Int128,
        LatentType, Lifetime, OpaqueType, ReferenceType, RefinementType, SliceType, TupleType,
        Type, TypeParentheses, TypePath, TypeSyntaxError, UInt8, UInt16, UInt32, UInt64, UInt128,
    },
    ty::InferType,
};

impl ParseTreeIterMut for TypeSyntaxError {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeSyntaxError);
        f(Order::Leave, RefNodeMut::TypeSyntaxError);
    }
}

impl ParseTreeIterMut for Bool {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeBool);
        f(Order::Leave, RefNodeMut::TypeBool);
    }
}

impl ParseTreeIterMut for UInt8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeUInt8);
        f(Order::Leave, RefNodeMut::TypeUInt8);
    }
}

impl ParseTreeIterMut for UInt16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeUInt16);
        f(Order::Leave, RefNodeMut::TypeUInt16);
    }
}

impl ParseTreeIterMut for UInt32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeUInt32);
        f(Order::Leave, RefNodeMut::TypeUInt32);
    }
}

impl ParseTreeIterMut for UInt64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeUInt64);
        f(Order::Leave, RefNodeMut::TypeUInt64);
    }
}

impl ParseTreeIterMut for UInt128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeUInt128);
        f(Order::Leave, RefNodeMut::TypeUInt128);
    }
}

impl ParseTreeIterMut for Int8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeInt8);
        f(Order::Leave, RefNodeMut::TypeInt8);
    }
}

impl ParseTreeIterMut for Int16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeInt16);
        f(Order::Leave, RefNodeMut::TypeInt16);
    }
}

impl ParseTreeIterMut for Int32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeInt32);
        f(Order::Leave, RefNodeMut::TypeInt32);
    }
}

impl ParseTreeIterMut for Int64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeInt64);
        f(Order::Leave, RefNodeMut::TypeInt64);
    }
}

impl ParseTreeIterMut for Int128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeInt128);
        f(Order::Leave, RefNodeMut::TypeInt128);
    }
}

impl ParseTreeIterMut for Float32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeFloat32);
        f(Order::Leave, RefNodeMut::TypeFloat32);
    }
}

impl ParseTreeIterMut for Float64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeFloat64);
        f(Order::Leave, RefNodeMut::TypeFloat64);
    }
}

impl ParseTreeIterMut for InferType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeInferType);
        f(Order::Leave, RefNodeMut::TypeInferType);
    }
}

impl ParseTreeIterMut for TypePath {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypePath(self));

        for segment in &mut self.segments {
            let _ = segment.name;

            if let Some(args) = &mut segment.type_arguments {
                for arg in args {
                    arg.depth_first_iter_mut(f);
                }
            }
        }

        let _ = self.resolved;

        f(Order::Leave, RefNodeMut::TypePath(self));
    }
}

impl ParseTreeIterMut for RefinementType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeRefinementType(self));

        self.basis_type.depth_first_iter_mut(f);

        if let Some(width) = &mut self.width {
            width.depth_first_iter_mut(f);
        }

        if let Some(minimum) = &mut self.minimum {
            minimum.depth_first_iter_mut(f);
        }

        if let Some(maximum) = &mut self.maximum {
            maximum.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::TypeRefinementType(self));
    }
}

impl ParseTreeIterMut for TupleType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeTupleType(self));

        for elem in &mut self.element_types {
            elem.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::TypeTupleType(self));
    }
}

impl ParseTreeIterMut for ArrayType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeArrayType(self));

        self.element_type.depth_first_iter_mut(f);
        self.len.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::TypeArrayType(self));
    }
}

impl ParseTreeIterMut for SliceType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeSliceType(self));

        self.element_type.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::TypeSliceType(self));
    }
}

impl ParseTreeIterMut for FunctionType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeFunctionType(self));

        if let Some(attributes) = &mut self.attributes {
            attributes.depth_first_iter_mut(f);
        }

        self.parameters.depth_first_iter_mut(f);

        if let Some(ret_ty) = &mut self.return_type {
            ret_ty.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::TypeFunctionType(self));
    }
}

impl ParseTreeIterMut for Lifetime {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeLifetime(self));
        f(Order::Leave, RefNodeMut::TypeLifetime(self));
    }
}

impl ParseTreeIterMut for ReferenceType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeReferenceType(self));

        let _ = self.mutability;
        let _ = self.exclusivity;

        if let Some(lt) = &mut self.lifetime {
            lt.depth_first_iter_mut(f);
        }

        self.to.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::TypeReferenceType(self));
    }
}

impl ParseTreeIterMut for OpaqueType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeOpaqueType(&mut self.name));

        let _ = self.name;

        f(Order::Leave, RefNodeMut::TypeOpaqueType(&mut self.name));
    }
}

impl ParseTreeIterMut for LatentType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeLatentType(&mut self.body));

        self.body.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::TypeLatentType(&mut self.body));
    }
}

impl ParseTreeIterMut for TypeParentheses {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::TypeParentheses(&mut self.inner));

        self.inner.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::TypeParentheses(&mut self.inner));
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
            Type::Float32(ty) => ty.depth_first_iter_mut(f),
            Type::Float64(ty) => ty.depth_first_iter_mut(f),
            Type::InferType(ty) => ty.depth_first_iter_mut(f),
            Type::TypePath(ty) => ty.depth_first_iter_mut(f),
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
