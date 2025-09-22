use std::sync::{Arc, RwLock};

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
        f(Order::Enter, RefNodeMut::ItemSyntaxError);
        f(Order::Leave, RefNodeMut::ItemSyntaxError);
    }
}

impl ParseTreeIterMut for Package {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemPackage(self));

        let _ = self.name;
        self.root.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ItemPackage(self));
    }
}

impl ParseTreeIterMut for Module {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemModule(self));

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

        f(Order::Leave, RefNodeMut::ItemModule(self));
    }
}

impl ParseTreeIterMut for Import {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemImport(self));

        let _ = self.visibility;
        let _ = self.alias;

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        self.path.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ItemImport(self));
    }
}

impl ParseTreeIterMut for GenericParameter {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemGenericParameter(self));

        let _ = self.name;

        if let Some(default_val) = &mut self.default {
            default_val.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ItemGenericParameter(self));
    }
}

impl ParseTreeIterMut for Arc<RwLock<TypeAlias>> {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemTypeAlias(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter_mut(f);
            }
        }

        if let Some(alias_type) = &mut this.alias_type {
            alias_type.depth_first_iter_mut(f);
        }

        drop(this);
        f(Order::Leave, RefNodeMut::ItemTypeAlias(self));
    }
}

impl ParseTreeIterMut for StructField {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemStructField(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        self.field_type.depth_first_iter_mut(f);

        if let Some(default) = &mut self.default {
            default.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ItemStructField(self));
    }
}

impl ParseTreeIterMut for Arc<RwLock<Struct>> {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemStruct(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter_mut(f);
            }
        }

        for field in &mut this.fields {
            field.depth_first_iter_mut(f);
        }

        for method in &mut this.methods {
            method.depth_first_iter_mut(f);
        }

        drop(this);
        f(Order::Leave, RefNodeMut::ItemStruct(self));
    }
}

impl ParseTreeIterMut for EnumVariant {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemEnumVariant(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(variant_type) = &mut self.variant_type {
            variant_type.depth_first_iter_mut(f);
        }

        if let Some(value) = &mut self.value {
            value.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ItemEnumVariant(self));
    }
}

impl ParseTreeIterMut for Arc<RwLock<Enum>> {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemEnum(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter_mut(f);
            }
        }

        for variant in &mut this.variants {
            variant.depth_first_iter_mut(f);
        }

        drop(this);
        f(Order::Leave, RefNodeMut::ItemEnum(self));
    }
}

impl ParseTreeIterMut for AssociatedItem {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        match self {
            AssociatedItem::SyntaxError(i) => i.depth_first_iter_mut(f),
            AssociatedItem::TypeAlias(i) => i.depth_first_iter_mut(f),
            AssociatedItem::ConstantItem(i) => i.depth_first_iter_mut(f),
            AssociatedItem::Method(i) => i.depth_first_iter_mut(f),
        }
    }
}

impl ParseTreeIterMut for Arc<RwLock<Trait>> {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemTrait(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter_mut(f);
            }
        }

        for associated_item in &mut this.items {
            associated_item.depth_first_iter_mut(f);
        }

        drop(this);
        f(Order::Leave, RefNodeMut::ItemTrait(self));
    }
}

impl ParseTreeIterMut for Impl {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemImpl(self));

        let _ = self.visibility;

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

        if let Some(trait_path) = &mut self.trait_path {
            trait_path.depth_first_iter_mut(f);
        }

        self.for_type.depth_first_iter_mut(f);

        for associated_item in &mut self.items {
            associated_item.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ItemImpl(self));
    }
}

impl ParseTreeIterMut for FunctionParameter {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemFunctionParameter(self));

        let _ = self.mutability;
        let _ = self.name;

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(param_type) = &mut self.param_type {
            param_type.depth_first_iter_mut(f);
        }

        if let Some(default) = &mut self.default {
            default.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ItemFunctionParameter(self));
    }
}

impl ParseTreeIterMut for Arc<RwLock<NamedFunction>> {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemNamedFunction(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter_mut(f);
            }
        }

        for param in &mut this.parameters {
            param.depth_first_iter_mut(f);
        }

        if let Some(return_type) = &mut this.return_type {
            return_type.depth_first_iter_mut(f);
        }

        if let Some(definition) = &mut this.definition {
            definition.depth_first_iter_mut(f);
        }

        drop(this);
        f(Order::Leave, RefNodeMut::ItemNamedFunction(self));
    }
}

impl ParseTreeIterMut for Arc<RwLock<Variable>> {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemVariable(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.kind;
        let _ = this.mutability;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        if let Some(var_type) = &mut this.var_type {
            var_type.depth_first_iter_mut(f);
        }

        if let Some(initializer) = &mut this.initializer {
            initializer.depth_first_iter_mut(f);
        }

        drop(this);
        f(Order::Leave, RefNodeMut::ItemVariable(self));
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
