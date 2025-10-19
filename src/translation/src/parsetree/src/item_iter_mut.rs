use std::sync::{Arc, RwLock};

use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    ast::{
        AssociatedItem, Enum, EnumVariant, FuncParam, Function, Generics, Impl, Import, Item,
        Module, Struct, StructField, Trait, TypeAlias, Variable,
    },
    item::{FuncParams, ItemSyntaxError},
};

impl ParseTreeIterMut for ItemSyntaxError {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemSyntaxError);
        f(Order::Leave, RefNodeMut::ItemSyntaxError);
    }
}

impl ParseTreeIterMut for Module {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemModule(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attributes) = &mut self.attributes {
            attributes.depth_first_iter_mut(f);
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
        let _ = self.items;
        let _ = self.import_name;

        if let Some(attributes) = &mut self.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(resolved) = &mut self.resolved {
            resolved.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ItemImport(self));
    }
}

impl ParseTreeIterMut for Generics {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemTypeParams(self));

        for param in &mut self.params {
            let _ = param.name;

            if let Some(default_val) = &mut param.default_value {
                default_val.depth_first_iter_mut(f);
            }
        }

        f(Order::Leave, RefNodeMut::ItemTypeParams(self));
    }
}

impl ParseTreeIterMut for Arc<RwLock<TypeAlias>> {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemTypeAlias(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attributes) = &mut this.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(params) = &mut this.generics {
            params.depth_first_iter_mut(f);
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

        if let Some(attributes) = &mut self.attributes {
            attributes.depth_first_iter_mut(f);
        }

        self.ty.depth_first_iter_mut(f);

        if let Some(default) = &mut self.default_value {
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

        if let Some(attributes) = &mut this.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(params) = &mut this.generics {
            params.depth_first_iter_mut(f);
        }

        for field in &mut this.fields {
            field.depth_first_iter_mut(f);
        }

        drop(this);
        f(Order::Leave, RefNodeMut::ItemStruct(self));
    }
}

impl ParseTreeIterMut for EnumVariant {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemEnumVariant(self));

        let _ = self.name;

        if let Some(attributes) = &mut self.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(variant_type) = &mut self.variant_type {
            variant_type.depth_first_iter_mut(f);
        }

        if let Some(value) = &mut self.default_value {
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

        if let Some(attributes) = &mut this.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(params) = &mut this.generics {
            params.depth_first_iter_mut(f);
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

        if let Some(attributes) = &mut this.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(params) = &mut this.generics {
            params.depth_first_iter_mut(f);
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

        if let Some(attributes) = &mut self.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(params) = &mut self.generics {
            params.depth_first_iter_mut(f);
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

impl ParseTreeIterMut for FuncParam {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemFunctionParameter(self));

        let _ = self.mutability;
        let _ = self.name;

        if let Some(attributes) = &mut self.attributes {
            attributes.depth_first_iter_mut(f);
        }

        self.ty.depth_first_iter_mut(f);

        if let Some(default) = &mut self.default_value {
            default.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ItemFunctionParameter(self));
    }
}

impl ParseTreeIterMut for FuncParams {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        for param in self {
            param.depth_first_iter_mut(f);
        }
    }
}

impl ParseTreeIterMut for Arc<RwLock<Function>> {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ItemFunction(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attributes) = &mut this.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(params) = &mut this.generics {
            params.depth_first_iter_mut(f);
        }

        this.parameters.depth_first_iter_mut(f);

        if let Some(return_type) = &mut this.return_type {
            return_type.depth_first_iter_mut(f);
        }

        if let Some(definition) = &mut this.definition {
            definition.depth_first_iter_mut(f);
        }

        drop(this);
        f(Order::Leave, RefNodeMut::ItemFunction(self));
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

        if let Some(attributes) = &mut this.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(var_type) = &mut this.ty {
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
            Item::Function(item) => item.depth_first_iter_mut(f),
            Item::Variable(item) => item.depth_first_iter_mut(f),
        }
    }
}
