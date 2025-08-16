use super::abstract_machine::{AbstractMachine, EvalError};
use crate::parsetree::{
    Builder, Expr, Type,
    nodes::{
        ArrayType, FunctionParameter, FunctionType, GenericType, ManagedRefType, MapType,
        RefinementType, SliceType, TupleType, UnmanagedRefType,
    },
};

impl<'a> AbstractMachine<'a> {
    pub(crate) fn evaluate_refinement_type(
        &mut self,
        refinement: &RefinementType<'a>,
    ) -> Result<Type<'a>, EvalError> {
        let width = match refinement.width() {
            Some(w) => Some(self.evaluate(&w)?),
            None => None,
        };

        let min = match refinement.min() {
            Some(m) => Some(self.evaluate(&m)?),
            None => None,
        };

        let max = match refinement.max() {
            Some(m) => Some(self.evaluate(&m)?),
            None => None,
        };

        let base = self.evaluate_type(&refinement.base())?;

        Ok(Builder::create_refinement_type()
            .with_width(width)
            .with_minimum(min)
            .with_maximum(max)
            .with_base(base)
            .build())
    }

    pub(crate) fn evaluate_tuple_type(
        &mut self,
        tuple: &TupleType<'a>,
    ) -> Result<Type<'a>, EvalError> {
        let mut elements = Vec::new();
        elements.reserve(tuple.elements().len());

        for element in tuple.elements() {
            elements.push(self.evaluate_type(element)?);
        }

        Ok(Builder::create_tuple_type().add_elements(elements).build())
    }

    pub(crate) fn evaluate_array_type(
        &mut self,
        array: &ArrayType<'a>,
    ) -> Result<Type<'a>, EvalError> {
        let element = self.evaluate_type(&array.element())?;
        let count = match self.evaluate(&array.count())? {
            Expr::IntegerLit(count) => Expr::IntegerLit(count),
            _ => return Err(EvalError::TypeError), // Expecting integer array size
        };

        Ok(Builder::create_array_type()
            .with_element(element)
            .with_count(count)
            .build())
    }

    pub(crate) fn evaluate_map_type(&mut self, map: &MapType<'a>) -> Result<Type<'a>, EvalError> {
        let key = self.evaluate_type(&map.key())?;
        let value = self.evaluate_type(&map.value())?;

        Ok(Builder::create_map_type()
            .with_key(key)
            .with_value(value)
            .build())
    }

    pub(crate) fn evaluate_slice_type(
        &mut self,
        slice: &SliceType<'a>,
    ) -> Result<Type<'a>, EvalError> {
        let element = self.evaluate_type(&slice.element())?;
        Ok(Builder::create_slice_type().with_element(element).build())
    }

    pub(crate) fn evaluate_function_type(
        &mut self,
        function: &FunctionType<'a>,
    ) -> Result<Type<'a>, EvalError> {
        let attributes = function.attributes().to_vec();

        let mut parameters = Vec::new();
        parameters.reserve(function.parameters().len());

        for parameter in function.parameters() {
            let type_ = self.evaluate_type(&parameter.type_())?;
            let default = parameter.default();
            parameters.push(FunctionParameter::new(
                parameter.name(),
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
        reference: &ManagedRefType<'a>,
    ) -> Result<Type<'a>, EvalError> {
        let is_mutable = reference.is_mutable();
        let target_type = self.evaluate_type(&reference.target())?;

        Ok(Builder::create_managed_type()
            .with_mutability(is_mutable)
            .with_target(target_type)
            .build())
    }

    pub(crate) fn evaluate_unmanaged_ref_type(
        &mut self,
        reference: &UnmanagedRefType<'a>,
    ) -> Result<Type<'a>, EvalError> {
        let is_mutable = reference.is_mutable();
        let target_type = self.evaluate_type(&reference.target())?;

        Ok(Builder::create_unmanaged_type()
            .with_mutability(is_mutable)
            .with_target(target_type)
            .build())
    }

    pub(crate) fn evaluate_generic_type(
        &mut self,
        generic: &GenericType<'a>,
    ) -> Result<Type<'a>, EvalError> {
        // TODO: Instantiate generic base with type arguments

        let mut arguments = Vec::new();
        arguments.reserve(generic.arguments().len());

        for (name, value) in generic.arguments() {
            let evaluated_value = self.evaluate(value)?;
            arguments.push((name.to_owned(), evaluated_value));
        }

        let base = self.evaluate_type(&generic.base())?;

        Ok(Builder::create_generic_type()
            .add_arguments(arguments)
            .with_base(base)
            .build())
    }

    pub fn evaluate_type(&mut self, type_expression: &Type<'a>) -> Result<Type<'a>, EvalError> {
        if self.already_evaluated_types.contains(type_expression) {
            return Ok(type_expression.to_owned());
        }

        let result = match type_expression {
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
            Type::TypeName(name) => Ok(Type::TypeName(name)),
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
            Type::HasParenthesesType(inner) => self.evaluate_type(inner),
        };

        result.inspect(|t| {
            self.already_evaluated_types.insert(t.clone());
        })
    }
}
