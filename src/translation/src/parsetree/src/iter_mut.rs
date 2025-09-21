use nitrate_tokenize::Integer;
use ordered_float::NotNan;

use crate::{
    expr::{
        Await, BinExpr, Block, Break, Call, Cast, Closure, Continue, DoWhileLoop, Expr, ForEach,
        If, IndexAccess, List, Object, Path, Return, Switch, UnaryExpr, WhileLoop,
    },
    item::{Enum, Impl, Import, Module, NamedFunction, Struct, Trait, TypeAlias, Variable},
    tag::{OpaqueTypeNameId, StringLiteralId},
    ty::{
        ArrayType, FunctionType, Lifetime, ReferenceType, RefinementType, SliceType, TupleType,
        Type,
    },
};

pub trait ParseTreeIterMut {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut));
}

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
