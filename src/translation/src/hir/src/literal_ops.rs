use crate::hir::Literal;

pub enum LiteralNegError {
    TypeError,
}

impl std::ops::Neg for Literal {
    type Output = Result<Literal, LiteralNegError>;

    fn neg(self) -> Self::Output {
        match self {
            Literal::I8(a) => Ok(Literal::I8(a.wrapping_neg())),
            Literal::I16(a) => Ok(Literal::I16(a.wrapping_neg())),
            Literal::I32(a) => Ok(Literal::I32(a.wrapping_neg())),
            Literal::I64(a) => Ok(Literal::I64(a.wrapping_neg())),
            Literal::I128(a) => Ok(Literal::I128(a.wrapping_neg())),
            Literal::ISize(a) => Ok(Literal::ISize(a.wrapping_neg())),
            Literal::U8(a) => Ok(Literal::U8(a.wrapping_neg())),
            Literal::U16(a) => Ok(Literal::U16(a.wrapping_neg())),
            Literal::U32(a) => Ok(Literal::U32(a.wrapping_neg())),
            Literal::U64(a) => Ok(Literal::U64(a.wrapping_neg())),
            Literal::U128(a) => Ok(Literal::U128(a.wrapping_neg())),
            Literal::USize(a) => Ok(Literal::USize(a.wrapping_neg())),
            Literal::F8(a) => Ok(Literal::F8(a.neg())),
            Literal::F16(a) => Ok(Literal::F16(a.neg())),
            Literal::F32(a) => Ok(Literal::F32(a.neg())),
            Literal::F64(a) => Ok(Literal::F64(a.neg())),
            Literal::F128(a) => Ok(Literal::F128(a.neg())),
            Literal::Bool(_) | Literal::Unit => Err(LiteralNegError::TypeError),
        }
    }
}

pub enum LiteralNotError {
    TypeError,
}

impl std::ops::Not for Literal {
    type Output = Result<Literal, LiteralNotError>;

    fn not(self) -> Self::Output {
        match self {
            Literal::Bool(b) => Ok(Literal::Bool(b.not())),
            Literal::I8(a) => Ok(Literal::I8(a.not())),
            Literal::I16(a) => Ok(Literal::I16(a.not())),
            Literal::I32(a) => Ok(Literal::I32(a.not())),
            Literal::I64(a) => Ok(Literal::I64(a.not())),
            Literal::I128(a) => Ok(Literal::I128(a.not())),
            Literal::ISize(a) => Ok(Literal::ISize(a.not())),
            Literal::U8(a) => Ok(Literal::U8(a.not())),
            Literal::U16(a) => Ok(Literal::U16(a.not())),
            Literal::U32(a) => Ok(Literal::U32(a.not())),
            Literal::U64(a) => Ok(Literal::U64(a.not())),
            Literal::U128(a) => Ok(Literal::U128(a.not())),
            Literal::USize(a) => Ok(Literal::USize(a.not())),
            Literal::F8(_)
            | Literal::F16(_)
            | Literal::F32(_)
            | Literal::F64(_)
            | Literal::F128(_)
            | Literal::Unit => Err(LiteralNotError::TypeError),
        }
    }
}

pub enum LiteralAddError {
    TypeError,
}

impl std::ops::Add for Literal {
    type Output = Result<Literal, LiteralAddError>;

