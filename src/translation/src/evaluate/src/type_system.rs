use super::abstract_machine::{AbstractMachine, Unwind};
use nitrate_parsetree::{
    Builder,
    kind::{
        ArrayType, Expr, FunctionParameter, FunctionType, GenericType, ManagedRefType, MapType,
        RefinementType, SliceType, StructType, TupleType, Type, UnmanagedRefType,
    },
};

impl AbstractMachine {
    pub(crate) fn evaluate_refinement_type(
        &mut self,
        refinement: &RefinementType,
    ) -> Result<Type, Unwind> {
        /*
         * The order of evaluation:
         * 1. Base type
         * 2. Width
         * 3. Minimum
         * 4. Maximum
         *
         * ------------------
         * let R = `<refinement type>`;
         * let base  = eval(R.get_base());
         * let width = eval(R.get_width());
         * let min   = eval(R.get_min());
         * let max   = eval(R.get_max());
         *
         * ret type(base: width: [min: max]);
         */

        let base = self.evaluate_type(&refinement.base())?;

        let width = match refinement.width() {
            Some(width) => Some(self.evaluate(&width)?),
            None => None,
        };

        let min = match refinement.min() {
            Some(min) => Some(self.evaluate(&min)?),
            None => None,
        };

        let max = match refinement.max() {
            Some(max) => Some(self.evaluate(&max)?),
            None => None,
        };

        Ok(Builder::create_refinement_type()
            .with_base(base)
            .with_width(width)
            .with_minimum(min)
            .with_maximum(max)
            .build())
    }

    pub(crate) fn evaluate_tuple_type(&mut self, tuple: &TupleType) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let mut elements = Vec::new();
        elements.reserve(tuple.elements().len());

        for element in tuple.elements() {
            elements.push(self.evaluate_type(element)?);
        }

