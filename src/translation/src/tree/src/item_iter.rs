use crate::{
    Order, ParseTreeIter, RefNode,
    ast::{
        AssociatedItem, Enum, EnumVariant, FuncParam, Function, Generics, GlobalVariable, Impl,
        Import, Item, Module, Struct, StructField, Trait, TypeAlias,
    },
    item::{FuncParams, ItemSyntaxError},
};

impl ParseTreeIter for ItemSyntaxError {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemSyntaxError);
        f(Order::Leave, RefNode::ItemSyntaxError);
    }
}

impl ParseTreeIter for Module {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemModule(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        for item in &self.items {
            item.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemModule(self));
    }
}

impl ParseTreeIter for Import {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemImport(self));

        let _ = self.visibility;
        let _ = self.use_tree;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(resolved) = &self.resolved {
            for item in resolved {
                item.depth_first_iter(f);
            }
        }

        f(Order::Leave, RefNode::ItemImport(self));
    }
}

impl ParseTreeIter for Generics {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemTypeParams(self));

        for param in &self.params {
            let _ = param.name;

            if let Some(default_val) = &param.default_value {
                default_val.depth_first_iter(f);
            }
        }

        f(Order::Leave, RefNode::ItemTypeParams(self));
    }
}

impl ParseTreeIter for TypeAlias {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemTypeAlias(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(params) = &self.generics {
            params.depth_first_iter(f);
        }

        if let Some(alias_type) = &self.alias_type {
            alias_type.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemTypeAlias(self));
    }
}

impl ParseTreeIter for StructField {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemStructField(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        self.ty.depth_first_iter(f);

        if let Some(default) = &self.default_value {
            default.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemStructField(self));
    }
}

impl ParseTreeIter for Struct {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemStruct(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(params) = &self.generics {
            params.depth_first_iter(f);
        }

        for field in &self.fields {
            field.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemStruct(self));
    }
}

impl ParseTreeIter for EnumVariant {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemEnumVariant(self));

        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(variant_type) = &self.ty {
            variant_type.depth_first_iter(f);
        }

        if let Some(value) = &self.default_value {
            value.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemEnumVariant(self));
    }
}

impl ParseTreeIter for Enum {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemEnum(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(params) = &self.generics {
            params.depth_first_iter(f);
        }

        for variant in &self.variants {
            variant.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemEnum(self));
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

impl ParseTreeIter for Trait {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemTrait(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(params) = &self.generics {
            params.depth_first_iter(f);
        }

        for associated_item in &self.items {
            associated_item.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemTrait(self));
    }
}

impl ParseTreeIter for Impl {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemImpl(self));

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(params) = &self.generics {
            params.depth_first_iter(f);
        }

        if let Some(trait_path) = &self.trait_path {
            trait_path.depth_first_iter(f);
        }

        self.for_type.depth_first_iter(f);

        for associated_item in &self.items {
            associated_item.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemImpl(self));
    }
}

impl ParseTreeIter for FuncParam {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemFuncParam(self));

        let _ = self.mutability;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        self.ty.depth_first_iter(f);

        if let Some(default) = &self.default_value {
            default.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemFuncParam(self));
    }
}

impl ParseTreeIter for FuncParams {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        for param in self {
            param.depth_first_iter(f);
        }
    }
}

impl ParseTreeIter for Function {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemFunction(self));

        let _ = self.visibility;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(params) = &self.generics {
            params.depth_first_iter(f);
        }

        self.parameters.depth_first_iter(f);

        if let Some(return_type) = &self.return_type {
            return_type.depth_first_iter(f);
        }

        if let Some(definition) = &self.definition {
            definition.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemFunction(self));
    }
}

impl ParseTreeIter for GlobalVariable {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ItemGlobalVariable(self));

        let _ = self.visibility;
        let _ = self.kind;
        let _ = self.mutability;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(var_type) = &self.ty {
            var_type.depth_first_iter(f);
        }

        if let Some(initializer) = &self.initializer {
            initializer.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ItemGlobalVariable(self));
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
            Item::Function(item) => item.depth_first_iter(f),
            Item::Variable(item) => item.depth_first_iter(f),
        }
    }
}
