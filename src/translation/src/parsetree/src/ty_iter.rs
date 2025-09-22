use crate::{
    Order, ParseTreeIter, RefNode,
    kind::{
        ArrayType, Bool, Float8, Float16, Float32, Float64, Float128, FunctionType,
        FunctionTypeParameter, Int8, Int16, Int32, Int64, Int128, LatentType, Lifetime, OpaqueType,
        ReferenceType, RefinementType, SliceType, TupleType, Type, TypeName, TypeParentheses,
        TypeSyntaxError, UInt8, UInt16, UInt32, UInt64, UInt128,
    },
    ty::{InferType, UnitType},
};

impl ParseTreeIter for TypeSyntaxError {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeSyntaxError);
        f(Order::Post, RefNode::TypeSyntaxError);
    }
}

impl ParseTreeIter for Bool {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeBool);
        f(Order::Post, RefNode::TypeBool);
    }
}

impl ParseTreeIter for UInt8 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeUInt8);
        f(Order::Post, RefNode::TypeUInt8);
    }
}

impl ParseTreeIter for UInt16 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeUInt16);
        f(Order::Post, RefNode::TypeUInt16);
    }
}

impl ParseTreeIter for UInt32 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeUInt32);
        f(Order::Post, RefNode::TypeUInt32);
    }
}

impl ParseTreeIter for UInt64 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeUInt64);
        f(Order::Post, RefNode::TypeUInt64);
    }
}

impl ParseTreeIter for UInt128 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeUInt128);
        f(Order::Post, RefNode::TypeUInt128);
    }
}

impl ParseTreeIter for Int8 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeInt8);
        f(Order::Post, RefNode::TypeInt8);
    }
}

impl ParseTreeIter for Int16 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeInt16);
        f(Order::Post, RefNode::TypeInt16);
    }
}

impl ParseTreeIter for Int32 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeInt32);
        f(Order::Post, RefNode::TypeInt32);
    }
}

impl ParseTreeIter for Int64 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeInt64);
        f(Order::Post, RefNode::TypeInt64);
    }
}

impl ParseTreeIter for Int128 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeInt128);
        f(Order::Post, RefNode::TypeInt128);
    }
}

impl ParseTreeIter for Float8 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeFloat8);
        f(Order::Post, RefNode::TypeFloat8);
    }
}

impl ParseTreeIter for Float16 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeFloat16);
        f(Order::Post, RefNode::TypeFloat16);
    }
}

impl ParseTreeIter for Float32 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeFloat32);
        f(Order::Post, RefNode::TypeFloat32);
    }
}

impl ParseTreeIter for Float64 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeFloat64);
        f(Order::Post, RefNode::TypeFloat64);
    }
}

impl ParseTreeIter for Float128 {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeFloat128);
        f(Order::Post, RefNode::TypeFloat128);
    }
}

impl ParseTreeIter for UnitType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeUnitType);
        f(Order::Post, RefNode::TypeUnitType);
    }
}

impl ParseTreeIter for InferType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeInferType);
        f(Order::Post, RefNode::TypeInferType);
    }
}

impl ParseTreeIter for TypeName {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeTypeName(&self.name));
        f(Order::Post, RefNode::TypeTypeName(&self.name));
    }
}

impl ParseTreeIter for RefinementType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeRefinementType(self));

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

        f(Order::Post, RefNode::TypeRefinementType(self));
    }
}

impl ParseTreeIter for TupleType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeTupleType(self));

        for elem in &self.element_types {
            elem.depth_first_iter(f);
        }

        f(Order::Post, RefNode::TypeTupleType(self));
    }
}

impl ParseTreeIter for ArrayType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeArrayType(self));

        self.element_type.depth_first_iter(f);
        self.len.depth_first_iter(f);

        f(Order::Post, RefNode::TypeArrayType(self));
    }
}

impl ParseTreeIter for SliceType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeSliceType(self));

        self.element_type.depth_first_iter(f);

        f(Order::Post, RefNode::TypeSliceType(self));
    }
}

impl ParseTreeIter for FunctionTypeParameter {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeFunctionTypeParameter(self));

        let _ = self.name;

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        self.param_type.depth_first_iter(f);

        if let Some(default) = &self.default {
            default.depth_first_iter(f);
        }

        f(Order::Post, RefNode::TypeFunctionTypeParameter(self));
    }
}

impl ParseTreeIter for FunctionType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeFunctionType(self));

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        for param in &self.parameters {
            param.depth_first_iter(f);
        }

        if let Some(ret_ty) = &self.return_type {
            ret_ty.depth_first_iter(f);
        }

        f(Order::Post, RefNode::TypeFunctionType(self));
    }
}

impl ParseTreeIter for Lifetime {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeLifetime(self));
        f(Order::Post, RefNode::TypeLifetime(self));
    }
}

impl ParseTreeIter for ReferenceType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeReferenceType(self));

        let _ = self.mutability;
        let _ = self.exclusive;

        if let Some(lt) = &self.lifetime {
            lt.depth_first_iter(f);
        }

        self.to.depth_first_iter(f);

        f(Order::Post, RefNode::TypeReferenceType(self));
    }
}

impl ParseTreeIter for OpaqueType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeOpaqueType(&self.name));

        let _ = self.name;

        f(Order::Post, RefNode::TypeOpaqueType(&self.name));
    }
}

impl ParseTreeIter for LatentType {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeLatentType(&self.body));

        self.body.depth_first_iter(f);

        f(Order::Post, RefNode::TypeLatentType(&self.body));
    }
}

impl ParseTreeIter for TypeParentheses {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::TypeParentheses(&self.inner));

        self.inner.depth_first_iter(f);

        f(Order::Post, RefNode::TypeParentheses(&self.inner));
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
            Type::Int8(ty) => ty.depth_first_iter(f),
            Type::Int16(ty) => ty.depth_first_iter(f),
            Type::Int32(ty) => ty.depth_first_iter(f),
            Type::Int64(ty) => ty.depth_first_iter(f),
            Type::Int128(ty) => ty.depth_first_iter(f),
            Type::Float8(ty) => ty.depth_first_iter(f),
            Type::Float16(ty) => ty.depth_first_iter(f),
            Type::Float32(ty) => ty.depth_first_iter(f),
            Type::Float64(ty) => ty.depth_first_iter(f),
            Type::Float128(ty) => ty.depth_first_iter(f),
            Type::UnitType(ty) => ty.depth_first_iter(f),
            Type::InferType(ty) => ty.depth_first_iter(f),
            Type::TypeName(ty) => ty.depth_first_iter(f),
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