    fn add(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => Ok(Literal::I8(a.wrapping_add(b))),
            (Literal::I16(a), Literal::I16(b)) => Ok(Literal::I16(a.wrapping_add(b))),
            (Literal::I32(a), Literal::I32(b)) => Ok(Literal::I32(a.wrapping_add(b))),
            (Literal::I64(a), Literal::I64(b)) => Ok(Literal::I64(a.wrapping_add(b))),
            (Literal::I128(a), Literal::I128(b)) => Ok(Literal::I128(a.wrapping_add(b))),
            (Literal::ISize(a), Literal::ISize(b)) => Ok(Literal::ISize(a.wrapping_add(b))),
            (Literal::U8(a), Literal::U8(b)) => Ok(Literal::U8(a.wrapping_add(b))),
            (Literal::U16(a), Literal::U16(b)) => Ok(Literal::U16(a.wrapping_add(b))),
            (Literal::U32(a), Literal::U32(b)) => Ok(Literal::U32(a.wrapping_add(b))),
            (Literal::U64(a), Literal::U64(b)) => Ok(Literal::U64(a.wrapping_add(b))),
            (Literal::U128(a), Literal::U128(b)) => Ok(Literal::U128(a.wrapping_add(b))),
            (Literal::USize(a), Literal::USize(b)) => Ok(Literal::USize(a.wrapping_add(b))),
            (Literal::F8(a), Literal::F8(b)) => {
                Ok(Literal::F8(a.add(b))) // TODO: Do proper f8 addition
            }
            (Literal::F16(a), Literal::F16(b)) => {
                Ok(Literal::F16(a.add(b))) // TODO: Do proper f16 addition
            }
            (Literal::F32(a), Literal::F32(b)) => Ok(Literal::F32(a.add(b))),
            (Literal::F64(a), Literal::F64(b)) => Ok(Literal::F64(a.add(b))),
            (Literal::F128(a), Literal::F128(b)) => Ok(Literal::F128(a.add(b))),

            _ => Err(LiteralAddError::TypeError),
        }
    }
}

pub enum LiteralSubError {
    TypeError,
}

impl std::ops::Sub for Literal {
    type Output = Result<Literal, LiteralSubError>;

    fn sub(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => Ok(Literal::I8(a.wrapping_sub(b))),
            (Literal::I16(a), Literal::I16(b)) => Ok(Literal::I16(a.wrapping_sub(b))),
            (Literal::I32(a), Literal::I32(b)) => Ok(Literal::I32(a.wrapping_sub(b))),
            (Literal::I64(a), Literal::I64(b)) => Ok(Literal::I64(a.wrapping_sub(b))),
            (Literal::I128(a), Literal::I128(b)) => Ok(Literal::I128(a.wrapping_sub(b))),
            (Literal::ISize(a), Literal::ISize(b)) => Ok(Literal::ISize(a.wrapping_sub(b))),
            (Literal::U8(a), Literal::U8(b)) => Ok(Literal::U8(a.wrapping_sub(b))),
            (Literal::U16(a), Literal::U16(b)) => Ok(Literal::U16(a.wrapping_sub(b))),
            (Literal::U32(a), Literal::U32(b)) => Ok(Literal::U32(a.wrapping_sub(b))),
            (Literal::U64(a), Literal::U64(b)) => Ok(Literal::U64(a.wrapping_sub(b))),
            (Literal::U128(a), Literal::U128(b)) => Ok(Literal::U128(a.wrapping_sub(b))),
            (Literal::USize(a), Literal::USize(b)) => Ok(Literal::USize(a.wrapping_sub(b))),
            (Literal::F8(a), Literal::F8(b)) => Ok(Literal::F8(a.sub(b))),
            (Literal::F16(a), Literal::F16(b)) => Ok(Literal::F16(a.sub(b))),
            (Literal::F32(a), Literal::F32(b)) => Ok(Literal::F32(a.sub(b))),
            (Literal::F64(a), Literal::F64(b)) => Ok(Literal::F64(a.sub(b))),
            (Literal::F128(a), Literal::F128(b)) => Ok(Literal::F128(a.sub(b))),
            _ => Err(LiteralSubError::TypeError),
        }
    }
}

pub enum LiteralMulError {
    TypeError,
}

impl std::ops::Mul for Literal {
    type Output = Result<Literal, LiteralMulError>;

