use crate::prelude::*;

#[derive(Debug)]
pub enum LiteralNegError {
    TypeError,
}

impl std::ops::Neg for Lit {
    type Output = Result<Lit, LiteralNegError>;

    #[inline(always)]
    fn neg(self) -> Self::Output {
        match self {
            Lit::I8(a) => Ok(Lit::I8(a.wrapping_neg())),
            Lit::I16(a) => Ok(Lit::I16(a.wrapping_neg())),
            Lit::I32(a) => Ok(Lit::I32(a.wrapping_neg())),
            Lit::I64(a) => Ok(Lit::I64(a.wrapping_neg())),
            Lit::I128(a) => Ok(Lit::I128(a.wrapping_neg())),
            Lit::U8(a) => Ok(Lit::U8(a.wrapping_neg())),
            Lit::U16(a) => Ok(Lit::U16(a.wrapping_neg())),
            Lit::U32(a) => Ok(Lit::U32(a.wrapping_neg())),
            Lit::U64(a) => Ok(Lit::U64(a.wrapping_neg())),
            Lit::U128(a) => Ok(Lit::U128(a.wrapping_neg())),
            Lit::F32(a) => Ok(Lit::F32(a.neg())),
            Lit::F64(a) => Ok(Lit::F64(a.neg())),
            Lit::USize32(a) => Ok(Lit::USize32(a.wrapping_neg())),
            Lit::USize64(a) => Ok(Lit::USize64(a.wrapping_neg())),
            Lit::Bool(_) | Lit::Unit => Err(LiteralNegError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralNotError {
    TypeError,
}

impl std::ops::Not for Lit {
    type Output = Result<Lit, LiteralNotError>;

    #[inline(always)]
    fn not(self) -> Self::Output {
        match self {
            Lit::Bool(b) => Ok(Lit::Bool(b.not())),
            Lit::I8(a) => Ok(Lit::I8(a.not())),
            Lit::I16(a) => Ok(Lit::I16(a.not())),
            Lit::I32(a) => Ok(Lit::I32(a.not())),
            Lit::I64(a) => Ok(Lit::I64(a.not())),
            Lit::I128(a) => Ok(Lit::I128(a.not())),
            Lit::U8(a) => Ok(Lit::U8(a.not())),
            Lit::U16(a) => Ok(Lit::U16(a.not())),
            Lit::U32(a) => Ok(Lit::U32(a.not())),
            Lit::U64(a) => Ok(Lit::U64(a.not())),
            Lit::U128(a) => Ok(Lit::U128(a.not())),
            Lit::USize32(a) => Ok(Lit::USize32(a.not())),
            Lit::USize64(a) => Ok(Lit::USize64(a.not())),
            Lit::F32(_) | Lit::F64(_) | Lit::Unit => Err(LiteralNotError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralAddError {
    TypeError,
}

impl std::ops::Add for Lit {
    type Output = Result<Lit, LiteralAddError>;

    #[inline(always)]
    fn add(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::I8(b)) => Ok(Lit::I8(a.wrapping_add(b))),
            (Lit::I16(a), Lit::I16(b)) => Ok(Lit::I16(a.wrapping_add(b))),
            (Lit::I32(a), Lit::I32(b)) => Ok(Lit::I32(a.wrapping_add(b))),
            (Lit::I64(a), Lit::I64(b)) => Ok(Lit::I64(a.wrapping_add(b))),
            (Lit::I128(a), Lit::I128(b)) => Ok(Lit::I128(a.wrapping_add(b))),
            (Lit::U8(a), Lit::U8(b)) => Ok(Lit::U8(a.wrapping_add(b))),
            (Lit::U16(a), Lit::U16(b)) => Ok(Lit::U16(a.wrapping_add(b))),
            (Lit::U32(a), Lit::U32(b)) => Ok(Lit::U32(a.wrapping_add(b))),
            (Lit::U64(a), Lit::U64(b)) => Ok(Lit::U64(a.wrapping_add(b))),
            (Lit::U128(a), Lit::U128(b)) => Ok(Lit::U128(a.wrapping_add(b))),
            (Lit::F32(a), Lit::F32(b)) => Ok(Lit::F32(a.add(b))),
            (Lit::F64(a), Lit::F64(b)) => Ok(Lit::F64(a.add(b))),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(Lit::USize32(a.wrapping_add(b))),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(Lit::USize64(a.wrapping_add(b))),
            _ => Err(LiteralAddError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralSubError {
    TypeError,
}

impl std::ops::Sub for Lit {
    type Output = Result<Lit, LiteralSubError>;

    #[inline(always)]
    fn sub(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::I8(b)) => Ok(Lit::I8(a.wrapping_sub(b))),
            (Lit::I16(a), Lit::I16(b)) => Ok(Lit::I16(a.wrapping_sub(b))),
            (Lit::I32(a), Lit::I32(b)) => Ok(Lit::I32(a.wrapping_sub(b))),
            (Lit::I64(a), Lit::I64(b)) => Ok(Lit::I64(a.wrapping_sub(b))),
            (Lit::I128(a), Lit::I128(b)) => Ok(Lit::I128(a.wrapping_sub(b))),
            (Lit::U8(a), Lit::U8(b)) => Ok(Lit::U8(a.wrapping_sub(b))),
            (Lit::U16(a), Lit::U16(b)) => Ok(Lit::U16(a.wrapping_sub(b))),
            (Lit::U32(a), Lit::U32(b)) => Ok(Lit::U32(a.wrapping_sub(b))),
            (Lit::U64(a), Lit::U64(b)) => Ok(Lit::U64(a.wrapping_sub(b))),
            (Lit::U128(a), Lit::U128(b)) => Ok(Lit::U128(a.wrapping_sub(b))),
            (Lit::F32(a), Lit::F32(b)) => Ok(Lit::F32(a.sub(b))),
            (Lit::F64(a), Lit::F64(b)) => Ok(Lit::F64(a.sub(b))),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(Lit::USize32(a.wrapping_sub(b))),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(Lit::USize64(a.wrapping_sub(b))),
            _ => Err(LiteralSubError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralMulError {
    TypeError,
}

impl std::ops::Mul for Lit {
    type Output = Result<Lit, LiteralMulError>;

    #[inline(always)]
    fn mul(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::I8(b)) => Ok(Lit::I8(a.wrapping_mul(b))),
            (Lit::I16(a), Lit::I16(b)) => Ok(Lit::I16(a.wrapping_mul(b))),
            (Lit::I32(a), Lit::I32(b)) => Ok(Lit::I32(a.wrapping_mul(b))),
            (Lit::I64(a), Lit::I64(b)) => Ok(Lit::I64(a.wrapping_mul(b))),
            (Lit::I128(a), Lit::I128(b)) => Ok(Lit::I128(a.wrapping_mul(b))),
            (Lit::U8(a), Lit::U8(b)) => Ok(Lit::U8(a.wrapping_mul(b))),
            (Lit::U16(a), Lit::U16(b)) => Ok(Lit::U16(a.wrapping_mul(b))),
            (Lit::U32(a), Lit::U32(b)) => Ok(Lit::U32(a.wrapping_mul(b))),
            (Lit::U64(a), Lit::U64(b)) => Ok(Lit::U64(a.wrapping_mul(b))),
            (Lit::U128(a), Lit::U128(b)) => Ok(Lit::U128(a.wrapping_mul(b))),
            (Lit::F32(a), Lit::F32(b)) => Ok(Lit::F32(a.mul(b))),
            (Lit::F64(a), Lit::F64(b)) => Ok(Lit::F64(a.mul(b))),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(Lit::USize32(a.wrapping_mul(b))),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(Lit::USize64(a.wrapping_mul(b))),
            _ => Err(LiteralMulError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralDivError {
    TypeError,
    DivisionByZero,
}

impl std::ops::Div for Lit {
    type Output = Result<Lit, LiteralDivError>;

    #[inline(always)]
    fn div(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::I8(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::I8(a.wrapping_div(b))),
            },
            (Lit::I16(a), Lit::I16(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::I16(a.wrapping_div(b))),
            },
            (Lit::I32(a), Lit::I32(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::I32(a.wrapping_div(b))),
            },
            (Lit::I64(a), Lit::I64(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::I64(a.wrapping_div(b))),
            },
            (Lit::I128(a), Lit::I128(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::I128(a.wrapping_div(b))),
            },
            (Lit::U8(a), Lit::U8(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::U8(a.wrapping_div(b))),
            },
            (Lit::U16(a), Lit::U16(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::U16(a.wrapping_div(b))),
            },
            (Lit::U32(a), Lit::U32(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::U32(a.wrapping_div(b))),
            },
            (Lit::U64(a), Lit::U64(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::U64(a.wrapping_div(b))),
            },
            (Lit::U128(a), Lit::U128(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::U128(a.wrapping_div(b))),
            },
            (Lit::F32(a), Lit::F32(b)) => Ok(Lit::F32(a.div(b))),
            (Lit::F64(a), Lit::F64(b)) => Ok(Lit::F64(a.div(b))),
            (Lit::USize32(a), Lit::USize32(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::USize32(a.wrapping_div(b))),
            },
            (Lit::USize64(a), Lit::USize64(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Lit::USize64(a.wrapping_div(b))),
            },
            _ => Err(LiteralDivError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralRemError {
    TypeError,
    ModuloByZero,
}

impl std::ops::Rem for Lit {
    type Output = Result<Lit, LiteralRemError>;

    #[inline(always)]
    fn rem(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::I8(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::I8(a.wrapping_rem(b))),
            },
            (Lit::I16(a), Lit::I16(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::I16(a.wrapping_rem(b))),
            },
            (Lit::I32(a), Lit::I32(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::I32(a.wrapping_rem(b))),
            },
            (Lit::I64(a), Lit::I64(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::I64(a.wrapping_rem(b))),
            },
            (Lit::I128(a), Lit::I128(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::I128(a.wrapping_rem(b))),
            },
            (Lit::U8(a), Lit::U8(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::U8(a.wrapping_rem(b))),
            },
            (Lit::U16(a), Lit::U16(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::U16(a.wrapping_rem(b))),
            },
            (Lit::U32(a), Lit::U32(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::U32(a.wrapping_rem(b))),
            },
            (Lit::U64(a), Lit::U64(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::U64(a.wrapping_rem(b))),
            },
            (Lit::U128(a), Lit::U128(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::U128(a.wrapping_rem(b))),
            },
            (Lit::F32(a), Lit::F32(b)) => Ok(Lit::F32(a.rem(b))),
            (Lit::F64(a), Lit::F64(b)) => Ok(Lit::F64(a.rem(b))),
            (Lit::USize32(a), Lit::USize32(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::USize32(a.wrapping_rem(b))),
            },
            (Lit::USize64(a), Lit::USize64(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Lit::USize64(a.wrapping_rem(b))),
            },
            _ => Err(LiteralRemError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralBitAndError {
    TypeError,
}

impl std::ops::BitAnd for Lit {
    type Output = Result<Lit, LiteralBitAndError>;

    #[inline(always)]
    fn bitand(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::I8(b)) => Ok(Lit::I8(a.bitand(b))),
            (Lit::I16(a), Lit::I16(b)) => Ok(Lit::I16(a.bitand(b))),
            (Lit::I32(a), Lit::I32(b)) => Ok(Lit::I32(a.bitand(b))),
            (Lit::I64(a), Lit::I64(b)) => Ok(Lit::I64(a.bitand(b))),
            (Lit::I128(a), Lit::I128(b)) => Ok(Lit::I128(a.bitand(b))),
            (Lit::U8(a), Lit::U8(b)) => Ok(Lit::U8(a.bitand(b))),
            (Lit::U16(a), Lit::U16(b)) => Ok(Lit::U16(a.bitand(b))),
            (Lit::U32(a), Lit::U32(b)) => Ok(Lit::U32(a.bitand(b))),
            (Lit::U64(a), Lit::U64(b)) => Ok(Lit::U64(a.bitand(b))),
            (Lit::U128(a), Lit::U128(b)) => Ok(Lit::U128(a.bitand(b))),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(Lit::USize32(a.bitand(b))),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(Lit::USize64(a.bitand(b))),
            _ => Err(LiteralBitAndError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralBitOrError {
    TypeError,
}

impl std::ops::BitOr for Lit {
    type Output = Result<Lit, LiteralBitOrError>;

    #[inline(always)]
    fn bitor(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::I8(b)) => Ok(Lit::I8(a.bitor(b))),
            (Lit::I16(a), Lit::I16(b)) => Ok(Lit::I16(a.bitor(b))),
            (Lit::I32(a), Lit::I32(b)) => Ok(Lit::I32(a.bitor(b))),
            (Lit::I64(a), Lit::I64(b)) => Ok(Lit::I64(a.bitor(b))),
            (Lit::I128(a), Lit::I128(b)) => Ok(Lit::I128(a.bitor(b))),
            (Lit::U8(a), Lit::U8(b)) => Ok(Lit::U8(a.bitor(b))),
            (Lit::U16(a), Lit::U16(b)) => Ok(Lit::U16(a.bitor(b))),
            (Lit::U32(a), Lit::U32(b)) => Ok(Lit::U32(a.bitor(b))),
            (Lit::U64(a), Lit::U64(b)) => Ok(Lit::U64(a.bitor(b))),
            (Lit::U128(a), Lit::U128(b)) => Ok(Lit::U128(a.bitor(b))),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(Lit::USize32(a.bitor(b))),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(Lit::USize64(a.bitor(b))),
            _ => Err(LiteralBitOrError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralBitXorError {
    TypeError,
}

impl std::ops::BitXor for Lit {
    type Output = Result<Lit, LiteralBitXorError>;

    #[inline(always)]
    fn bitxor(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::I8(b)) => Ok(Lit::I8(a.bitxor(b))),
            (Lit::I16(a), Lit::I16(b)) => Ok(Lit::I16(a.bitxor(b))),
            (Lit::I32(a), Lit::I32(b)) => Ok(Lit::I32(a.bitxor(b))),
            (Lit::I64(a), Lit::I64(b)) => Ok(Lit::I64(a.bitxor(b))),
            (Lit::I128(a), Lit::I128(b)) => Ok(Lit::I128(a.bitxor(b))),
            (Lit::U8(a), Lit::U8(b)) => Ok(Lit::U8(a.bitxor(b))),
            (Lit::U16(a), Lit::U16(b)) => Ok(Lit::U16(a.bitxor(b))),
            (Lit::U32(a), Lit::U32(b)) => Ok(Lit::U32(a.bitxor(b))),
            (Lit::U64(a), Lit::U64(b)) => Ok(Lit::U64(a.bitxor(b))),
            (Lit::U128(a), Lit::U128(b)) => Ok(Lit::U128(a.bitxor(b))),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(Lit::USize32(a.bitxor(b))),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(Lit::USize64(a.bitxor(b))),
            _ => Err(LiteralBitXorError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralShlError {
    TypeError,
    ShiftAmountError,
}

impl std::ops::Shl<Lit> for Lit {
    type Output = Result<Lit, LiteralShlError>;

    #[inline(always)]
    fn shl(self, rhs: Lit) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::I8(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::I16(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::I16(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::I32(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::I32(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::I64(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::I64(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::I128(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::I128(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::U8(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::U8(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::U16(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::U16(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::U32(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::U32(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::U64(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::U64(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::U128(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::U128(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::USize32(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::USize32(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Lit::USize64(a), Lit::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Lit::USize64(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            _ => Err(LiteralShlError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralShrError {
    TypeError,
    ShiftAmountError,
}

impl std::ops::Shr<Lit> for Lit {
    type Output = Result<Lit, LiteralShrError>;

    #[inline(always)]
    fn shr(self, rhs: Lit) -> Self::Output {
        match (self, rhs) {
            (Lit::I8(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::I8(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::I16(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::I16(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::I32(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::I32(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::I64(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::I64(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::I128(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::I128(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::U8(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::U8(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::U16(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::U16(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::U32(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::U32(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::U64(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::U64(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::U128(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::U128(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::USize32(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::USize32(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Lit::USize64(a), Lit::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Lit::USize64(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            _ => Err(LiteralShrError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralRolError {
    TypeError,
}

#[derive(Debug)]
pub enum LiteralRorError {
    TypeError,
}

impl Lit {
    #[inline(always)]
    pub fn rotate_left(self, rhs: Lit) -> Result<Lit, LiteralRolError> {
        // General formula: (lhs << (rhs % bits)) | (lhs >> (bits - (rhs % bits)))
        // Assume bits is 8, 16, 32, 64, or 128 depending on the type
        // (rhs % bits) = (rhs & (bits - 1)) since bits is a power of two

        // Let typeof(rhs) = T
        // typeof(bit_width) = T
        let bit_width = Lit::new_integer(&rhs, self.size_of()).unwrap();

        // typeof(one) = typeof(bit_width) = T
        let one = Lit::new_integer(&bit_width, 1).unwrap();

        // typeof(bit_width_minus_one) = T
        let bit_width_minus_one = (bit_width - one).unwrap();

        // typeof(shift_amount) = T
        let shift_amount = (rhs & bit_width_minus_one).unwrap();

        // let typeof(left) = U
        let left = (self << shift_amount).map_err(|_| LiteralRolError::TypeError)?;

        // typeof(right_shift_amount) = T
        let right_shift_amount = (bit_width - shift_amount).unwrap();

        // typeof(right) = U
        let right = (self >> right_shift_amount).map_err(|_| LiteralRolError::TypeError)?;

        // typeof(result) = U
        let result = (left | right).map_err(|_| LiteralRolError::TypeError)?;

        Ok(result)
    }

    #[inline(always)]
    pub fn rotate_right(self, rhs: Lit) -> Result<Lit, LiteralRorError> {
        // General formula: (lhs >> (rhs % bits)) | (lhs << (bits - (rhs % bits)))
        // Assume bits is 8, 16, 32, 64, or 128 depending on the type
        // (rhs % bits) = (rhs & (bits - 1)) since bits is a power of two

        // Let typeof(rhs) = T
        // typeof(bit_width) = T
        let bit_width = Lit::new_integer(&rhs, self.size_of()).unwrap();

        // typeof(one) = typeof(bit_width) = T
        let one = Lit::new_integer(&bit_width, 1).unwrap();

        // typeof(bit_width_minus_one) = T
        let bit_width_minus_one = (bit_width - one).unwrap();

        // typeof(shift_amount) = T
        let shift_amount = (rhs & bit_width_minus_one).unwrap();

        // let typeof(right) = U
        let right = (self >> shift_amount).map_err(|_| LiteralRorError::TypeError)?;

        // typeof(left_shift_amount) = T
        let left_shift_amount = (bit_width - shift_amount).unwrap();

        // typeof(left) = U
        let left = (self << left_shift_amount).map_err(|_| LiteralRorError::TypeError)?;

        // typeof(result) = U
        let result = (left | right).map_err(|_| LiteralRorError::TypeError)?;

        Ok(result)
    }
}

#[derive(Debug)]
pub enum LiteralCmpError {
    TypeError,
}

impl Lit {
    #[inline(always)]
    pub fn lt(&self, other: &Self) -> Result<bool, LiteralCmpError> {
        match (self, other) {
            (Lit::Unit, Lit::Unit) => Ok(false),
            (Lit::Bool(a), Lit::Bool(b)) => Ok(a.lt(b)),
            (Lit::I8(a), Lit::I8(b)) => Ok(a.lt(b)),
            (Lit::I16(a), Lit::I16(b)) => Ok(a.lt(b)),
            (Lit::I32(a), Lit::I32(b)) => Ok(a.lt(b)),
            (Lit::I64(a), Lit::I64(b)) => Ok(a.lt(b)),
            (Lit::I128(a), Lit::I128(b)) => Ok(a.lt(b)),
            (Lit::U8(a), Lit::U8(b)) => Ok(a.lt(b)),
            (Lit::U16(a), Lit::U16(b)) => Ok(a.lt(b)),
            (Lit::U32(a), Lit::U32(b)) => Ok(a.lt(b)),
            (Lit::U64(a), Lit::U64(b)) => Ok(a.lt(b)),
            (Lit::U128(a), Lit::U128(b)) => Ok(a.lt(b)),
            (Lit::F32(a), Lit::F32(b)) => Ok(a.lt(b)),
            (Lit::F64(a), Lit::F64(b)) => Ok(a.lt(b)),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(a.lt(b)),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(a.lt(b)),
            _ => Err(LiteralCmpError::TypeError),
        }
    }

    #[inline(always)]
    pub fn gt(&self, other: &Self) -> Result<bool, LiteralCmpError> {
        match (self, other) {
            (Lit::Unit, Lit::Unit) => Ok(false),
            (Lit::Bool(a), Lit::Bool(b)) => Ok(a.gt(b)),
            (Lit::I8(a), Lit::I8(b)) => Ok(a.gt(b)),
            (Lit::I16(a), Lit::I16(b)) => Ok(a.gt(b)),
            (Lit::I32(a), Lit::I32(b)) => Ok(a.gt(b)),
            (Lit::I64(a), Lit::I64(b)) => Ok(a.gt(b)),
            (Lit::I128(a), Lit::I128(b)) => Ok(a.gt(b)),
            (Lit::U8(a), Lit::U8(b)) => Ok(a.gt(b)),
            (Lit::U16(a), Lit::U16(b)) => Ok(a.gt(b)),
            (Lit::U32(a), Lit::U32(b)) => Ok(a.gt(b)),
            (Lit::U64(a), Lit::U64(b)) => Ok(a.gt(b)),
            (Lit::U128(a), Lit::U128(b)) => Ok(a.gt(b)),
            (Lit::F32(a), Lit::F32(b)) => Ok(a.gt(b)),
            (Lit::F64(a), Lit::F64(b)) => Ok(a.gt(b)),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(a.gt(b)),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(a.gt(b)),
            _ => Err(LiteralCmpError::TypeError),
        }
    }

    #[inline(always)]
    pub fn le(&self, other: &Self) -> Result<bool, LiteralCmpError> {
        match (self, other) {
            (Lit::Unit, Lit::Unit) => Ok(true),
            (Lit::Bool(a), Lit::Bool(b)) => Ok(a.le(b)),
            (Lit::I8(a), Lit::I8(b)) => Ok(a.le(b)),
            (Lit::I16(a), Lit::I16(b)) => Ok(a.le(b)),
            (Lit::I32(a), Lit::I32(b)) => Ok(a.le(b)),
            (Lit::I64(a), Lit::I64(b)) => Ok(a.le(b)),
            (Lit::I128(a), Lit::I128(b)) => Ok(a.le(b)),
            (Lit::U8(a), Lit::U8(b)) => Ok(a.le(b)),
            (Lit::U16(a), Lit::U16(b)) => Ok(a.le(b)),
            (Lit::U32(a), Lit::U32(b)) => Ok(a.le(b)),
            (Lit::U64(a), Lit::U64(b)) => Ok(a.le(b)),
            (Lit::U128(a), Lit::U128(b)) => Ok(a.le(b)),
            (Lit::F32(a), Lit::F32(b)) => Ok(a.le(b)),
            (Lit::F64(a), Lit::F64(b)) => Ok(a.le(b)),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(a.le(b)),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(a.le(b)),
            _ => Err(LiteralCmpError::TypeError),
        }
    }

    #[inline(always)]
    pub fn ge(&self, other: &Self) -> Result<bool, LiteralCmpError> {
        match (self, other) {
            (Lit::Unit, Lit::Unit) => Ok(true),
            (Lit::Bool(a), Lit::Bool(b)) => Ok(a.ge(b)),
            (Lit::I8(a), Lit::I8(b)) => Ok(a.ge(b)),
            (Lit::I16(a), Lit::I16(b)) => Ok(a.ge(b)),
            (Lit::I32(a), Lit::I32(b)) => Ok(a.ge(b)),
            (Lit::I64(a), Lit::I64(b)) => Ok(a.ge(b)),
            (Lit::I128(a), Lit::I128(b)) => Ok(a.ge(b)),
            (Lit::U8(a), Lit::U8(b)) => Ok(a.ge(b)),
            (Lit::U16(a), Lit::U16(b)) => Ok(a.ge(b)),
            (Lit::U32(a), Lit::U32(b)) => Ok(a.ge(b)),
            (Lit::U64(a), Lit::U64(b)) => Ok(a.ge(b)),
            (Lit::U128(a), Lit::U128(b)) => Ok(a.ge(b)),
            (Lit::F32(a), Lit::F32(b)) => Ok(a.ge(b)),
            (Lit::F64(a), Lit::F64(b)) => Ok(a.ge(b)),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(a.ge(b)),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(a.ge(b)),
            _ => Err(LiteralCmpError::TypeError),
        }
    }

    #[inline(always)]
    pub fn eq(&self, other: &Self) -> Result<bool, LiteralCmpError> {
        match (self, other) {
            (Lit::Unit, Lit::Unit) => Ok(true),
            (Lit::Bool(a), Lit::Bool(b)) => Ok(a.eq(b)),
            (Lit::I8(a), Lit::I8(b)) => Ok(a.eq(b)),
            (Lit::I16(a), Lit::I16(b)) => Ok(a.eq(b)),
            (Lit::I32(a), Lit::I32(b)) => Ok(a.eq(b)),
            (Lit::I64(a), Lit::I64(b)) => Ok(a.eq(b)),
            (Lit::I128(a), Lit::I128(b)) => Ok(a.eq(b)),
            (Lit::U8(a), Lit::U8(b)) => Ok(a.eq(b)),
            (Lit::U16(a), Lit::U16(b)) => Ok(a.eq(b)),
            (Lit::U32(a), Lit::U32(b)) => Ok(a.eq(b)),
            (Lit::U64(a), Lit::U64(b)) => Ok(a.eq(b)),
            (Lit::U128(a), Lit::U128(b)) => Ok(a.eq(b)),
            (Lit::F32(a), Lit::F32(b)) => Ok(a.eq(b) || (a.is_nan() && b.is_nan())),
            (Lit::F64(a), Lit::F64(b)) => Ok(a.eq(b) || (a.is_nan() && b.is_nan())),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(a.eq(b)),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(a.eq(b)),
            _ => Err(LiteralCmpError::TypeError),
        }
    }

    #[inline(always)]
    pub fn ne(&self, other: &Self) -> Result<bool, LiteralCmpError> {
        match (self, other) {
            (Lit::Unit, Lit::Unit) => Ok(false),
            (Lit::Bool(a), Lit::Bool(b)) => Ok(a.ne(b)),
            (Lit::I8(a), Lit::I8(b)) => Ok(a.ne(b)),
            (Lit::I16(a), Lit::I16(b)) => Ok(a.ne(b)),
            (Lit::I32(a), Lit::I32(b)) => Ok(a.ne(b)),
            (Lit::I64(a), Lit::I64(b)) => Ok(a.ne(b)),
            (Lit::I128(a), Lit::I128(b)) => Ok(a.ne(b)),
            (Lit::U8(a), Lit::U8(b)) => Ok(a.ne(b)),
            (Lit::U16(a), Lit::U16(b)) => Ok(a.ne(b)),
            (Lit::U32(a), Lit::U32(b)) => Ok(a.ne(b)),
            (Lit::U64(a), Lit::U64(b)) => Ok(a.ne(b)),
            (Lit::U128(a), Lit::U128(b)) => Ok(a.ne(b)),
            (Lit::F32(a), Lit::F32(b)) => Ok(a.ne(b) && !(a.is_nan() && b.is_nan())),
            (Lit::F64(a), Lit::F64(b)) => Ok(a.ne(b) && !(a.is_nan() && b.is_nan())),
            (Lit::USize32(a), Lit::USize32(b)) => Ok(a.ne(b)),
            (Lit::USize64(a), Lit::USize64(b)) => Ok(a.ne(b)),
            _ => Err(LiteralCmpError::TypeError),
        }
    }

    #[inline(always)]
    pub fn logical_and(self, other: Self) -> Result<bool, LiteralCmpError> {
        match (self, other) {
            (Lit::Bool(a), Lit::Bool(b)) => Ok(a && b),
            _ => Err(LiteralCmpError::TypeError),
        }
    }

    #[inline(always)]
    pub fn logical_or(self, other: Self) -> Result<bool, LiteralCmpError> {
        match (self, other) {
            (Lit::Bool(a), Lit::Bool(b)) => Ok(a || b),
            _ => Err(LiteralCmpError::TypeError),
        }
    }

    #[inline(always)]
    pub fn logical_xor(self, other: Self) -> Result<bool, LiteralCmpError> {
        match (self, other) {
            (Lit::Bool(a), Lit::Bool(b)) => Ok(a ^ b),
            _ => Err(LiteralCmpError::TypeError),
        }
    }
}
