use crate::{HirCtx, TryIntoHir, diagnosis::HirErr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_evaluate::HirEvalCtx;
use nitrate_parsetree::kind::{self as ast};
use std::ops::Deref;

impl TryIntoHir for ast::TypeSyntaxError {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Err(())
    }
}

impl TryIntoHir for ast::Bool {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::Bool)
    }
}

impl TryIntoHir for ast::UInt8 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U8)
    }
}

impl TryIntoHir for ast::UInt16 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U16)
    }
}

impl TryIntoHir for ast::UInt32 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U32)
    }
}

impl TryIntoHir for ast::UInt64 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U64)
    }
}

impl TryIntoHir for ast::UInt128 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U128)
    }
}

impl TryIntoHir for ast::Int8 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I8)
    }
}

impl TryIntoHir for ast::Int16 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I16)
    }
}

impl TryIntoHir for ast::Int32 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I32)
    }
}

impl TryIntoHir for ast::Int64 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I64)
    }
}

impl TryIntoHir for ast::Int128 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I128)
    }
}

impl TryIntoHir for ast::Float8 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::F8)
    }
}

impl TryIntoHir for ast::Float16 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::F16)
    }
}

impl TryIntoHir for ast::Float32 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::F32)
    }
}

impl TryIntoHir for ast::Float64 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::F64)
    }
}

impl TryIntoHir for ast::Float128 {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::F128)
    }
}

impl TryIntoHir for ast::InferType {
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(ctx.create_inference_placeholder())
    }
}

impl TryIntoHir for ast::TypePath {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::TypePath into hir::TypePath
        log.report(&HirErr::UnimplementedFeature("type path".into()));
        Err(())
    }
}

impl TryIntoHir for ast::RefinementType {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::RefinementType into hir::RefinementType
        log.report(&HirErr::UnimplementedFeature("refinement type".into()));
        Err(())
    }
}

impl TryIntoHir for ast::TupleType {
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        if self.element_types.is_empty() {
            return Ok(Type::Unit);
        }

        let mut elements = Vec::with_capacity(self.element_types.len());
        for ast_elem_ty in self.element_types.into_iter() {
            let hir_elem_ty = ast_elem_ty.try_into_hir(ctx, log)?;
            elements.push(hir_elem_ty.into_id(ctx.store()));
        }

        Ok(Type::Tuple {
            element_types: elements.into_id(ctx.store()),
        })
    }
}

impl TryIntoHir for ast::ArrayType {
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let element_type = self
            .element_type
            .try_into_hir(ctx, log)?
            .into_id(ctx.store());

        let hir_length = Value::Cast {
            expr: self.len.try_into_hir(ctx, log)?.into_id(ctx.store()),
            to: Type::USize.into_id(ctx.store()),
        };

        let mut eval = HirEvalCtx::new(ctx.store(), log, ctx.ptr_size());
        let len = match eval.evaluate_to_literal(&hir_length) {
            Ok(Lit::USize32(val)) => {
                if ctx.ptr_size() != PtrSize::U32 {
                    log.report(&HirErr::FoundUSize32InNon32BitTarget);
                    return Err(());
                }

                val as u64
            }

            Ok(Lit::USize64(val)) => {
                if ctx.ptr_size() != PtrSize::U64 {
                    log.report(&HirErr::FoundUSize64InNon64BitTarget);
                    return Err(());
                }

                val
            }

            Ok(_) => {
                log.report(&HirErr::ArrayLengthExpectedUSize);
                return Err(());
            }

            Err(_) => {
                log.report(&HirErr::ArrayTypeLengthEvalError);
                return Err(());
            }
        };

        Ok(Type::Array { element_type, len })
    }
}

impl TryIntoHir for ast::SliceType {
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let element_type = self
            .element_type
            .try_into_hir(ctx, log)?
            .into_id(ctx.store());

        Ok(Type::Slice { element_type })
    }
}

impl TryIntoHir for ast::FunctionType {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::FunctionType into hir::FunctionType
        Err(())
    }
}

impl TryIntoHir for ast::ReferenceType {
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let to = self.to.try_into_hir(ctx, log)?.into_id(ctx.store());

        let lifetime = match self.lifetime {
            None => Lifetime::Inferred,
            Some(ast::Lifetime { name }) => match name.deref() {
                "static" => Lifetime::Static,
                "gc" => Lifetime::Gc,
                "thread" => Lifetime::ThreadLocal,
                "task" => Lifetime::TaskLocal,
                "_" => Lifetime::Inferred,
                name => Lifetime::Stack {
                    id: EntityName(name.into()),
                },
            },
        };

        let mutable = match self.mutability {
            Some(ast::Mutability::Mut) => true,
            Some(ast::Mutability::Const) | None => false,
        };

        let exclusive = match self.exclusivity {
            Some(ast::Exclusivity::Iso) => true,
            Some(ast::Exclusivity::Poly) => false,
            None => mutable,
        };

        Ok(Type::Reference {
            lifetime,
            exclusive,
            mutable,
            to,
        })
    }
}

impl TryIntoHir for ast::OpaqueType {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::Opaque {
            name: self.name.deref().into(),
        })
    }
}

impl TryIntoHir for ast::LatentType {
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let block = self.body.try_into_hir(ctx, log)?.into_id(ctx.store());

        let mut eval = HirEvalCtx::new(ctx.store(), log, ctx.ptr_size());
        let hir_type = match eval.evaluate_into_type(&Value::Block { block }) {
            Ok(ty) => ty,

            Err(_) => {
                log.report(&HirErr::LatentTypeEvaluationError);
                return Err(());
            }
        };

        Ok(hir_type)
    }
}

impl TryIntoHir for ast::Lifetime {
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Lifetime into hir::Lifetime
        log.report(&HirErr::UnimplementedFeature("ast::Lifetime".into()));
        Err(())
    }
}

impl TryIntoHir for ast::TypeParentheses {
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        self.inner.try_into_hir(ctx, log)
    }
}

impl TryIntoHir for ast::Type {
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        match self {
            ast::Type::SyntaxError(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Bool(ty) => ty.try_into_hir(ctx, log),
            ast::Type::UInt8(ty) => ty.try_into_hir(ctx, log),
            ast::Type::UInt16(ty) => ty.try_into_hir(ctx, log),
            ast::Type::UInt32(ty) => ty.try_into_hir(ctx, log),
            ast::Type::UInt64(ty) => ty.try_into_hir(ctx, log),
            ast::Type::UInt128(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Int8(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Int16(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Int32(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Int64(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Int128(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Float8(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Float16(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Float32(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Float64(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Float128(ty) => ty.try_into_hir(ctx, log),
            ast::Type::InferType(ty) => ty.try_into_hir(ctx, log),
            ast::Type::TypePath(ty) => ty.try_into_hir(ctx, log),
            ast::Type::RefinementType(ty) => ty.try_into_hir(ctx, log),
            ast::Type::TupleType(ty) => ty.try_into_hir(ctx, log),
            ast::Type::ArrayType(ty) => ty.try_into_hir(ctx, log),
            ast::Type::SliceType(ty) => ty.try_into_hir(ctx, log),
            ast::Type::FunctionType(ty) => ty.try_into_hir(ctx, log),
            ast::Type::ReferenceType(ty) => ty.try_into_hir(ctx, log),
            ast::Type::OpaqueType(ty) => ty.try_into_hir(ctx, log),
            ast::Type::LatentType(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Lifetime(ty) => ty.try_into_hir(ctx, log),
            ast::Type::Parentheses(ty) => ty.try_into_hir(ctx, log),
        }
    }
}
