use super::helpers::*;
use nitrate_tree::ast::*;

fn check_binop(src: &str, expected: BinExprOp) {
    assert!(
        matches!(&parse_expr(src), Expr::BinExpr(b) if b.operator == expected),
        "Expected {:?} for {src}",
        expected
    );
}

#[test]
fn test_binop_add() {
    check_binop("1 + 2", BinExprOp::Add);
}

#[test]
fn test_binop_sub() {
    check_binop("1 - 2", BinExprOp::Sub);
}

#[test]
fn test_binop_mul() {
    check_binop("1 * 2", BinExprOp::Mul);
}

#[test]
fn test_binop_div() {
    check_binop("1 / 2", BinExprOp::Div);
}

#[test]
fn test_binop_mod() {
    check_binop("1 % 2", BinExprOp::Mod);
}

#[test]
fn test_binop_bitand() {
    check_binop("1 & 2", BinExprOp::BitAnd);
}

#[test]
fn test_binop_bitor() {
    check_binop("1 | 2", BinExprOp::BitOr);
}

#[test]
fn test_binop_bitxor() {
    check_binop("1 ^ 2", BinExprOp::BitXor);
}

#[test]
fn test_binop_shl() {
    check_binop("1 << 2", BinExprOp::BitShl);
}

#[test]
fn test_binop_shr() {
    check_binop("1 >> 2", BinExprOp::BitShr);
}

#[test]
fn test_binop_rol() {
    check_binop("1 <<< 2", BinExprOp::BitRol);
}

#[test]
fn test_binop_ror() {
    check_binop("1 >>> 2", BinExprOp::BitRor);
}

#[test]
fn test_binop_and() {
    check_binop("true && false", BinExprOp::LogicAnd);
}

#[test]
fn test_binop_or() {
    check_binop("true || false", BinExprOp::LogicOr);
}

#[test]
fn test_binop_eq() {
    check_binop("1 == 2", BinExprOp::LogicEq);
}

#[test]
fn test_binop_lt() {
    check_binop("1 < 2", BinExprOp::LogicLt);
}

#[test]
fn test_binop_gt() {
    check_binop("1 > 2", BinExprOp::LogicGt);
}

#[test]
fn test_binop_le() {
    check_binop("1 <= 2", BinExprOp::LogicLe);
}

#[test]
fn test_binop_ge() {
    check_binop("1 >= 2", BinExprOp::LogicGe);
}

#[test]
fn test_binop_assign() {
    check_binop("x = 42", BinExprOp::Set);
}

#[test]
fn test_binop_add_eq() {
    check_binop("x += 1", BinExprOp::SetPlus);
}

#[test]
fn test_binop_sub_eq() {
    check_binop("x -= 1", BinExprOp::SetMinus);
}

#[test]
fn test_binop_mul_eq() {
    check_binop("x *= 2", BinExprOp::SetTimes);
}

#[test]
fn test_binop_div_eq() {
    check_binop("x /= 2", BinExprOp::SetSlash);
}

#[test]
fn test_binop_range() {
    check_binop("0..10", BinExprOp::Range);
}

#[test]
fn test_set_percent() {
    check_binop("x %= 1", BinExprOp::SetPercent);
}

#[test]
fn test_set_bitand() {
    check_binop("x &= 1", BinExprOp::SetBitAnd);
}

#[test]
fn test_set_logicand() {
    let expr = parse_expr("true &&= false");
    assert!(matches!(&expr, Expr::BinExpr(b) if b.operator == BinExprOp::SetLogicAnd));
}

#[test]
fn test_set_logicor() {
    let expr = parse_expr("true ||= false");
    assert!(matches!(&expr, Expr::BinExpr(b) if b.operator == BinExprOp::SetLogicOr));
}

#[test]
fn test_set_shl() {
    check_binop("x <<= 1", BinExprOp::SetBitShl);
}

#[test]
fn test_set_shr() {
    check_binop("x >>= 1", BinExprOp::SetBitShr);
}

#[test]
fn test_set_rol() {
    check_binop("x <<<= 1", BinExprOp::SetBitRotl);
}

#[test]
fn test_set_ror() {
    check_binop("x >>>= 1", BinExprOp::SetBitRotr);
}

#[test]
fn test_set_xor() {
    check_binop("x ^= 1", BinExprOp::SetBitXor);
}

#[test]
fn test_set_or() {
    check_binop("x |= 1", BinExprOp::SetBitOr);
}

#[test]
fn test_binop_not_range() {
    // `..` without left side should NOT parse as range, just dot dot
    let expr = parse_expr("x..y");
    assert!(matches!(&expr, Expr::BinExpr(b) if b.operator == BinExprOp::Range));
}
