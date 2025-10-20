use std::{any::type_name, cell::Ref};

use crate::lower::Ast2Hir;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_source::{Order, ParseTreeIter, RefNode, ast};

fn visit_node(node: RefNode, ctx: &mut HirCtx, log: &CompilerLog) {
    // TODO: implement

    match node {
        RefNode::ExprSyntaxError
        | RefNode::ExprParentheses(_)
        | RefNode::ExprBooleanLit(_)
        | RefNode::ExprIntegerLit(_)
        | RefNode::ExprFloatLit(_)
        | RefNode::ExprStringLit(_)
        | RefNode::ExprBStringLit(_)
        | RefNode::ExprTypeInfo(_)
        | RefNode::ExprList(_)
        | RefNode::ExprTuple(_)
        | RefNode::ExprStructInit(_)
        | RefNode::ExprUnaryExpr(_)
        | RefNode::ExprBinExpr(_)
        | RefNode::ExprCast(_)
        | RefNode::ExprBlockItem(_)
        | RefNode::ExprBlock(_)
        | RefNode::ExprAttributeList(_)
        | RefNode::ExprClosure(_)
        | RefNode::ExprPathTypeArgument(_)
        | RefNode::ExprPath(_)
        | RefNode::ExprIndexAccess(_)
        | RefNode::ExprIf(_)
        | RefNode::ExprWhile(_)
        | RefNode::ExprMatchCase(_)
        | RefNode::ExprMatch(_)
        | RefNode::ExprBreak(_)
        | RefNode::ExprContinue(_)
        | RefNode::ExprReturn(_)
        | RefNode::ExprFor(_)
        | RefNode::ExprAwait(_)
        | RefNode::ExprCallArgument(_)
        | RefNode::ExprFunctionCall(_)
        | RefNode::ExprMethodCall(_)
        | RefNode::TypeSyntaxError
        | RefNode::TypeBool
        | RefNode::TypeUInt8
        | RefNode::TypeUInt16
        | RefNode::TypeUInt32
        | RefNode::TypeUInt64
        | RefNode::TypeUInt128
        | RefNode::TypeUSize
        | RefNode::TypeInt8
        | RefNode::TypeInt16
        | RefNode::TypeInt32
        | RefNode::TypeInt64
        | RefNode::TypeInt128
        | RefNode::TypeFloat32
        | RefNode::TypeFloat64
        | RefNode::TypeInferType
        | RefNode::TypeTypeName(_)
        | RefNode::TypeRefinementType(_)
        | RefNode::TypeTupleType(_)
        | RefNode::TypeArrayType(_)
        | RefNode::TypeSliceType(_)
        | RefNode::TypeFunctionType(_)
        | RefNode::TypeReferenceType(_)
        | RefNode::TypeOpaqueType(_)
        | RefNode::TypeLatentType(_)
        | RefNode::TypeLifetime(_)
        | RefNode::TypeParentheses(_)
        | RefNode::ItemSyntaxError
        | RefNode::ItemItemPath(_)
        | RefNode::ItemTypeParams(_)
        | RefNode::ItemImport(_)
        | RefNode::ItemStructField(_)
        | RefNode::ItemEnumVariant(_)
        | RefNode::ItemImpl(_)
        | RefNode::ItemFuncParam(_)
        | RefNode::ItemModule(_) => {}

        RefNode::ItemTypeAlias(type_alias) => {
            // TODO: implement
            println!("scope = {:?}", ctx.current_scope());
        }

        RefNode::ItemStruct(_) => {
            // TODO: implement
            println!("scope = {:?}", ctx.current_scope());
        }

        RefNode::ItemEnum(_) => {
            // TODO: implement
            println!("scope = {:?}", ctx.current_scope());
        }

        RefNode::ItemTrait(_) => {
            // TODO: implement
            println!("scope = {:?}", ctx.current_scope());
        }

        RefNode::ItemFunction(function) => {
            // TODO: implement
            println!("scope = {:?}", ctx.current_scope());
        }

        RefNode::ItemVariable(variable) => {
            // TODO: implement
            println!("scope = {:?}", ctx.current_scope());
        }
    }
}

pub fn ast_mod2hir(module: ast::Module, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Module, ()> {
    module.depth_first_iter(&mut |order, node| match order {
        Order::Enter => {
            if let RefNode::ItemModule(module) = node {
                if let Some(name) = &module.name {
                    ctx.push_current_scope(name.to_string());
                } else {
                    ctx.push_current_scope("".to_string());
                }
            } else if let RefNode::ItemFunction(function) = node {
                ctx.push_current_scope(function.name.to_string());
            }

            visit_node(node, ctx, log);
        }

        Order::Leave => {
            if let RefNode::ItemModule(_) | RefNode::ItemFunction(_) = node {
                ctx.pop_current_scope();
            }
        }
    });

    module.ast2hir(ctx, log)
}

pub fn ast_expr2hir(expr: ast::Expr, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    expr.depth_first_iter(&mut |order, node| match order {
        Order::Enter => visit_node(node, ctx, log),
        Order::Leave => {}
    });

    expr.ast2hir(ctx, log)
}

pub fn ast_type2hir(ty: ast::Type, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    ty.depth_first_iter(&mut |order, node| match order {
        Order::Enter => visit_node(node, ctx, log),
        Order::Leave => {}
    });

    ty.ast2hir(ctx, log)
}
