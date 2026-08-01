use crate::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use std::collections::{BTreeMap, BTreeSet, HashMap};

#[derive(Debug)]
pub struct SymbolTab {
    global_variales: HashMap<NString, GlobalVariableId>,
    local_variables: HashMap<NString, LocalVariableId>,
    parameters: HashMap<NString, ParameterId>,
    functions: HashMap<NString, FunctionId>,
    enum_variants: HashMap<NString, EnumDefId>,

    type_alises: HashMap<NString, TypeAliasDefId>,
    structs: HashMap<NString, StructDefId>,
    enums: HashMap<NString, EnumDefId>,

    methods: HashMap<(TypeId, NString), FunctionId>,
    traits: HashMap<NString, TraitId>,
    impls: HashMap<TypeId, HashMap<TraitId, HashMap<NString, FunctionId>>>,
    impl_associated_types: HashMap<TypeId, HashMap<TraitId, HashMap<NString, TypeAliasDefId>>>,
    impl_associated_constants: HashMap<TypeId, HashMap<TraitId, HashMap<NString, GlobalVariableId>>>,
    arch_ptr_size: PtrSize,
}

impl SymbolTab {
    #[must_use]
    pub fn new(arch_ptr_size: PtrSize) -> Self {
        Self {
            global_variales: HashMap::new(),
            local_variables: HashMap::new(),
            parameters: HashMap::new(),
            functions: HashMap::new(),
            enum_variants: HashMap::new(),
            type_alises: HashMap::new(),
            structs: HashMap::new(),
            enums: HashMap::new(),
            methods: HashMap::new(),
            traits: HashMap::new(),
            impls: HashMap::new(),
            impl_associated_types: HashMap::new(),
            impl_associated_constants: HashMap::new(),
            arch_ptr_size,
        }
    }

    pub fn reset(&mut self) {
        *self = Self::new(self.arch_ptr_size);
    }

    pub fn add_global_variable(&mut self, global_var: GlobalVariableId) {
        let name = global_var.borrow().name.clone();
        self.global_variales.insert(name, global_var);
    }

    pub fn add_local_variable(&mut self, local_var: LocalVariableId) {
        let name = local_var.borrow().name.clone();
        self.local_variables.insert(name, local_var);
    }

    pub fn add_parameter(&mut self, param: ParameterId) {
        let name = param.borrow().name.clone();
        self.parameters.insert(name, param);
    }

    pub fn add_function(&mut self, function: FunctionId) {
        let name = function.borrow().name.clone();
        self.functions.insert(name, function);
    }

    pub fn add_enum_variant(&mut self, name: NString, enum_def_id: EnumDefId) {
        self.enum_variants.insert(name, enum_def_id);
    }

    pub fn add_method(&mut self, type_id: TypeId, method_name: NString, function_id: FunctionId) {
        self.methods.insert((type_id, method_name), function_id);
    }

    pub fn add_trait(&mut self, trait_id: TraitId) {
        let name = trait_id.borrow().name.clone();
        self.traits.insert(name, trait_id);
    }

    pub fn add_impl_trait(&mut self, type_id: TypeId, trait_id: TraitId) {
        self.impls.entry(type_id).or_default().entry(trait_id).or_default();
    }

    pub fn add_trait_method(
        &mut self,
        type_id: TypeId,
        trait_id: TraitId,
        method_name: NString,
        function_id: FunctionId,
    ) {
        self.impls
            .entry(type_id)
            .or_default()
            .entry(trait_id)
            .or_default()
            .insert(method_name, function_id);
    }

    pub fn add_type_alias(&mut self, type_alias_id: TypeAliasDefId) {
        let name = type_alias_id.borrow().name.clone();
        self.type_alises.insert(name, type_alias_id);
    }

    pub fn add_struct(&mut self, struct_def_id: StructDefId) {
        let name = struct_def_id.borrow().name.clone();
        self.structs.insert(name, struct_def_id);
    }

    pub fn add_enum(&mut self, enum_def_id: EnumDefId) {
        let name = enum_def_id.borrow().name.clone();
        self.enums.insert(name, enum_def_id);
    }

