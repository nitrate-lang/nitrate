use inkwell::values::PointerValue;

use crate::rvalue::{CodegenCtx, CodegenError};
use nitrate_nstring::NString;
use nitrate_hir::prelude as hir;

fn gen_place_field_access<'ctx>(
    _ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    _struct_value: &hir::Value,
    _field_name: &NString,
) -> Result<PointerValue<'ctx>, CodegenError> {
    // TODO: implement field access codegen
    unimplemented!()
}

fn gen_place_index_access<'ctx>(
    _ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    _collection: &hir::Value,
    _index: &hir::Value,
) -> Result<PointerValue<'ctx>, CodegenError> {
    // TODO: implement index access codegen
    unimplemented!()
}

fn gen_place_deref<'ctx>(
    _ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    _place: &hir::Value,
) -> Result<PointerValue<'ctx>, CodegenError> {
    // TODO: implement dereference codegen
    unimplemented!()
}

fn gen_place_symbol<'ctx>(
    _ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    _symbol_name: &NString,
) -> Result<PointerValue<'ctx>, CodegenError> {
    // TODO: implement symbol codegen
    unimplemented!()
}

pub(crate) fn gen_place<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    hir_value: &hir::Value,
) -> Result<PointerValue<'ctx>, CodegenError> {
    match hir_value {
        hir::Value::Unit
        | hir::Value::Bool(_)
        | hir::Value::I8(_)
        | hir::Value::I16(_)
        | hir::Value::I32(_)
        | hir::Value::I64(_)
        | hir::Value::I128(_)
        | hir::Value::U8(_)
        | hir::Value::U16(_)
        | hir::Value::U32(_)
        | hir::Value::U64(_)
        | hir::Value::U128(_)
        | hir::Value::F32(_)
        | hir::Value::F64(_)
        | hir::Value::USize32(_)
        | hir::Value::USize64(_)
        | hir::Value::StringLit(_)
        | hir::Value::BStringLit(_)
        | hir::Value::InferredInteger(_)
        | hir::Value::InferredFloat(_)
        | hir::Value::StructObject { .. }
        | hir::Value::EnumVariant { .. }
        | hir::Value::Binary { .. }
        | hir::Value::Unary { .. }
        | hir::Value::Assign { .. }
        | hir::Value::Cast { .. }
        | hir::Value::Borrow { .. }
        | hir::Value::List { .. }
        | hir::Value::Tuple { .. }
        | hir::Value::If { .. }
        | hir::Value::While { .. }
        | hir::Value::Loop { .. }
        | hir::Value::Break { .. }
        | hir::Value::Continue { .. }
        | hir::Value::Return { .. }
        | hir::Value::Block { .. }
        | hir::Value::Closure { .. }
        | hir::Value::Call { .. }
        | hir::Value::MethodCall { .. } => Err(CodegenError::InvalidPlaceValue),

        hir::Value::FieldAccess { expr, field_name } => {
            let expr = ctx.store[expr].borrow();
            gen_place_field_access(ctx, &expr, field_name)
        }

        hir::Value::IndexAccess { collection, index } => {
            let collection = ctx.store[collection].borrow();
            let index = ctx.store[index].borrow();
            gen_place_index_access(ctx, &collection, &index)
        }

        hir::Value::Deref { place } => {
            let place = ctx.store[place].borrow();
            gen_place_deref(ctx, &place)
        }

        hir::Value::Symbol { path } => gen_place_symbol(ctx, path),
    }
}
