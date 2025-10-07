use crate::hir::Literal;

#[derive(Debug)]
pub enum LiteralNegError {
    TypeError,
}

impl std::ops::Neg for Literal {
    type Output = Result<Literal, LiteralNegError>;

    #[inline(always)]
    fn neg(self) -> Self::Output {
        match self {
            Literal::I8(a) => Ok(Literal::I8(a.wrapping_neg())),
            Literal::I16(a) => Ok(Literal::I16(a.wrapping_neg())),
            Literal::I32(a) => Ok(Literal::I32(a.wrapping_neg())),
            Literal::I64(a) => Ok(Literal::I64(a.wrapping_neg())),
            Literal::I128(a) => Ok(Literal::I128(a.wrapping_neg())),
            Literal::U8(a) => Ok(Literal::U8(a.wrapping_neg())),
            Literal::U16(a) => Ok(Literal::U16(a.wrapping_neg())),
            Literal::U32(a) => Ok(Literal::U32(a.wrapping_neg())),
            Literal::U64(a) => Ok(Literal::U64(a.wrapping_neg())),
            Literal::U128(a) => Ok(Literal::U128(a.wrapping_neg())),
            Literal::F8(a) => Ok(Literal::F8(a.neg())),
            Literal::F16(a) => Ok(Literal::F16(a.neg())),
            Literal::F32(a) => Ok(Literal::F32(a.neg())),
            Literal::F64(a) => Ok(Literal::F64(a.neg())),
            Literal::F128(a) => Ok(Literal::F128(a.neg())),
            Literal::Bool(_) | Literal::Unit => Err(LiteralNegError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralNotError {
    TypeError,
}

impl std::ops::Not for Literal {
    type Output = Result<Literal, LiteralNotError>;

    #[inline(always)]
    fn not(self) -> Self::Output {
        match self {
            Literal::Bool(b) => Ok(Literal::Bool(b.not())),
            Literal::I8(a) => Ok(Literal::I8(a.not())),
            Literal::I16(a) => Ok(Literal::I16(a.not())),
            Literal::I32(a) => Ok(Literal::I32(a.not())),
            Literal::I64(a) => Ok(Literal::I64(a.not())),
            Literal::I128(a) => Ok(Literal::I128(a.not())),
            Literal::U8(a) => Ok(Literal::U8(a.not())),
            Literal::U16(a) => Ok(Literal::U16(a.not())),
            Literal::U32(a) => Ok(Literal::U32(a.not())),
            Literal::U64(a) => Ok(Literal::U64(a.not())),
            Literal::U128(a) => Ok(Literal::U128(a.not())),
            Literal::F8(_)
            | Literal::F16(_)
            | Literal::F32(_)
            | Literal::F64(_)
            | Literal::F128(_)
            | Literal::Unit => Err(LiteralNotError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralAddError {
    TypeError,
}

impl std::ops::Add for Literal {
    type Output = Result<Literal, LiteralAddError>;

    #[inline(always)]
    fn add(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => Ok(Literal::I8(a.wrapping_add(b))),
            (Literal::I16(a), Literal::I16(b)) => Ok(Literal::I16(a.wrapping_add(b))),
            (Literal::I32(a), Literal::I32(b)) => Ok(Literal::I32(a.wrapping_add(b))),
            (Literal::I64(a), Literal::I64(b)) => Ok(Literal::I64(a.wrapping_add(b))),
            (Literal::I128(a), Literal::I128(b)) => Ok(Literal::I128(a.wrapping_add(b))),
            (Literal::U8(a), Literal::U8(b)) => Ok(Literal::U8(a.wrapping_add(b))),
            (Literal::U16(a), Literal::U16(b)) => Ok(Literal::U16(a.wrapping_add(b))),
            (Literal::U32(a), Literal::U32(b)) => Ok(Literal::U32(a.wrapping_add(b))),
            (Literal::U64(a), Literal::U64(b)) => Ok(Literal::U64(a.wrapping_add(b))),
            (Literal::U128(a), Literal::U128(b)) => Ok(Literal::U128(a.wrapping_add(b))),
            (Literal::F8(a), Literal::F8(b)) => Ok(Literal::F8(a.add(b))),
            (Literal::F16(a), Literal::F16(b)) => Ok(Literal::F16(a.add(b))),
            (Literal::F32(a), Literal::F32(b)) => Ok(Literal::F32(a.add(b))),
            (Literal::F64(a), Literal::F64(b)) => Ok(Literal::F64(a.add(b))),
            (Literal::F128(a), Literal::F128(b)) => Ok(Literal::F128(a.add(b))),
            _ => Err(LiteralAddError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralSubError {
    TypeError,
}

impl std::ops::Sub for Literal {
    type Output = Result<Literal, LiteralSubError>;

    #[inline(always)]
    fn sub(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => Ok(Literal::I8(a.wrapping_sub(b))),
            (Literal::I16(a), Literal::I16(b)) => Ok(Literal::I16(a.wrapping_sub(b))),
            (Literal::I32(a), Literal::I32(b)) => Ok(Literal::I32(a.wrapping_sub(b))),
            (Literal::I64(a), Literal::I64(b)) => Ok(Literal::I64(a.wrapping_sub(b))),
            (Literal::I128(a), Literal::I128(b)) => Ok(Literal::I128(a.wrapping_sub(b))),
            (Literal::U8(a), Literal::U8(b)) => Ok(Literal::U8(a.wrapping_sub(b))),
            (Literal::U16(a), Literal::U16(b)) => Ok(Literal::U16(a.wrapping_sub(b))),
            (Literal::U32(a), Literal::U32(b)) => Ok(Literal::U32(a.wrapping_sub(b))),
            (Literal::U64(a), Literal::U64(b)) => Ok(Literal::U64(a.wrapping_sub(b))),
            (Literal::U128(a), Literal::U128(b)) => Ok(Literal::U128(a.wrapping_sub(b))),
            (Literal::F8(a), Literal::F8(b)) => Ok(Literal::F8(a.sub(b))),
            (Literal::F16(a), Literal::F16(b)) => Ok(Literal::F16(a.sub(b))),
            (Literal::F32(a), Literal::F32(b)) => Ok(Literal::F32(a.sub(b))),
            (Literal::F64(a), Literal::F64(b)) => Ok(Literal::F64(a.sub(b))),
            (Literal::F128(a), Literal::F128(b)) => Ok(Literal::F128(a.sub(b))),
            _ => Err(LiteralSubError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralMulError {
    TypeError,
}

impl std::ops::Mul for Literal {
    type Output = Result<Literal, LiteralMulError>;

    #[inline(always)]
    fn mul(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => Ok(Literal::I8(a.wrapping_mul(b))),
            (Literal::I16(a), Literal::I16(b)) => Ok(Literal::I16(a.wrapping_mul(b))),
            (Literal::I32(a), Literal::I32(b)) => Ok(Literal::I32(a.wrapping_mul(b))),
            (Literal::I64(a), Literal::I64(b)) => Ok(Literal::I64(a.wrapping_mul(b))),
            (Literal::I128(a), Literal::I128(b)) => Ok(Literal::I128(a.wrapping_mul(b))),
            (Literal::U8(a), Literal::U8(b)) => Ok(Literal::U8(a.wrapping_mul(b))),
            (Literal::U16(a), Literal::U16(b)) => Ok(Literal::U16(a.wrapping_mul(b))),
            (Literal::U32(a), Literal::U32(b)) => Ok(Literal::U32(a.wrapping_mul(b))),
            (Literal::U64(a), Literal::U64(b)) => Ok(Literal::U64(a.wrapping_mul(b))),
            (Literal::U128(a), Literal::U128(b)) => Ok(Literal::U128(a.wrapping_mul(b))),
            (Literal::F8(a), Literal::F8(b)) => Ok(Literal::F8(a.mul(b))),
            (Literal::F16(a), Literal::F16(b)) => Ok(Literal::F16(a.mul(b))),
            (Literal::F32(a), Literal::F32(b)) => Ok(Literal::F32(a.mul(b))),
            (Literal::F64(a), Literal::F64(b)) => Ok(Literal::F64(a.mul(b))),
            (Literal::F128(a), Literal::F128(b)) => Ok(Literal::F128(a.mul(b))),
            _ => Err(LiteralMulError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralDivError {
    TypeError,
    DivisionByZero,
}

impl std::ops::Div for Literal {
    type Output = Result<Literal, LiteralDivError>;

    #[inline(always)]
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
            (Literal::U8(a), Literal::U8(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::U8(a.wrapping_div(b))),
            },
            (Literal::U16(a), Literal::U16(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::U16(a.wrapping_div(b))),
            },
            (Literal::U32(a), Literal::U32(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::U32(a.wrapping_div(b))),
            },
            (Literal::U64(a), Literal::U64(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::U64(a.wrapping_div(b))),
            },
            (Literal::U128(a), Literal::U128(b)) => match b {
                0 => Err(LiteralDivError::DivisionByZero),
                _ => Ok(Literal::U128(a.wrapping_div(b))),
            },
            (Literal::F8(a), Literal::F8(b)) => Ok(Literal::F8(a.div(b))),
            (Literal::F16(a), Literal::F16(b)) => Ok(Literal::F16(a.div(b))),
            (Literal::F32(a), Literal::F32(b)) => Ok(Literal::F32(a.div(b))),
            (Literal::F64(a), Literal::F64(b)) => Ok(Literal::F64(a.div(b))),
            (Literal::F128(a), Literal::F128(b)) => Ok(Literal::F128(a.div(b))),
            _ => Err(LiteralDivError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralRemError {
    TypeError,
    ModuloByZero,
}

impl std::ops::Rem for Literal {
    type Output = Result<Literal, LiteralRemError>;

    #[inline(always)]
    fn rem(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::I8(a.wrapping_rem(b))),
            },
            (Literal::I16(a), Literal::I16(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::I16(a.wrapping_rem(b))),
            },
            (Literal::I32(a), Literal::I32(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::I32(a.wrapping_rem(b))),
            },
            (Literal::I64(a), Literal::I64(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::I64(a.wrapping_rem(b))),
            },
            (Literal::I128(a), Literal::I128(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::I128(a.wrapping_rem(b))),
            },
            (Literal::U8(a), Literal::U8(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::U8(a.wrapping_rem(b))),
            },
            (Literal::U16(a), Literal::U16(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::U16(a.wrapping_rem(b))),
            },
            (Literal::U32(a), Literal::U32(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::U32(a.wrapping_rem(b))),
            },
            (Literal::U64(a), Literal::U64(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::U64(a.wrapping_rem(b))),
            },
            (Literal::U128(a), Literal::U128(b)) => match b {
                0 => Err(LiteralRemError::ModuloByZero),
                _ => Ok(Literal::U128(a.wrapping_rem(b))),
            },
            (Literal::F8(a), Literal::F8(b)) => Ok(Literal::F8(a.rem(b))),
            (Literal::F16(a), Literal::F16(b)) => Ok(Literal::F16(a.rem(b))),
            (Literal::F32(a), Literal::F32(b)) => Ok(Literal::F32(a.rem(b))),
            (Literal::F64(a), Literal::F64(b)) => Ok(Literal::F64(a.rem(b))),
            (Literal::F128(a), Literal::F128(b)) => Ok(Literal::F128(a.rem(b))),
            _ => Err(LiteralRemError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralBitAndError {
    TypeError,
}

impl std::ops::BitAnd for Literal {
    type Output = Result<Literal, LiteralBitAndError>;

    #[inline(always)]
    fn bitand(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => Ok(Literal::I8(a.bitand(b))),
            (Literal::I16(a), Literal::I16(b)) => Ok(Literal::I16(a.bitand(b))),
            (Literal::I32(a), Literal::I32(b)) => Ok(Literal::I32(a.bitand(b))),
            (Literal::I64(a), Literal::I64(b)) => Ok(Literal::I64(a.bitand(b))),
            (Literal::I128(a), Literal::I128(b)) => Ok(Literal::I128(a.bitand(b))),
            (Literal::U8(a), Literal::U8(b)) => Ok(Literal::U8(a.bitand(b))),
            (Literal::U16(a), Literal::U16(b)) => Ok(Literal::U16(a.bitand(b))),
            (Literal::U32(a), Literal::U32(b)) => Ok(Literal::U32(a.bitand(b))),
            (Literal::U64(a), Literal::U64(b)) => Ok(Literal::U64(a.bitand(b))),
            (Literal::U128(a), Literal::U128(b)) => Ok(Literal::U128(a.bitand(b))),
            _ => Err(LiteralBitAndError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralBitOrError {
    TypeError,
}

impl std::ops::BitOr for Literal {
    type Output = Result<Literal, LiteralBitOrError>;

    #[inline(always)]
    fn bitor(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => Ok(Literal::I8(a.bitor(b))),
            (Literal::I16(a), Literal::I16(b)) => Ok(Literal::I16(a.bitor(b))),
            (Literal::I32(a), Literal::I32(b)) => Ok(Literal::I32(a.bitor(b))),
            (Literal::I64(a), Literal::I64(b)) => Ok(Literal::I64(a.bitor(b))),
            (Literal::I128(a), Literal::I128(b)) => Ok(Literal::I128(a.bitor(b))),
            (Literal::U8(a), Literal::U8(b)) => Ok(Literal::U8(a.bitor(b))),
            (Literal::U16(a), Literal::U16(b)) => Ok(Literal::U16(a.bitor(b))),
            (Literal::U32(a), Literal::U32(b)) => Ok(Literal::U32(a.bitor(b))),
            (Literal::U64(a), Literal::U64(b)) => Ok(Literal::U64(a.bitor(b))),
            (Literal::U128(a), Literal::U128(b)) => Ok(Literal::U128(a.bitor(b))),
            _ => Err(LiteralBitOrError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralBitXorError {
    TypeError,
}

impl std::ops::BitXor for Literal {
    type Output = Result<Literal, LiteralBitXorError>;

    #[inline(always)]
    fn bitxor(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::I8(b)) => Ok(Literal::I8(a.bitxor(b))),
            (Literal::I16(a), Literal::I16(b)) => Ok(Literal::I16(a.bitxor(b))),
            (Literal::I32(a), Literal::I32(b)) => Ok(Literal::I32(a.bitxor(b))),
            (Literal::I64(a), Literal::I64(b)) => Ok(Literal::I64(a.bitxor(b))),
            (Literal::I128(a), Literal::I128(b)) => Ok(Literal::I128(a.bitxor(b))),
            (Literal::U8(a), Literal::U8(b)) => Ok(Literal::U8(a.bitxor(b))),
            (Literal::U16(a), Literal::U16(b)) => Ok(Literal::U16(a.bitxor(b))),
            (Literal::U32(a), Literal::U32(b)) => Ok(Literal::U32(a.bitxor(b))),
            (Literal::U64(a), Literal::U64(b)) => Ok(Literal::U64(a.bitxor(b))),
            (Literal::U128(a), Literal::U128(b)) => Ok(Literal::U128(a.bitxor(b))),
            _ => Err(LiteralBitXorError::TypeError),
        }
    }
}

#[derive(Debug)]
pub enum LiteralShlError {
    TypeError,
    ShiftAmountError,
}

impl std::ops::Shl<Literal> for Literal {
    type Output = Result<Literal, LiteralShlError>;

    #[inline(always)]
    fn shl(self, rhs: Literal) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::I8(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Literal::I16(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::I16(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Literal::I32(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::I32(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Literal::I64(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::I64(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Literal::I128(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::I128(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Literal::U8(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::U8(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Literal::U16(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::U16(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Literal::U32(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::U32(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Literal::U64(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::U64(res)),
                None => Err(LiteralShlError::ShiftAmountError),
            },
            (Literal::U128(a), Literal::U32(b)) => match a.checked_shl(b) {
                Some(res) => Ok(Literal::U128(res)),
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

impl std::ops::Shr<Literal> for Literal {
    type Output = Result<Literal, LiteralShrError>;

    #[inline(always)]
    fn shr(self, rhs: Literal) -> Self::Output {
        match (self, rhs) {
            (Literal::I8(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::I8(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Literal::I16(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::I16(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Literal::I32(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::I32(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Literal::I64(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::I64(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Literal::I128(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::I128(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Literal::U8(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::U8(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Literal::U16(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::U16(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Literal::U32(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::U32(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Literal::U64(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::U64(res)),
                None => Err(LiteralShrError::ShiftAmountError),
            },
            (Literal::U128(a), Literal::U32(b)) => match a.checked_shr(b) {
                Some(res) => Ok(Literal::U128(res)),
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

pub enum LiteralRorError {
    TypeError,
}

impl Literal {
    #[inline(always)]
    pub fn rotate_left(self, rhs: Literal) -> Result<Literal, LiteralRolError> {
        // General formula: (lhs << (rhs % bits)) | (lhs >> (bits - (rhs % bits)))
        // Assume bits is 8, 16, 32, 64, or 128 depending on the type
        // (rhs % bits) = (rhs & (bits - 1)) since bits is a power of two

        // Let typeof(rhs) = T
        // typeof(bit_width) = T
        let bit_width = Literal::new_integer(&rhs, self.size_of()).unwrap();

        // typeof(one) = typeof(bit_width) = T
        let one = Literal::new_integer(&bit_width, 1).unwrap();

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
    pub fn rotate_right(self, rhs: Literal) -> Result<Literal, LiteralRorError> {
        // General formula: (lhs >> (rhs % bits)) | (lhs << (bits - (rhs % bits)))
        // Assume bits is 8, 16, 32, 64, or 128 depending on the type
        // (rhs % bits) = (rhs & (bits - 1)) since bits is a power of two

        // Let typeof(rhs) = T
        // typeof(bit_width) = T
        let bit_width = Literal::new_integer(&rhs, self.size_of()).unwrap();

        // typeof(one) = typeof(bit_width) = T
        let one = Literal::new_integer(&bit_width, 1).unwrap();

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
