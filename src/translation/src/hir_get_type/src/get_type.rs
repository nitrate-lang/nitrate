use nitrate_hir::{
    Store, TypeId,
    hir::{IntoStoreId, Type, Value},
};

pub enum TypeInferenceError {
    CannotInfer,
}

pub fn get_type(value: &Value, store: &Store) -> Result<Type, TypeInferenceError> {
    match value {
        Value::Unit => Ok(Type::Unit),
        Value::Bool(_) => Ok(Type::Bool),
        Value::I8(_) => Ok(Type::I8),
        Value::I16(_) => Ok(Type::I16),
        Value::I32(_) => Ok(Type::I32),
        Value::I64(_) => Ok(Type::I64),
        Value::I128(_) => Ok(Type::I128),
        Value::U8(_) => Ok(Type::U8),
        Value::U16(_) => Ok(Type::U16),
        Value::U32(_) => Ok(Type::U32),
        Value::U64(_) => Ok(Type::U64),
        Value::U128(_) => Ok(Type::U128),
        Value::F8(_) => Ok(Type::F8),
        Value::F16(_) => Ok(Type::F16),
        Value::F32(_) => Ok(Type::F32),
        Value::F64(_) => Ok(Type::F64),
        Value::F128(_) => Ok(Type::F128),
        Value::USize32(_) => Ok(Type::USize),
        Value::USize64(_) => Ok(Type::USize),
        Value::InferredInteger(_) => Ok(Type::InferredInteger),
        Value::InferredFloat(_) => Ok(Type::InferredFloat),

        Value::StringLit(thin_str) => {
            let element_type = Type::U8.into_id(store);
            Ok(Type::Array {
                element_type,
                len: thin_str.len() as u64,
            })
        }

        Value::BStringLit(thin_vec) => {
            let element_type = Type::U8.into_id(store);
            Ok(Type::Array {
                element_type,
                len: thin_vec.len() as u64,
            })
        }

        Value::Struct {
            struct_type,
            fields,
        } => {
            // TODO: Get type of struct
            todo!()
        }

        Value::Enum {
            enum_type,
            variant,
            value,
        } => {
            // TODO: Get type of enum
            todo!()
        }

        Value::Binary { left, op, right } => {
            // TODO: Implement type inference for binary operations
            todo!()
        }

        Value::Unary { op, expr } => {
            // TODO: Implement type inference for unary operations
            todo!()
        }

        Value::FieldAccess { expr, field } => {
            // TODO: Implement type inference for field access
            todo!()
        }

        Value::IndexAccess { collection, index } => {
            // TODO: Implement type inference for index access
            todo!()
        }

        Value::Assign { place, value } => {
            // TODO: Implement type inference for assignment
            todo!()
        }

        Value::Deref { place } => {
            // TODO: Implement type inference for dereference
            todo!()
        }

        Value::Cast { expr, to } => {
            // TODO: Implement type inference for cast
            todo!()
        }

        Value::GetAddressOf { place } => {
            // TODO: Implement type inference for address-of
            todo!()
        }

        Value::GetTypeOf { expr } => {
            // TODO: Implement type inference for type-of
            todo!()
        }

        Value::List { elements } => {
            todo!()
        }

        Value::Tuple { elements } => {
            // TODO: Implement type inference for tuple
            todo!()
        }

        Value::If {
            condition,
            true_branch,
            false_branch,
        } => {
            // TODO: Implement type inference for if expressions
            todo!()
        }

        Value::While { condition, body } => {
            // TODO: Implement type inference for while loops
            todo!()
        }

        Value::Loop { body } => {
            // TODO: Implement type inference for loop
            todo!()
        }

        Value::Break { label } => {
            // TODO: Implement type inference for break
            todo!()
        }

        Value::Continue { label } => {
            // TODO: Implement type inference for continue
            todo!()
        }

        Value::Return { value } => {
            // TODO: Implement type inference for return
            todo!()
        }

        Value::Block { block } => {
            // TODO: Implement type inference for block
            todo!()
        }

        Value::Call { callee, arguments } => {
            // TODO: Implement type inference for function calls
            todo!()
        }

        Value::Symbol { symbol } => {
            // TODO: Implement type inference for symbols
            todo!()
        }
    }
}
