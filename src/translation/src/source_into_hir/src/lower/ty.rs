use crate::{context::Ast2HirCtx, diagnosis::HirErr, lower::lower::Ast2Hir};
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_evaluate::HirEvalCtx;
use nitrate_source::ast::{self as ast};
use std::{collections::BTreeSet, ops::Deref};

impl Ast2Hir for ast::TypeSyntaxError {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Err(())
    }
}

impl Ast2Hir for ast::Bool {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::Bool)
    }
}

impl Ast2Hir for ast::UInt8 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U8)
    }
}

impl Ast2Hir for ast::UInt16 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U16)
    }
}

impl Ast2Hir for ast::UInt32 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U32)
    }
}

impl Ast2Hir for ast::UInt64 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U64)
    }
}

impl Ast2Hir for ast::UInt128 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::U128)
    }
}

impl Ast2Hir for ast::USize {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::USize)
    }
}

impl Ast2Hir for ast::Int8 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I8)
    }
}

impl Ast2Hir for ast::Int16 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I16)
    }
}

impl Ast2Hir for ast::Int32 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I32)
    }
}

impl Ast2Hir for ast::Int64 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I64)
    }
}

impl Ast2Hir for ast::Int128 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::I128)
    }
}

impl Ast2Hir for ast::Float32 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::F32)
    }
}

impl Ast2Hir for ast::Float64 {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::F64)
    }
}

impl Ast2Hir for ast::InferType {
    type Hir = Type;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(ctx.create_inference_placeholder())
    }
}

impl Ast2Hir for ast::TypePath {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        if self.segments.iter().any(|seg| seg.type_arguments.is_some()) {
            // TODO: Support generic type arguments
            log.report(&HirErr::UnimplementedFeature(
                "generic type arguments in type paths".into(),
            ));
        }

        if let Some(resolved_path) = self.resolved_path {
            let path = IString::from(resolved_path);
            return Ok(Type::Symbol { path });
        }

        log.report(&HirErr::UnresolvedTypePath);
        Err(())
    }
}

impl Ast2Hir for ast::RefinementType {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::RefinementType
        log.report(&HirErr::UnimplementedFeature("refinement type".into()));
        Err(())
    }
}

impl Ast2Hir for ast::TupleType {
    type Hir = Type;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        if self.element_types.is_empty() {
            return Ok(Type::Unit);
        }

        let mut elements = Vec::with_capacity(self.element_types.len());
        for ast_elem_ty in self.element_types.into_iter() {
            let hir_elem_ty = ast_elem_ty.ast2hir(ctx, log)?.into_id(&ctx.store);
            elements.push(hir_elem_ty);
        }

        Ok(Type::Tuple {
            element_types: elements.into(),
        })
    }
}

impl Ast2Hir for ast::ArrayType {
    type Hir = Type;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let element_type = self.element_type.ast2hir(ctx, log)?.into_id(&ctx.store);

        let hir_length = Value::Cast {
            expr: self.len.ast2hir(ctx, log)?.into_id(&ctx.store),
            to: Type::USize.into_id(&ctx.store),
        };

        let mut eval = HirEvalCtx::new(&ctx.store, log, ctx.ptr_size);
        let len = match eval.evaluate_to_literal(&hir_length) {
            Ok(Lit::USize32(val)) => {
                if ctx.ptr_size != PtrSize::U32 {
                    log.report(&HirErr::FoundUSize32InNon32BitTarget);
                    return Err(());
                }

                val as u64
            }

            Ok(Lit::USize64(val)) => {
                if ctx.ptr_size != PtrSize::U64 {
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

impl Ast2Hir for ast::SliceType {
    type Hir = Type;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let element_type = self.element_type.ast2hir(ctx, log)?.into_id(&ctx.store);

        Ok(Type::Slice { element_type })
    }
}

impl Ast2Hir for ast::FunctionType {
    type Hir = Type;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let ast_attributes = self.attributes.unwrap_or_default();

        let attributes = BTreeSet::new();
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedFunctionAttribute);
        }

        let mut parameters = Vec::with_capacity(self.parameters.len());
        for param in self.parameters {
            // let attributes = BTreeSet::new();
            if let Some(ast_attributes) = &param.attributes {
                for _attr in ast_attributes {
                    log.report(&HirErr::UnrecognizedFunctionParameterAttribute);
                }
            }

            let name = IString::from(param.name.deref());
            let ty = param.ty.ast2hir(ctx, log)?.into_id(&ctx.store);

            parameters.push((name, ty));
        }

        let return_type = match self.return_type {
            Some(ret_ty) => ret_ty.ast2hir(ctx, log)?,
            None => Type::Unit,
        };

        let function_type = FunctionType {
            attributes,
            params: parameters,
            return_type: return_type.into_id(&ctx.store),
        };

        Ok(Type::Function {
            function_type: function_type.into_id(&ctx.store),
        })
    }
}

