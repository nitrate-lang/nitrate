use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    item::{ItemSyntaxError, Package},
    kind::{
        AssociatedItem, Enum, EnumVariant, FunctionParameter, GenericParameter, Impl, Import, Item,
        Module, NamedFunction, Struct, StructField, Trait, TypeAlias, Variable,
    },
};

impl ParseTreeIterMut for ItemSyntaxError {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ItemSyntaxError);
        f(Order::Post, RefNodeMut::ItemSyntaxError);
    }
}

impl ParseTreeIterMut for Package {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ItemPackage(self));

        let _ = self.name;
        self.root.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ItemPackage(self));
    }
}

impl ParseTreeIterMut for Module {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ItemModule(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        for item in &mut self.items {
            item.depth_first_iter_mut(f);
        }

        f(Order::Post, RefNodeMut::ItemModule(self));
    }
}

impl ParseTreeIterMut for Import {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ItemImport(self));

        let _ = self.visibility;
        let _ = self.alias;

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        self.path.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ItemImport(self));
    }
}

impl ParseTreeIterMut for GenericParameter {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ItemGenericParameter(self));

        let _ = self.name;
        self.default.as_mut().map(|ty| ty.depth_first_iter_mut(f));

        f(Order::Post, RefNodeMut::ItemGenericParameter(self));
    }
}

impl ParseTreeIterMut for TypeAlias {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ItemTypeAlias(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(params) = &mut self.type_params {
            for param in params {
                param.depth_first_iter_mut(f);
            }
        }

        self.alias_type
            .as_mut()
            .map(|ty| ty.depth_first_iter_mut(f));

        f(Order::Post, RefNodeMut::ItemTypeAlias(self));
    }
}

impl ParseTreeIterMut for StructField {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ItemStructField(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        self.field_type.depth_first_iter_mut(f);
        self.default.as_mut().map(|e| e.depth_first_iter_mut(f));

        f(Order::Post, RefNodeMut::ItemStructField(self));
    }
}

impl ParseTreeIterMut for Struct {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ItemStruct(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(params) = &mut self.type_params {
            for param in params {
                param.depth_first_iter_mut(f);
            }
        }

        for field in &mut self.fields {
            field.depth_first_iter_mut(f);
        }

        for method in &mut self.methods {
            method.depth_first_iter_mut(f);
        }

        f(Order::Post, RefNodeMut::ItemStruct(self));
    }
}

impl ParseTreeIterMut for EnumVariant {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Enum {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for AssociatedItem {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Trait {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Impl {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for FunctionParameter {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for NamedFunction {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Variable {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Item {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        match self {
            Item::SyntaxError(item) => item.depth_first_iter_mut(f),
            Item::Module(item) => item.depth_first_iter_mut(f),
            Item::Import(item) => item.depth_first_iter_mut(f),
            Item::TypeAlias(item) => item.depth_first_iter_mut(f),
            Item::Struct(item) => item.depth_first_iter_mut(f),
            Item::Enum(item) => item.depth_first_iter_mut(f),
            Item::Trait(item) => item.depth_first_iter_mut(f),
            Item::Impl(item) => item.depth_first_iter_mut(f),
            Item::NamedFunction(item) => item.depth_first_iter_mut(f),
            Item::Variable(item) => item.depth_first_iter_mut(f),
        }
    }
}
