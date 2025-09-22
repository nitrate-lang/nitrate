use std::sync::{Arc, RwLock};

use crate::{
    Order, ParseTreeIter, RefNode,
    item::{ItemSyntaxError, Package},
    kind::{
        AssociatedItem, Enum, EnumVariant, FunctionParameter, GenericParameter, Impl, Import, Item,
        Module, NamedFunction, Struct, StructField, Trait, TypeAlias, Variable,
    },
};

impl ParseTreeIter for ItemSyntaxError {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemSyntaxError);
        f(Order::Post, RefNode::ItemSyntaxError);
    }
}

impl ParseTreeIter for Package {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemPackage(self));

        let _ = self.name;
        self.root.depth_first_iter(f);

        f(Order::Post, RefNode::ItemPackage(self));
    }
}

impl ParseTreeIter for Module {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemModule(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        for item in &self.items {
            item.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ItemModule(self));
    }
}

impl ParseTreeIter for Import {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemImport(self));

        let _ = self.visibility;
        let _ = self.alias;

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        self.path.depth_first_iter(f);

        f(Order::Post, RefNode::ItemImport(self));
    }
}

impl ParseTreeIter for GenericParameter {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemGenericParameter(self));

        let _ = self.name;

        if let Some(default_val) = &self.default {
            default_val.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ItemGenericParameter(self));
    }
}

impl ParseTreeIter for Arc<RwLock<TypeAlias>> {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemTypeAlias(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter(f);
            }
        }

        if let Some(alias_type) = &mut this.alias_type {
            alias_type.depth_first_iter(f);
        }

        drop(this);
        f(Order::Post, RefNode::ItemTypeAlias(self));
    }
}

impl ParseTreeIter for StructField {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemStructField(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        self.field_type.depth_first_iter(f);

        if let Some(default) = &self.default {
            default.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ItemStructField(self));
    }
}

impl ParseTreeIter for Arc<RwLock<Struct>> {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemStruct(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter(f);
            }
        }

        for field in &mut this.fields {
            field.depth_first_iter(f);
        }

        for method in &mut this.methods {
            method.depth_first_iter(f);
        }

        drop(this);
        f(Order::Post, RefNode::ItemStruct(self));
    }
}

impl ParseTreeIter for EnumVariant {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemEnumVariant(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        if let Some(variant_type) = &self.variant_type {
            variant_type.depth_first_iter(f);
        }

        if let Some(value) = &self.value {
            value.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ItemEnumVariant(self));
    }
}

impl ParseTreeIter for Arc<RwLock<Enum>> {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemEnum(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter(f);
            }
        }

        for variant in &mut this.variants {
            variant.depth_first_iter(f);
        }

        drop(this);
        f(Order::Post, RefNode::ItemEnum(self));
    }
}

impl ParseTreeIter for AssociatedItem {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        match self {
            AssociatedItem::SyntaxError(i) => i.depth_first_iter(f),
            AssociatedItem::TypeAlias(i) => i.depth_first_iter(f),
            AssociatedItem::ConstantItem(i) => i.depth_first_iter(f),
            AssociatedItem::Method(i) => i.depth_first_iter(f),
        }
    }
}

impl ParseTreeIter for Arc<RwLock<Trait>> {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemTrait(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter(f);
            }
        }

        for associated_item in &mut this.items {
            associated_item.depth_first_iter(f);
        }

        drop(this);
        f(Order::Post, RefNode::ItemTrait(self));
    }
}

impl ParseTreeIter for Impl {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemImpl(self));

        let _ = self.visibility;

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        if let Some(params) = &self.type_params {
            for param in params {
                param.depth_first_iter(f);
            }
        }

        if let Some(trait_path) = &self.trait_path {
            trait_path.depth_first_iter(f);
        }

        self.for_type.depth_first_iter(f);

        for associated_item in &self.items {
            associated_item.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ItemImpl(self));
    }
}

impl ParseTreeIter for FunctionParameter {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemFunctionParameter(self));

        let _ = self.mutability;
        let _ = self.name;

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        if let Some(param_type) = &self.param_type {
            param_type.depth_first_iter(f);
        }

        if let Some(default) = &self.default {
            default.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ItemFunctionParameter(self));
    }
}

impl ParseTreeIter for Arc<RwLock<NamedFunction>> {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemNamedFunction(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        if let Some(params) = &mut this.type_params {
            for param in params {
                param.depth_first_iter(f);
            }
        }

        for param in &mut this.parameters {
            param.depth_first_iter(f);
        }

        if let Some(return_type) = &mut this.return_type {
            return_type.depth_first_iter(f);
        }

        if let Some(definition) = &mut this.definition {
            definition.depth_first_iter(f);
        }

        drop(this);
        f(Order::Post, RefNode::ItemNamedFunction(self));
    }
}

impl ParseTreeIter for Arc<RwLock<Variable>> {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ItemVariable(self));
        let mut this = self.write().unwrap();

        let _ = this.visibility;
        let _ = this.kind;
        let _ = this.mutability;
        let _ = this.name;

        if let Some(attrs) = &mut this.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        if let Some(var_type) = &mut this.var_type {
            var_type.depth_first_iter(f);
        }

        if let Some(initializer) = &mut this.initializer {
            initializer.depth_first_iter(f);
        }

        drop(this);
        f(Order::Post, RefNode::ItemVariable(self));
    }
}

impl ParseTreeIter for Item {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        match self {
            Item::SyntaxError(item) => item.depth_first_iter(f),
            Item::Module(item) => item.depth_first_iter(f),
            Item::Import(item) => item.depth_first_iter(f),
            Item::TypeAlias(item) => item.depth_first_iter(f),
            Item::Struct(item) => item.depth_first_iter(f),
            Item::Enum(item) => item.depth_first_iter(f),
            Item::Trait(item) => item.depth_first_iter(f),
            Item::Impl(item) => item.depth_first_iter(f),
            Item::NamedFunction(item) => item.depth_first_iter(f),
            Item::Variable(item) => item.depth_first_iter(f),
        }
    }
}