impl Ast2Hir for ast::ReferenceType {
    type Hir = Type;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let to = self.to.ast2hir(ctx, log)?.into_id(&ctx.store);

        let lifetime = match self.lifetime {
            None => Lifetime::Inferred,
            Some(ast::Lifetime { name }) => match name.deref() {
                "static" => Lifetime::Static,
                "gc" => Lifetime::Gc,
                "thread" => Lifetime::ThreadLocal,
                "task" => Lifetime::TaskLocal,
                "_" => Lifetime::Inferred,
                _ => {
                    log.report(&HirErr::UnrecognizedLifetime);
                    return Err(());
                }
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

impl Ast2Hir for ast::OpaqueType {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Type::Opaque {
            name: self.name.deref().into(),
        })
    }
}

impl Ast2Hir for ast::LatentType {
    type Hir = Type;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let block = self.body.ast2hir(ctx, log)?.into_id(&ctx.store);

        let mut eval = HirEvalCtx::new(&ctx.store, log, ctx.ptr_size);
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

impl Ast2Hir for ast::Lifetime {
    type Hir = Type;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Lifetime
        log.report(&HirErr::UnimplementedFeature("ast::Lifetime".into()));
        Err(())
    }
}

impl Ast2Hir for ast::TypeParentheses {
    type Hir = Type;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        self.inner.ast2hir(ctx, log)
    }
}

impl Ast2Hir for ast::Type {
    type Hir = Type;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        match self {
            ast::Type::SyntaxError(ty) => ty.ast2hir(ctx, log),
            ast::Type::Bool(ty) => ty.ast2hir(ctx, log),
            ast::Type::UInt8(ty) => ty.ast2hir(ctx, log),
            ast::Type::UInt16(ty) => ty.ast2hir(ctx, log),
            ast::Type::UInt32(ty) => ty.ast2hir(ctx, log),
            ast::Type::UInt64(ty) => ty.ast2hir(ctx, log),
            ast::Type::UInt128(ty) => ty.ast2hir(ctx, log),
            ast::Type::USize(ty) => ty.ast2hir(ctx, log),
            ast::Type::Int8(ty) => ty.ast2hir(ctx, log),
            ast::Type::Int16(ty) => ty.ast2hir(ctx, log),
            ast::Type::Int32(ty) => ty.ast2hir(ctx, log),
            ast::Type::Int64(ty) => ty.ast2hir(ctx, log),
            ast::Type::Int128(ty) => ty.ast2hir(ctx, log),
            ast::Type::Float32(ty) => ty.ast2hir(ctx, log),
            ast::Type::Float64(ty) => ty.ast2hir(ctx, log),
            ast::Type::InferType(ty) => ty.ast2hir(ctx, log),
            ast::Type::TypePath(ty) => ty.ast2hir(ctx, log),
            ast::Type::RefinementType(ty) => ty.ast2hir(ctx, log),
            ast::Type::TupleType(ty) => ty.ast2hir(ctx, log),
            ast::Type::ArrayType(ty) => ty.ast2hir(ctx, log),
            ast::Type::SliceType(ty) => ty.ast2hir(ctx, log),
            ast::Type::FunctionType(ty) => ty.ast2hir(ctx, log),
            ast::Type::ReferenceType(ty) => ty.ast2hir(ctx, log),
            ast::Type::OpaqueType(ty) => ty.ast2hir(ctx, log),
            ast::Type::LatentType(ty) => ty.ast2hir(ctx, log),
            ast::Type::Lifetime(ty) => ty.ast2hir(ctx, log),
            ast::Type::Parentheses(ty) => ty.ast2hir(ctx, log),
        }
    }
}
