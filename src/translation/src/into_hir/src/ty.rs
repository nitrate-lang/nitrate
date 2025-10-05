use crate::{HirCtx, TryIntoHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::TypeSyntaxError {
    type Error = ();
    type Hir = ();

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
        Ok(Type::Inferred {
            id: ctx.next_type_infer_id(),
        })
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
        // TODO: Lower ast::RefinementType into hir::RefinementType
        Err(())
    }
}

impl TryIntoHir for ast::TupleType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        let mut elements = Vec::with_capacity(self.element_types.len());

        for ast_ty in self.element_types.into_iter() {
            let hir_ty = ast_ty.try_into_hir(ctx, log)?;
            elements.push(hir_ty.into_id(ctx.store_mut()));
        }

        Ok(Type::Tuple {
            elements: Box::new(elements),
        })
    }
}

impl TryIntoHir for ast::ArrayType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::ArrayType into hir::ArrayType
        Err(())
    }
}

impl TryIntoHir for ast::SliceType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::SliceType into hir::SliceType
        Err(())
    }
}

impl TryIntoHir for ast::FunctionType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::FunctionType into hir::FunctionType
        Err(())
    }
}

impl TryIntoHir for ast::ReferenceType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::ReferenceType into hir::ReferenceType
        Err(())
    }
}

impl TryIntoHir for ast::OpaqueType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::OpaqueType into hir::OpaqueType
        Err(())
    }
}

impl TryIntoHir for ast::LatentType {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::LatentType into hir::LatentType
        Err(())
    }
}

impl TryIntoHir for ast::Lifetime {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::Lifetime into hir::Lifetime
        Err(())
    }
}

impl TryIntoHir for ast::TypeParentheses {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::TypeParentheses into hir::TypeParentheses
        Err(())
    }
}

impl TryIntoHir for ast::Type {
    type Error = ();
    type Hir = Type;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Lower ast::TypeParentheses into hir::TypeParentheses
        Err(())
    }
}
