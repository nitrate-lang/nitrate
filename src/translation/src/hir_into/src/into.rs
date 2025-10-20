use crate::{context::Ast2HirCtx, lower::Ast2Hir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_source::{Order, ParseTreeIter, RefNode, ast};

fn visit_node(node: RefNode, ctx: &mut Ast2HirCtx, log: &CompilerLog) {
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
            if let Ok(type_alias) = type_alias.to_owned().ast2hir(ctx, log) {
                let defintion = TypeDefinition::TypeAliasDef(type_alias);
                ctx.symbol_tab.add_type(defintion, &ctx.store);
            }
        }

        RefNode::ItemStruct(struct_def) => {
            if let Ok(struct_def) = struct_def.to_owned().ast2hir(ctx, log) {
                let defintion = TypeDefinition::StructDef(struct_def);
                ctx.symbol_tab.add_type(defintion, &ctx.store);
            }
        }

        RefNode::ItemEnum(enum_def) => {
            if let Ok(enum_def) = enum_def.to_owned().ast2hir(ctx, log) {
                let defintion = TypeDefinition::EnumDef(enum_def);
                ctx.symbol_tab.add_type(defintion, &ctx.store);
            }
        }

        RefNode::ItemTrait(_) => {
            // TODO: implement
            println!("scope = {:?}", ctx.current_scope);
        }

        RefNode::ItemFunction(function) => {
            if let Ok(function) = function.to_owned().ast2hir(ctx, log) {
                let defintion = SymbolId::Function(function);
                ctx.symbol_tab.add_symbol(defintion, &ctx.store);
            }
        }

        RefNode::ItemVariable(_variable) => {
            // TODO: implement. is it local or global?
            println!("scope = {:?}", ctx.current_scope);
        }
    }
}

pub fn ast_mod2hir(
    module: ast::Module,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Module, ()> {
    module.depth_first_iter(&mut |order, node| match order {
        Order::Enter => {
            if let RefNode::ItemModule(module) = node {
                if let Some(name) = &module.name {
                    ctx.current_scope.push(name.to_string());
                } else {
                    ctx.current_scope.push("".to_string());
                }
            } else if let RefNode::ItemFunction(function) = node {
                ctx.current_scope.push(function.name.to_string());
            }

            visit_node(node, ctx, log);
        }

        Order::Leave => {
            if let RefNode::ItemModule(_) | RefNode::ItemFunction(_) = node {
                ctx.current_scope.pop();
            }
        }
    });

    module.ast2hir(ctx, log)
}

pub fn ast_expr2hir(expr: ast::Expr, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    expr.depth_first_iter(&mut |order, node| match order {
        Order::Enter => visit_node(node, ctx, log),
        Order::Leave => {}
    });

    expr.ast2hir(ctx, log)
}

pub fn ast_type2hir(ty: ast::Type, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    ty.depth_first_iter(&mut |order, node| match order {
        Order::Enter => visit_node(node, ctx, log),
        Order::Leave => {}
    });

    ty.ast2hir(ctx, log)
}