    fn mul(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => Ok(Literal::I8(a.wrapping_mul(b))),
            (Literal::I16(a), Literal::I16(b)) => Ok(Literal::I16(a.wrapping_mul(b))),
            (Literal::I32(a), Literal::I32(b)) => Ok(Literal::I32(a.wrapping_mul(b))),
            (Literal::I64(a), Literal::I64(b)) => Ok(Literal::I64(a.wrapping_mul(b))),
            (Literal::I128(a), Literal::I128(b)) => Ok(Literal::I128(a.wrapping_mul(b))),
            (Literal::ISize(a), Literal::ISize(b)) => Ok(Literal::ISize(a.wrapping_mul(b))),
            (Literal::U8(a), Literal::U8(b)) => Ok(Literal::U8(a.wrapping_mul(b))),
            (Literal::U16(a), Literal::U16(b)) => Ok(Literal::U16(a.wrapping_mul(b))),
            (Literal::U32(a), Literal::U32(b)) => Ok(Literal::U32(a.wrapping_mul(b))),
            (Literal::U64(a), Literal::U64(b)) => Ok(Literal::U64(a.wrapping_mul(b))),
            (Literal::U128(a), Literal::U128(b)) => Ok(Literal::U128(a.wrapping_mul(b))),
            (Literal::USize(a), Literal::USize(b)) => Ok(Literal::USize(a.wrapping_mul(b))),
            (Literal::F8(a), Literal::F8(b)) => Ok(Literal::F8(a.mul(b))),
            (Literal::F16(a), Literal::F16(b)) => Ok(Literal::F16(a.mul(b))),
            (Literal::F32(a), Literal::F32(b)) => Ok(Literal::F32(a.mul(b))),
            (Literal::F64(a), Literal::F64(b)) => Ok(Literal::F64(a.mul(b))),
            (Literal::F128(a), Literal::F128(b)) => Ok(Literal::F128(a.mul(b))),
            _ => Err(LiteralMulError::TypeError),
        }
    }
}

pub enum LiteralDivError {
    TypeError,
    DivisionByZero,
}

impl std::ops::Div for Literal {
    type Output = Result<Literal, LiteralDivError>;

    fn div(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::I8(a.wrapping_div(b))),
            },
            (Literal::I16(a), Literal::I16(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::I16(a.wrapping_div(b))),
            },
            (Literal::I32(a), Literal::I32(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::I32(a.wrapping_div(b))),
            },
            (Literal::I64(a), Literal::I64(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::I64(a.wrapping_div(b))),
            },
            (Literal::I128(a), Literal::I128(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::I128(a.wrapping_div(b))),
            },
            (Literal::ISize(a), Literal::ISize(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::ISize(a.wrapping_div(b))),
            },
            (Literal::U8(a), Literal::U8(b)) => a
                .checked_div(b)
                .map(Literal::U8)
                .ok_or(LiteralDivError::DivisionByZero),
            (Literal::U16(a), Literal::U16(b)) => a
                .checked_div(b)
                .map(Literal::U16)
                .ok_or(LiteralDivError::DivisionByZero),
            (Literal::U32(a), Literal::U32(b)) => a
                .checked_div(b)
                .map(Literal::U32)
                .ok_or(LiteralDivError::DivisionByZero),
            (Literal::U64(a), Literal::U64(b)) => a
                .checked_div(b)
                .map(Literal::U64)
                .ok_or(LiteralDivError::DivisionByZero),
            (Literal::U128(a), Literal::U128(b)) => a
                .checked_div(b)
                .map(Literal::U128)
                .ok_or(LiteralDivError::DivisionByZero),
            (Literal::USize(a), Literal::USize(b)) => a
                .checked_div(b)
                .map(Literal::USize)
                .ok_or(LiteralDivError::DivisionByZero),
            (Literal::F8(a), Literal::F8(b)) => Ok(Literal::F8(a.div(b))),
            (Literal::F16(a), Literal::F16(b)) => Ok(Literal::F16(a.div(b))),
            (Literal::F32(a), Literal::F32(b)) => Ok(Literal::F32(a.div(b))),
            (Literal::F64(a), Literal::F64(b)) => Ok(Literal::F64(a.div(b))),
            (Literal::F128(a), Literal::F128(b)) => Ok(Literal::F128(a.div(b))),
            _ => Err(LiteralDivError::TypeError),
        }
    }
}