    pub fn get_global_variable_or_insert_placeholder(&mut self, name: &NString) -> GlobalVariableId {
        if let Some(global_var_id) = self.global_variales.get(name).cloned() {
            return global_var_id;
        };

        let placeholder = GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Sec,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: name.clone(),
            mangled_name: None,
            ty: Type::Unit {
                span: ByteSpan::default(),
            }
            .into(),
            initializer: Value::Unit {
                span: ByteSpan::default(),
            }
            .into(),
        };

        let global_var_id: GlobalVariableId = placeholder.into();
        self.add_global_variable(global_var_id.clone());
        self.get_global_variable_or_insert_placeholder(name)
    }

    pub fn get_global_variable(&self, name: &NString) -> Option<&GlobalVariableId> {
        self.global_variales.get(name)
    }

    pub fn globals(&self) -> impl Iterator<Item = &GlobalVariableId> {
        self.global_variales.values()
    }

    pub fn get_local_variable_or_insert_placeholder(&mut self, name: &NString) -> LocalVariableId {
        if let Some(local_var_id) = self.local_variables.get(name).cloned() {
            return local_var_id;
        };

        let placeholder = LocalVariable {
            span: ByteSpan::default(),
            kind: LocalKind::Let,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: name.clone(),
            ty: Type::Unit {
                span: ByteSpan::default(),
            }
            .into(),
            initializer: Value::Unit {
                span: ByteSpan::default(),
            }
            .into(),
        };

        let local_var_id: LocalVariableId = placeholder.into();
        self.add_local_variable(local_var_id.clone());
        self.get_local_variable_or_insert_placeholder(name)
    }

    pub fn get_local_variable(&self, name: &NString) -> Option<&LocalVariableId> {
        self.local_variables.get(name)
    }

    pub fn get_parameter_or_insert_placeholder(&mut self, name: &NString) -> ParameterId {
        if let Some(param_id) = self.parameters.get(name).cloned() {
            return param_id;
        };

        let placeholder = Parameter {
            span: ByteSpan::default(),
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: name.clone(),
            ty: Type::Unit {
                span: ByteSpan::default(),
            }
            .into(),
            default_value: None,
        };

        let param_id: ParameterId = placeholder.into();
        self.add_parameter(param_id.clone());
        self.get_parameter_or_insert_placeholder(name)
    }

    pub fn get_parameter(&self, name: &NString) -> Option<&ParameterId> {
        self.parameters.get(name)
    }

    pub fn get_function_or_insert_placeholder(&mut self, name: &NString) -> FunctionId {
        if let Some(func_id) = self.functions.get(name).cloned() {
            return func_id;
        };

        let placeholder = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Sec,
            attributes: BTreeSet::new(),
            is_unsafe: false,
            name: name.clone(),
            mangled_name: None,
            generics: None,
            params: Vec::new(),
            return_type: Type::Unit {
                span: ByteSpan::default(),
            }
            .into(),
            body: None,
        };

        let func_id: FunctionId = placeholder.into();
        self.add_function(func_id.clone());
        self.get_function_or_insert_placeholder(name)
    }

    pub fn get_function(&self, name: &NString) -> Option<&FunctionId> {
        self.functions.get(name)
    }

    pub fn functions(&self) -> impl Iterator<Item = &FunctionId> {
        self.functions.values()
    }

    pub fn get_enum_variant_or_insert_placeholder(&mut self, name: &NString) -> EnumDefId {
        if let Some(enum_def_id) = self.enum_variants.get(name).cloned() {
            return enum_def_id;
        };

        let parts = name.split("::");
        let enum_name: NString = parts
            .clone()
            .take(parts.clone().count() - 1)
            .collect::<Vec<_>>()
            .join("::")
            .into();

        let enum_def = self.get_enum_or_insert_placeholder(&enum_name);
        self.add_enum_variant(name.clone(), enum_def.clone());
        enum_def
    }

    pub fn get_enum_variant(&self, name: &NString) -> Option<&EnumDefId> {
        self.enum_variants.get(name)
    }

    pub fn get_method(&self, type_def: &TypeId, method_name: &NString) -> Option<&FunctionId> {
        if let Some(method) = self.methods.get(&(*type_def, method_name.clone())) {
            return Some(method);
        }

        if let Some(method) = self
            .impls
            .get(type_def)?
            .values()
            .find_map(|methods_map| methods_map.get(method_name))
        {
            return Some(method);
        }

        None
    }

    pub fn get_trait_or_insert_placeholder(&mut self, name: &NString) -> TraitId {
        if let Some(trait_id) = self.traits.get(name).cloned() {
            return trait_id;
        };

        let placeholder = Trait {
            span: ByteSpan::default(),
            visibility: Visibility::Sec,
            name: name.clone(),
            generics: None,
            supertraits: Vec::new(),
            where_clause: None,
            methods: Vec::new(),
            associated_types: Vec::new(),
            associated_constants: Vec::new(),
        };

        let trait_id: TraitId = placeholder.into();
        self.add_trait(trait_id.clone());
        self.get_trait_or_insert_placeholder(name)
    }

    pub fn get_trait(&self, name: &NString) -> Option<&TraitId> {
        self.traits.get(name)
    }

    pub fn add_impl_associated_type(
        &mut self,
        type_id: TypeId,
        trait_id: TraitId,
        assoc_name: NString,
        type_alias_id: TypeAliasDefId,
    ) {
        self.impl_associated_types
            .entry(type_id)
            .or_default()
            .entry(trait_id)
            .or_default()
            .insert(assoc_name, type_alias_id);
    }

    pub fn add_impl_associated_constant(
        &mut self,
        type_id: TypeId,
        trait_id: TraitId,
        assoc_name: NString,
        const_id: GlobalVariableId,
    ) {
        self.impl_associated_constants
            .entry(type_id)
            .or_default()
            .entry(trait_id)
            .or_default()
            .insert(assoc_name, const_id);
    }

    pub fn get_type_alias_or_insert_placeholder(&mut self, name: &NString) -> TypeAliasDefId {
        if let Some(type_alias_id) = self.type_alises.get(name).cloned() {
            return type_alias_id;
        };

        let placeholder = TypeAliasDef {
            span: ByteSpan::default(),
            visibility: Visibility::Sec,
            name: name.clone(),
            generics: None,
            type_id: Type::Unit {
                span: ByteSpan::default(),
            }
            .into(),
        };

        let type_alias_def: TypeAliasDefId = placeholder.into();
        self.add_type_alias(type_alias_def.clone());
        self.get_type_alias_or_insert_placeholder(name)
    }

    pub fn get_type_alias(&self, name: &NString) -> Option<&TypeAliasDefId> {
        self.type_alises.get(name)
    }

    pub fn get_struct_or_insert_placeholder(&mut self, name: &NString) -> StructDefId {
        if let Some(struct_def_id) = self.structs.get(name).cloned() {
            return struct_def_id;
        };

        let placeholder = StructDef {
            span: ByteSpan::default(),
            visibility: Visibility::Sec,
            attributes: BTreeSet::new(),
            name: name.clone(),
            generics: None,
            fields: BTreeMap::new(),
            layout: StructLayout::new(),
        };

        let struct_def: StructDefId = placeholder.into();
        self.add_struct(struct_def.clone());
        self.get_struct_or_insert_placeholder(name)
    }

    pub fn get_struct(&self, name: &NString) -> Option<&StructDefId> {
        self.structs.get(name)
    }

    pub fn get_enum_or_insert_placeholder(&mut self, name: &NString) -> EnumDefId {
        if let Some(enum_def_id) = self.enums.get(name).cloned() {
            return enum_def_id;
        };

        let placeholder = EnumDef {
            span: ByteSpan::default(),
            visibility: Visibility::Sec,
            attributes: BTreeSet::new(),
            name: name.clone(),
            generics: None,
            variants: Vec::new().into(),
        };

        let enum_def: EnumDefId = placeholder.into();
        self.add_enum(enum_def.clone());
        self.get_enum_or_insert_placeholder(name)
    }

    pub fn get_enum(&self, name: &NString) -> Option<&EnumDefId> {
        self.enums.get(name)
    }

    pub fn arch_ptr_size(&self) -> PtrSize {
        self.arch_ptr_size
    }
}
