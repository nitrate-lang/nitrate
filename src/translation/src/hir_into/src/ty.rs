use crate::{HirCtx, TryIntoHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;
use std::ops::Deref;

fn create_inference_variable(ctx: &mut HirCtx) -> Type {
    Type::Inferred {
        id: ctx.next_type_infer_id(),
    }
}

impl TryIntoHir for ast::TypeSyntaxError {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Err(())
    }
}

impl TryIntoHir for ast::Bool {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::Bool)
    }
}

impl TryIntoHir for ast::UInt8 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::U8)
    }
}

impl TryIntoHir for ast::UInt16 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::U16)
    }
}

impl TryIntoHir for ast::UInt32 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::U32)
    }
}

impl TryIntoHir for ast::UInt64 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::U64)
    }
}

impl TryIntoHir for ast::UInt128 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::U128)
    }
}

impl TryIntoHir for ast::Int8 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::I8)
    }
}

impl TryIntoHir for ast::Int16 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::I16)
    }
}

impl TryIntoHir for ast::Int32 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::I32)
    }
}

impl TryIntoHir for ast::Int64 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::I64)
    }
}

impl TryIntoHir for ast::Int128 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::I128)
    }
}

impl TryIntoHir for ast::Float8 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::F8)
    }
}

impl TryIntoHir for ast::Float16 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::F16)
    }
}

impl TryIntoHir for ast::Float32 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::F32)
    }
}

impl TryIntoHir for ast::Float64 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::F64)
    }
}

impl TryIntoHir for ast::Float128 {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::F128)
    }
}

impl TryIntoHir for ast::InferType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(create_inference_variable(ctx))
    }
}

impl TryIntoHir for ast::TypePath {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::TypePath into hir::TypePath
        Err(())
    }
}

impl TryIntoHir for ast::RefinementType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        let base = match self.width {
            None => self.basis_type.try_into_hir(ctx, log)?.into_id(ctx.store()),

            Some(expr) => {
                let base = self.basis_type.try_into_hir(ctx, log)?.into_id(ctx.store());

                let hir_expr = expr.try_into_hir(ctx, log)?;
                let literal = Literal::try_from(hir_expr).map_err(|_| ())?;

                let bits = match literal {
                    Literal::U8(x) => x,
                    Literal::U16(x) => u8::try_from(x).map_err(|_| ())?,
                    Literal::U32(x) => u8::try_from(x).map_err(|_| ())?,
                    Literal::U64(x) => u8::try_from(x).map_err(|_| ())?,
                    Literal::U128(x) => u8::try_from(x).map_err(|_| ())?,
                    Literal::I8(x) => u8::try_from(x).map_err(|_| ())?,
                    Literal::I16(x) => u8::try_from(x).map_err(|_| ())?,
                    Literal::I32(x) => u8::try_from(x).map_err(|_| ())?,
                    Literal::I64(x) => u8::try_from(x).map_err(|_| ())?,
                    Literal::I128(x) => u8::try_from(x).map_err(|_| ())?,
                    _ => return Err(()),
                };

                Type::Bitfield { base, bits }.into_id(ctx.store())
            }
        };

        let min = match self.minimum {
            None => {
                // TODO: Do type inference to find the minimum value for the base type
                todo!("Implement type inference for refinement type minimum deduction");
            }

            Some(expr) => {
                let hir_expr = expr.try_into_hir(ctx, log)?;
                let literal = Literal::try_from(hir_expr).map_err(|_| ())?;
                literal.into_id(ctx.store())
            }
        };

        let max = match self.maximum {
            None => {
                // TODO: Do type inference to find the maximum value for the base type
                todo!("Implement type inference for refinement type maximum deduction");
            }

            Some(expr) => {
                let hir_expr = expr.try_into_hir(ctx, log)?;
                let literal = Literal::try_from(hir_expr).map_err(|_| ())?;
                literal.into_id(ctx.store())
            }
        };

        Ok(Type::Refine { base, min, max })
    }
}

impl TryIntoHir for ast::TupleType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        if self.element_types.is_empty() {
            return Ok(Type::Unit);
        }

        let mut elements = Vec::with_capacity(self.element_types.len());

        for ast_ty in self.element_types.into_iter() {
            let hir_ty = ast_ty.try_into_hir(ctx, log)?;
            elements.push(hir_ty.into_id(ctx.store()));
        }

        Ok(Type::Tuple {
            elements: elements.into_id(ctx.store()),
        })
    }
}

impl TryIntoHir for ast::ArrayType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        let hir_element_type = self.element_type.try_into_hir(ctx, log)?;

        // TODO: Evaluate the length expression to a constant u64 value
        let hir_length = 0;

        Ok(Type::Array {
            element_type: hir_element_type.into_id(ctx.store()),
            len: hir_length,
        })
    }
}

impl TryIntoHir for ast::SliceType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        let hir_element_type = self.element_type.try_into_hir(ctx, log)?;

        Ok(Type::Slice {
            element_type: hir_element_type.into_id(ctx.store()),
        })
    }
}

impl TryIntoHir for ast::FunctionType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        let return_type = match self.return_type {
            Some(ty) => ty.try_into_hir(ctx, log)?.into_id(ctx.store()),
            None => Type::Unit.into_id(ctx.store()),
        };

        // let mut parameter_types = Vec::with_capacity(self.parameters.len());

        for param in self.parameters.into_iter() {
            if !param.attributes.unwrap_or_default().elements.is_empty() {
                // TODO: Handle parameter attributes
                return Err(());
            }

            if param.mutability.is_some() {
                // TODO: Print error
                return Err(());
            }

            let param_type = match param.param_type {
                Some(ty) => ty.try_into_hir(ctx, log)?.into_id(ctx.store()),
                None => create_inference_variable(ctx).into_id(ctx.store()),
            };

            let default_value = match param.default_value {
                Some(expr) => Some(expr.try_into_hir(ctx, log)?.into_id(ctx.store())),
                None => None,
            };

            let _ = param.name;
        }

        // TODO: Lower ast::FunctionType into hir::FunctionType
        Err(())
    }
}

impl TryIntoHir for ast::ReferenceType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
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
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        Ok(Type::Opaque {
            name: self.name.deref().into(),
        })
    }
}

impl TryIntoHir for ast::LatentType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::LatentType into hir::LatentType
        todo!("Implement latent type evaluation");
    }
}

impl TryIntoHir for ast::Lifetime {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::Lifetime into hir::Lifetime
        Err(())
    }
}

impl TryIntoHir for ast::TypeParentheses {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        self.inner.try_into_hir(ctx, log)
    }
}

impl TryIntoHir for ast::Type {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
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
