use std::path::Path;

use nitrate_tokenize::Integer;
use ordered_float::NotNan;

use crate::{
    expr::{
        Await, BinExpr, Block, Break, Call, Cast, Closure, Continue, DoWhileLoop, Expr, ForEach,
        If, IndexAccess, List, Object, Return, Switch, UnaryExpr, WhileLoop,
    },
    item::{Enum, Impl, Import, Item, Module, NamedFunction, Struct, Trait, TypeAlias, Variable},
    tag::{OpaqueTypeNameId, StringLiteralId},
    ty::{
        ArrayType, FunctionType, Lifetime, ReferenceType, RefinementType, SliceType, TupleType,
        Type,
    },
};

pub enum RefNodeMut<'a> {
    ExprSyntaxError,
    ExprParentheses(&'a mut Expr),
    ExprBoolean(&'a mut bool),
    ExprInteger(&'a mut Integer),
    ExprFloat(&'a mut NotNan<f64>),
    ExprString(&'a mut StringLiteralId),
    ExprBString(&'a mut Vec<u8>),
    ExprUnit,
    ExprTypeInfo(&'a mut Type),
    ExprList(&'a mut List),
    ExprObject(&'a mut Object),
    ExprUnaryExpr(&'a mut UnaryExpr),
    ExprBinExpr(&'a mut BinExpr),
    ExprCast(&'a mut Cast),
    ExprBlock(&'a mut Block),
    ExprClosure(&'a mut Closure),
    ExprVariable(&'a mut Variable),
    ExprPath(&'a mut Path),
    ExprIndexAccess(&'a mut IndexAccess),
    ExprIf(&'a mut If),
    ExprWhile(&'a mut WhileLoop),
    ExprDoWhileLoop(&'a mut DoWhileLoop),
    ExprSwitch(&'a mut Switch),
    ExprBreak(&'a mut Break),
    ExprContinue(&'a mut Continue),
    ExprReturn(&'a mut Return),
    ExprFor(&'a mut ForEach),
    ExprAwait(&'a mut Await),
    ExprCall(&'a mut Call),

    TypeSyntaxError,
    TypeBool,
    TypeUInt8,
    TypeUInt16,
    TypeUInt32,
    TypeUInt64,
    TypeUInt128,
    TypeInt8,
    TypeInt16,
    TypeInt32,
    TypeInt64,
    TypeInt128,
    TypeFloat8,
    TypeFloat16,
    TypeFloat32,
    TypeFloat64,
    TypeFloat128,
    TypeUnitType,
    TypeInferType,
    TypeTypeName(&'a mut Path),
    TypeRefinementType(&'a mut RefinementType),
    TypeTupleType(&'a mut TupleType),
    TypeArrayType(&'a mut ArrayType),
    TypeSliceType(&'a mut SliceType),
    TypeFunctionType(&'a mut FunctionType),
    TypeReferenceType(&'a mut ReferenceType),
    TypeOpaqueType(&'a mut OpaqueTypeNameId),
    TypeLatentType(&'a mut Block),
    TypeLifetime(&'a mut Lifetime),
    TypeParentheses(&'a mut Type),

    ItemSyntaxError,
    ItemModule(&'a mut Module),
    ItemImport(&'a mut Import),
    ItemTypeAlias(&'a mut TypeAlias),
    ItemStruct(&'a mut Struct),
    ItemEnum(&'a mut Enum),
    ItemTrait(&'a mut Trait),
    ItemImpl(&'a mut Impl),
    ItemNamedFunction(&'a mut NamedFunction),
    ItemVariable(&'a mut Variable),
}

pub enum Order {
    Pre,
    Post,
}

pub fn expr_depth_first_iter_mut(node: &mut Expr, f: &mut dyn FnMut(Order, RefNodeMut)) {
    // f(Order::Pre, NodeMut::Expr(RefExpr::Expr(node)));

    // TODO: Recurse into expression children.

    // f(Order::Post, NodeMut::Expr(node));
}

pub fn type_depth_first_iter_mut(node: &mut Type, f: &mut dyn FnMut(Order, RefNodeMut)) {
    // f(Order::Pre, NodeMut::Type(node));

    // TODO: Recurse into type children.

    // f(Order::Post, NodeMut::Type(node));
}

pub fn item_depth_first_iter_mut(node: &mut Item, f: &mut dyn FnMut(Order, RefNodeMut)) {
    match node {
        Item::SyntaxError => {}

        Item::Module(module) => {
            f(Order::Pre, RefNodeMut::ItemModule(module));

            if let Some(attributes) = module.attributes.as_mut() {
                for expr in attributes {
                    expr_depth_first_iter_mut(expr, f);
                }
            }

            for item in &mut module.items {
                item_depth_first_iter_mut(item, f);
            }

            f(Order::Post, RefNodeMut::ItemModule(module));
        }

        Item::Import(import) => {
            if let Some(attributes) = import.attributes.as_mut() {
                for expr in attributes {
                    expr_depth_first_iter_mut(expr, f);
                }
            }
        }

        Item::TypeAlias(type_alias) => {
            if let Some(attributes) = type_alias.attributes.as_mut() {
                for expr in attributes {
                    expr_depth_first_iter_mut(expr, f);
                }
            }

            if let Some(ty_params) = type_alias.type_params.as_mut() {
                for param in ty_params {
                    if let Some(default) = param.default.as_mut() {
                        type_depth_first_iter_mut(default, f);
                    }
                }
            }

            if let Some(ty) = type_alias.aliased_type.as_mut() {
                type_depth_first_iter_mut(ty, f);
            }
        }

        Item::Struct(struct_) => {
            if let Some(attributes) = struct_.attributes.as_mut() {
                for expr in attributes {
                    expr_depth_first_iter_mut(expr, f);
                }
            }

            if let Some(ty_params) = struct_.type_params.as_mut() {
                for param in ty_params {
                    if let Some(default) = param.default.as_mut() {
                        type_depth_first_iter_mut(default, f);
                    }
                }
            }

            for field in &mut struct_.fields {
                if let Some(attributes) = field.attributes.as_mut() {
                    for expr in attributes {
                        expr_depth_first_iter_mut(expr, f);
                    }
                }

                type_depth_first_iter_mut(&mut field.field_type, f);

                if let Some(default) = field.default.as_mut() {
                    expr_depth_first_iter_mut(default, f);
                }
            }
        }

        _ => {}
    }
}
