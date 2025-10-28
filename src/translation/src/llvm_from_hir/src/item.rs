use crate::codegen::CodeGen;
use inkwell::module::Module;
use nitrate_hir::{Store, SymbolTab, prelude as hir};
use nitrate_llvm::LLVMContext;

impl<'ctx> CodeGen<'ctx> for hir::GlobalVariableAttribute {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::GlobalVariable {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::LocalVariableAttribute {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::LocalVariable {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::ParameterAttribute {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Parameter {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Function {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Trait {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::ModuleAttribute {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Module {
    type Output = Module<'ctx>;

    fn generate(
        self,
        ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        let module = ctx.create_module(self.name.unwrap_or_default().to_string().as_str());
        let bb = ctx.create_builder();

        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::TypeAliasDef {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::StructDef {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::EnumDef {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::TypeDefinition {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::SymbolId {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Item {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}
