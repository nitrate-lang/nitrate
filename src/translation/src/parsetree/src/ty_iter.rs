use crate::{
    Order, ParseTreeIter, RefNode,
    ast::{
        ArrayType, Bool, Float32, Float64, FunctionType, Int8, Int16, Int32, Int64, Int128,
        LatentType, Lifetime, OpaqueType, ReferenceType, RefinementType, SliceType, TupleType,
        Type, TypeParentheses, TypePath, TypeSyntaxError, UInt8, UInt16, UInt32, UInt64, UInt128,
        USize,
    },
    ty::InferType,
};

impl ParseTreeIter for TypeSyntaxError {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeSyntaxError);
        f(Order::Leave, RefNode::TypeSyntaxError);
    }
}

impl ParseTreeIter for Bool {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeBool);
        f(Order::Leave, RefNode::TypeBool);
    }
}

impl ParseTreeIter for UInt8 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeUInt8);
        f(Order::Leave, RefNode::TypeUInt8);
    }
}

impl ParseTreeIter for UInt16 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeUInt16);
        f(Order::Leave, RefNode::TypeUInt16);
    }
}

impl ParseTreeIter for UInt32 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeUInt32);
        f(Order::Leave, RefNode::TypeUInt32);
    }
}

impl ParseTreeIter for UInt64 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeUInt64);
        f(Order::Leave, RefNode::TypeUInt64);
    }
}

impl ParseTreeIter for UInt128 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeUInt128);
        f(Order::Leave, RefNode::TypeUInt128);
    }
}

impl ParseTreeIter for USize {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeUSize);
        f(Order::Leave, RefNode::TypeUSize);
    }
}

impl ParseTreeIter for Int8 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeInt8);
        f(Order::Leave, RefNode::TypeInt8);
    }
}

impl ParseTreeIter for Int16 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeInt16);
        f(Order::Leave, RefNode::TypeInt16);
    }
}

impl ParseTreeIter for Int32 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeInt32);
        f(Order::Leave, RefNode::TypeInt32);
    }
}

impl ParseTreeIter for Int64 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeInt64);
        f(Order::Leave, RefNode::TypeInt64);
    }
}

impl ParseTreeIter for Int128 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeInt128);
        f(Order::Leave, RefNode::TypeInt128);
    }
}

impl ParseTreeIter for Float32 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeFloat32);
        f(Order::Leave, RefNode::TypeFloat32);
    }
}

impl ParseTreeIter for Float64 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeFloat64);
        f(Order::Leave, RefNode::TypeFloat64);
    }
}

impl ParseTreeIter for InferType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeInferType);
        f(Order::Leave, RefNode::TypeInferType);
    }
}

impl ParseTreeIter for TypePath {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeTypeName(self));

        for segment in &self.segments {
            let _ = segment.name;

            if let Some(args) = &segment.type_arguments {
                for arg in args {
                    arg.depth_first_iter(f);
                }
            }
        }

        let _ = self.resolved;

        f(Order::Leave, RefNode::TypeTypeName(self));
    }
}

impl ParseTreeIter for RefinementType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeRefinementType(self));

        self.basis_type.depth_first_iter(f);

        if let Some(width) = &self.width {
            width.depth_first_iter(f);
        }

        if let Some(minimum) = &self.minimum {
            minimum.depth_first_iter(f);
        }

        if let Some(maximum) = &self.maximum {
            maximum.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::TypeRefinementType(self));
    }
}

impl ParseTreeIter for TupleType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeTupleType(self));

        for elem in &self.element_types {
            elem.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::TypeTupleType(self));
    }
}

impl ParseTreeIter for ArrayType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeArrayType(self));

        self.element_type.depth_first_iter(f);
        self.len.depth_first_iter(f);

        f(Order::Leave, RefNode::TypeArrayType(self));
    }
}

impl ParseTreeIter for SliceType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeSliceType(self));

        self.element_type.depth_first_iter(f);

        f(Order::Leave, RefNode::TypeSliceType(self));
    }
}

impl ParseTreeIter for FunctionType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeFunctionType(self));

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        self.parameters.depth_first_iter(f);

        if let Some(ret_ty) = &self.return_type {
            ret_ty.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::TypeFunctionType(self));
    }
}

impl ParseTreeIter for Lifetime {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeLifetime(self));
        f(Order::Leave, RefNode::TypeLifetime(self));
    }
}

impl ParseTreeIter for ReferenceType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeReferenceType(self));

        let _ = self.mutability;
        let _ = self.exclusivity;

        if let Some(lt) = &self.lifetime {
            lt.depth_first_iter(f);
        }

        self.to.depth_first_iter(f);

        f(Order::Leave, RefNode::TypeReferenceType(self));
    }
}

impl ParseTreeIter for OpaqueType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeOpaqueType(&self.name));

        let _ = self.name;

        f(Order::Leave, RefNode::TypeOpaqueType(&self.name));
    }
}

impl ParseTreeIter for LatentType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeLatentType(&self.body));

        self.body.depth_first_iter(f);

        f(Order::Leave, RefNode::TypeLatentType(&self.body));
    }
}

impl ParseTreeIter for TypeParentheses {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::TypeParentheses(&self.inner));

        self.inner.depth_first_iter(f);

        f(Order::Leave, RefNode::TypeParentheses(&self.inner));
    }
}

impl ParseTreeIter for Type {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        match self {
            Type::SyntaxError(ty) => ty.depth_first_iter(f),
            Type::Bool(ty) => ty.depth_first_iter(f),
            Type::UInt8(ty) => ty.depth_first_iter(f),
            Type::UInt16(ty) => ty.depth_first_iter(f),
            Type::UInt32(ty) => ty.depth_first_iter(f),
            Type::UInt64(ty) => ty.depth_first_iter(f),
            Type::UInt128(ty) => ty.depth_first_iter(f),
            Type::USize(ty) => ty.depth_first_iter(f),
            Type::Int8(ty) => ty.depth_first_iter(f),
            Type::Int16(ty) => ty.depth_first_iter(f),
            Type::Int32(ty) => ty.depth_first_iter(f),
            Type::Int64(ty) => ty.depth_first_iter(f),
            Type::Int128(ty) => ty.depth_first_iter(f),
            Type::Float32(ty) => ty.depth_first_iter(f),
            Type::Float64(ty) => ty.depth_first_iter(f),
            Type::InferType(ty) => ty.depth_first_iter(f),
            Type::TypePath(ty) => ty.depth_first_iter(f),
            Type::RefinementType(ty) => ty.depth_first_iter(f),
            Type::TupleType(ty) => ty.depth_first_iter(f),
            Type::ArrayType(ty) => ty.depth_first_iter(f),
            Type::SliceType(ty) => ty.depth_first_iter(f),
            Type::FunctionType(ty) => ty.depth_first_iter(f),
            Type::ReferenceType(ty) => ty.depth_first_iter(f),
            Type::OpaqueType(ty) => ty.depth_first_iter(f),
            Type::LatentType(ty) => ty.depth_first_iter(f),
            Type::Lifetime(ty) => ty.depth_first_iter(f),
            Type::Parentheses(ty) => ty.depth_first_iter(f),
        }
    }
}