        Ok(Builder::create_tuple_type().add_elements(elements).build())
    }

    pub(crate) fn evaluate_array_type(&mut self, array: &ArrayType) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let element = self.evaluate_type(&array.element())?;
        let count = match self.evaluate(&array.count())? {
            Expr::Integer(count) => Expr::Integer(count),
            _ => return Err(Unwind::TypeError), // Expecting integer array size
        };

        Ok(Builder::create_array_type()
            .with_element(element)
            .with_count(count)
            .build())
    }

    pub(crate) fn evaluate_map_type(&mut self, map: &MapType) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let key = self.evaluate_type(&map.key())?;
        let value = self.evaluate_type(&map.value())?;

        Ok(Builder::create_map_type()
            .with_key(key)
            .with_value(value)
            .build())
    }

    pub(crate) fn evaluate_slice_type(&mut self, slice: &SliceType) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let element = self.evaluate_type(&slice.element())?;
        Ok(Builder::create_slice_type().with_element(element).build())
    }

    pub(crate) fn evaluate_function_type(
        &mut self,
        function: &FunctionType,
    ) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let attributes = function.attributes().to_vec();

        let mut parameters = Vec::new();
        parameters.reserve(function.parameters().len());

        for parameter in function.parameters() {
            let type_ = self.evaluate_type(&parameter.type_())?;
            let default = parameter.default();
            parameters.push(FunctionParameter::new(
                parameter.name().to_owned(),
                type_,
                default.cloned(),
            ));
        }

        let return_type = self.evaluate_type(&function.return_type())?;

        Ok(Builder::create_function_type()
            .add_attributes(attributes)
            .add_parameters(parameters)
            .with_return_type(return_type)
            .build())
    }

    pub(crate) fn evaluate_managed_ref_type(
        &mut self,
        reference: &ManagedRefType,
    ) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let is_mutable = reference.is_mutable();
        let target_type = self.evaluate_type(&reference.target())?;

        Ok(Builder::create_managed_type()
            .with_mutability(is_mutable)
            .with_target(target_type)
            .build())
    }

    pub(crate) fn evaluate_unmanaged_ref_type(
        &mut self,
        reference: &UnmanagedRefType,
    ) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let is_mutable = reference.is_mutable();
        let target_type = self.evaluate_type(&reference.target())?;

        Ok(Builder::create_unmanaged_type()
            .with_mutability(is_mutable)
            .with_target(target_type)
            .build())
    }

    pub(crate) fn evaluate_generic_type(&mut self, generic: &GenericType) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let base = self.evaluate_type(&generic.base())?;

        let mut arguments = Vec::new();
        arguments.reserve(generic.arguments().len());

        for (name, value) in generic.arguments() {
            let evaluated_type = self.evaluate_type(value)?;
            arguments.push((name.to_owned(), evaluated_type));
        }

        Ok(Builder::create_generic_type()
            .with_base(base)
            .add_arguments(arguments)
            .build())
    }

    pub(crate) fn evaluate_struct_type(
        &mut self,
        struct_type: &StructType,
    ) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let mut fields = Vec::new();
        fields.reserve(struct_type.fields().len());

        for (name, type_, default) in struct_type.fields() {
            let evaluated_type = self.evaluate_type(type_)?;
            fields.push((name.to_owned(), evaluated_type, default.clone()));
        }

        Ok(Builder::create_struct_type().add_fields(fields).build())
    }

    pub(crate) fn evaluate_latent_type(&mut self, latent_type: &Expr) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        let Expr::Object(_object) = self.evaluate(latent_type)? else {
            return Err(Unwind::TypeError);
        };

        // TODO: Implement latent type evaluation
        todo!()

        // object
        //     .access("inner".intern())
        //     .ok_or(Unwind::TypeError)?
        //     .to_owned()
        //     .try_into()
        //     .map_err(|_| Unwind::TypeError)
    }

    pub fn evaluate_type(&mut self, type_expression: &Type) -> Result<Type, Unwind> {
        // TODO: Verify and write tests

        match type_expression {
            Type::Bool => Ok(Type::Bool),
            Type::UInt8 => Ok(Type::UInt8),
            Type::UInt16 => Ok(Type::UInt16),
            Type::UInt32 => Ok(Type::UInt32),
            Type::UInt64 => Ok(Type::UInt64),
            Type::UInt128 => Ok(Type::UInt128),
            Type::Int8 => Ok(Type::Int8),
            Type::Int16 => Ok(Type::Int16),
            Type::Int32 => Ok(Type::Int32),
            Type::Int64 => Ok(Type::Int64),
            Type::Int128 => Ok(Type::Int128),
            Type::Float8 => Ok(Type::Float8),
            Type::Float16 => Ok(Type::Float16),
            Type::Float32 => Ok(Type::Float32),
            Type::Float64 => Ok(Type::Float64),
            Type::Float128 => Ok(Type::Float128),
            Type::UnitType => Ok(Type::UnitType),
            Type::InferType => Ok(Type::InferType),
            Type::TypeName(name) => Ok(Type::TypeName(name.to_owned())),
            Type::OpaqueType(identity) => Ok(Type::OpaqueType(identity.clone())),
            Type::RefinementType(refinement) => self.evaluate_refinement_type(refinement),
            Type::TupleType(tuple) => self.evaluate_tuple_type(tuple),
            Type::ArrayType(array) => self.evaluate_array_type(array),
            Type::MapType(map) => self.evaluate_map_type(map),
            Type::SliceType(slice) => self.evaluate_slice_type(slice),
            Type::FunctionType(function) => self.evaluate_function_type(function),
            Type::ManagedRefType(reference) => self.evaluate_managed_ref_type(reference),
            Type::UnmanagedRefType(reference) => self.evaluate_unmanaged_ref_type(reference),
            Type::GenericType(generic) => self.evaluate_generic_type(generic),
            Type::StructType(struct_type) => self.evaluate_struct_type(struct_type),
            Type::LatentType(latent_type) => self.evaluate_latent_type(latent_type),
            Type::HasParenthesesType(inner) => self.evaluate_type(inner),
        }
    }
}
