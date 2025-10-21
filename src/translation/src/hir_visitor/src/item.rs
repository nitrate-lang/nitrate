use interned_string::IString;
use nitrate_hir::prelude::*;
use std::collections::BTreeSet;

pub trait HirItemVisitor<T> {
    fn visit_module(
        &mut self,
        vis: Visibility,
        name: Option<&IString>,
        attrs: &BTreeSet<ModuleAttribute>,
        items: &[Item],
    ) -> T;

    fn visit_global_variable(
        &mut self,
        vis: Visibility,
        attrs: &BTreeSet<GlobalVariableAttribute>,
        is_mutable: bool,
        name: &IString,
        ty: &Type,
        init: Option<&Value>,
    ) -> T;

    fn visit_function(
        &mut self,
        vis: Visibility,
        attrs: &BTreeSet<FunctionAttribute>,
        name: &IString,
        params: &[ParameterId],
        ret: &Type,
        body: Option<&Block>,
    ) -> T;

    fn visit_type_alias(&mut self, vis: Visibility, name: &IString, ty: &Type) -> T;

    fn visit_struct_def(
        &mut self,
        vis: Visibility,
        name: &IString,
        fields_extra: &[(Visibility, Option<ValueId>)],
        struct_ty: &StructTypeId,
    ) -> T;

    fn visit_enum_def(
        &mut self,
        vis: Visibility,
        name: &IString,
        variants: &[Option<ValueId>],
        enum_ty: &EnumTypeId,
    ) -> T;

    fn visit_item(&mut self, item: &Item, store: &Store) -> T {
        match item {
            Item::Module(module_id) => {
                let m = store[module_id].borrow();
                self.visit_module(m.visibility, m.name.as_ref(), &m.attributes, &m.items)
            }

            Item::GlobalVariable(global_variable_id) => {
                let gv = store[global_variable_id].borrow();
                match &gv.init {
                    Some(init) => self.visit_global_variable(
                        gv.visibility,
                        &gv.attributes,
                        gv.is_mutable,
                        &gv.name,
                        &store[&gv.ty],
                        Some(&store[init].borrow()),
                    ),

                    None => self.visit_global_variable(
                        gv.visibility,
                        &gv.attributes,
                        gv.is_mutable,
                        &gv.name,
                        &store[&gv.ty],
                        None,
                    ),
                }
            }

            Item::Function(function_id) => {
                let f = store[function_id].borrow();
                match &f.body {
                    Some(body_id) => {
                        let body = &store[body_id].borrow();
                        self.visit_function(
                            f.visibility,
                            &f.attributes,
                            &f.name,
                            &f.params,
                            &store[&f.return_type],
                            Some(body),
                        )
                    }
                    None => self.visit_function(
                        f.visibility,
                        &f.attributes,
                        &f.name,
                        &f.params,
                        &store[&f.return_type],
                        None,
                    ),
                }
            }

            Item::TypeAliasDef(type_alias_id) => {
                let ta = store[type_alias_id].borrow();
                self.visit_type_alias(ta.visibility, &ta.name, &store[&ta.type_id])
            }

            Item::StructDef(struct_def_id) => {
                let sd = store[struct_def_id].borrow();
                self.visit_struct_def(sd.visibility, &sd.name, &sd.field_extras, &sd.struct_id)
            }

            Item::EnumDef(enum_def_id) => {
                let ed = store[enum_def_id].borrow();
                self.visit_enum_def(ed.visibility, &ed.name, &ed.variant_extras, &ed.enum_id)
            }
        }
    }
}
