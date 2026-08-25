// ─────────────────────────────────────────────────────────────
// MIR CFG Graphviz Dot Export
// ─────────────────────────────────────────────────────────────
//
// Provides Graphviz DOT-format export for MIR control-flow graphs,
// modeled after Rust's `rustc -Z dump-mir`. Basic blocks become nodes,
// terminators become directed edges. Statements and values are rendered
// in a Rust-style MIR syntax rather than as serialized data structures.
//
// Usage:
//   use nitrate_mir::prelude::*;
//   let dot = mir_func.emit_dot();
//   std::fs::write("func.dot", dot)?;
//   let dot = mir_module.emit_dot();
//   std::fs::write("module.dot", dot)?;

use crate::func::{MirFunction, MirModule};
use crate::operand::{MirBinaryOp, MirLiteral, MirUnaryOp, Operand};
use crate::place::Place;
use crate::rvalue::{AggregateKind, BorrowKind, NullaryOp, Rvalue};
use crate::stmt::{Statement, Terminator};
use crate::store::{BasicBlockId, LocalId, MirTypeId};
use std::fmt::Write;
use std::format;
use thin_vec::ThinVec;

// ─────────────────────────────────────────────────────────────
// DOT emission for individual MIR functions
// ─────────────────────────────────────────────────────────────

impl MirFunction {
    /// Emit this function's CFG as a Graphviz DOT string.
    ///
    /// Extern functions (no body) emit a single dashed declaration node.
    /// Functions with bodies emit the full CFG with MIR-syntax statements,
    /// terminators, and block argument edge labels.
    #[must_use]
    pub fn emit_dot(&self) -> String {
        let mut dot = String::new();
        let graph_id = sanitize_id(&self.name);
        let _ = writeln!(dot, "digraph {} {{", graph_id);
        let _ = writeln!(dot, "    label=\"{}\";", self.name);
        let _ = writeln!(dot, "    labelloc=t;");
        let _ = writeln!(dot, "    fontsize=14;");
        let _ = writeln!(
            dot,
            "    node [shape=box, style=filled, fillcolor=white, fontname=\"monospace\", fontsize=10];"
        );
        let _ = writeln!(dot, "    edge [fontname=\"monospace\", fontsize=8, color=gray40];");

        if let Some(ref body) = self.body {
            let _ = writeln!(dot, "    entry [shape=point, fillcolor=black, width=0.2];");

            // ── Emit basic block nodes ──
            for bb_id in body.blocks.iter() {
                let bb = bb_id.borrow();
                let is_entry = bb_id.as_usize() == body.entry_block.as_usize();
                emit_block_node(&mut dot, bb_id.as_usize(), &bb, is_entry);
            }

            // ── Entry edge ──
            let _ = writeln!(dot, "    entry -> bb{};", body.entry_block.as_usize());

            // ── Edges ──
            for bb_id in body.blocks.iter() {
                let bb = bb_id.borrow();
                emit_edges_single(&mut dot, bb_id.as_usize(), &bb.terminator);
            }
        } else {
            let _ = writeln!(
                dot,
                "    decl [shape=box, style=dashed, label=\"{}\\n(extern declaration)\"];",
                self.name
            );
        }

        let _ = writeln!(dot, "}}");
        dot
    }
}

// ─────────────────────────────────────────────────────────────
// DOT emission for entire modules
// ─────────────────────────────────────────────────────────────

impl MirModule {
    /// Emit all functions in this module as a single Graphviz DOT string,
    /// with each function rendered in its own subgraph cluster.
    #[must_use]
    pub fn emit_dot(&self) -> String {
        let mut dot = String::new();
        let _ = writeln!(dot, "digraph MirModule {{");
        let _ = writeln!(dot, "    label=\"MIR Module\";");
        let _ = writeln!(dot, "    labelloc=t;");
        let _ = writeln!(dot, "    fontsize=16;");
        let _ = writeln!(dot, "    compound=true;");
        let _ = writeln!(
            dot,
            "    node [shape=box, style=filled, fillcolor=white, fontname=\"monospace\", fontsize=10];"
        );

        for (fi, func_id) in self.functions.iter().enumerate() {
            let mir_func = func_id.borrow();
            let _ = writeln!(dot, "    subgraph cluster_{} {{", fi);
            let _ = writeln!(dot, "        label=\"{}\";", mir_func.name);
            let _ = writeln!(dot, "        fontsize=12;");
            let _ = writeln!(dot, "        style=filled;");
            let _ = writeln!(dot, "        fillcolor=gray95;");

            if let Some(ref body) = mir_func.body {
                let entry_id = format!("entry_{}", fi);
                let _ = writeln!(dot, "        {} [shape=point, fillcolor=black, width=0.2];", entry_id);

                for bb_id in body.blocks.iter() {
                    let bb = bb_id.borrow();
                    let node_id = format!("bb{}_{}", bb_id.as_usize(), fi);
                    let is_entry = bb_id.as_usize() == body.entry_block.as_usize();
                    emit_block_node_with_id(&mut dot, &node_id, bb_id.as_usize(), &bb, is_entry);
                }

                let _ = writeln!(dot, "        {} -> bb{}_{};", entry_id, body.entry_block.as_usize(), fi);

                for bb_id in body.blocks.iter() {
                    let bb = bb_id.borrow();
                    let from = format!("bb{}_{}", bb_id.as_usize(), fi);
                    emit_edges_module(&mut dot, &from, fi, &bb.terminator);
                }
            } else {
                let _ = writeln!(
                    dot,
                    "        decl_{} [shape=box, style=dashed, label=\"{}\\n(extern)\"];",
                    fi, mir_func.name
                );
            }

            let _ = writeln!(dot, "    }}");
        }

        let _ = writeln!(dot, "}}");
        dot
    }
}

// ─────────────────────────────────────────────────────────────
// Node emission
// ─────────────────────────────────────────────────────────────

fn emit_block_node(dot: &mut String, id: usize, bb: &crate::stmt::BasicBlock, is_entry: bool) {
    emit_block_node_with_id(dot, &format!("bb{}", id), id, bb, is_entry);
}

fn emit_block_node_with_id(dot: &mut String, node_id: &str, id: usize, bb: &crate::stmt::BasicBlock, is_entry: bool) {
    let mut label = String::new();
    let _ = writeln!(label, "bb{}:", id);
    if is_entry {
        let _ = writeln!(label, "    // entry block");
    }
    if !bb.args.is_empty() {
        let _ = writeln!(label, "    // block args: ({})", fmt_type_list(&bb.args));
    }
    for stmt in bb.statements.iter() {
        let _ = writeln!(label, "    {}", fmt_statement(stmt));
    }
    let _ = writeln!(label, "    {};", fmt_terminator(bb));

    let (style, extra) = match &bb.terminator {
        Terminator::Return { .. } => ("filled", "fillcolor=lightyellow"),
        Terminator::Unreachable => ("dashed", "fillcolor=lightpink"),
        _ => ("filled", "fillcolor=white"),
    };

    let escaped = label.replace('\\', "\\\\").replace('"', "\\\"").replace('\n', "\\l");
    let _ = writeln!(
        dot,
        "    {} [shape=box, style={}, {}, label=\"{}\\l\"];",
        node_id, style, extra, escaped
    );
}

// ─────────────────────────────────────────────────────────────
// Edge emission (single-function mode)
// ─────────────────────────────────────────────────────────────

fn emit_edges_single(dot: &mut String, from: usize, term: &Terminator) {
    match term {
        Terminator::Goto { target, args } => {
            let _ = writeln!(
                dot,
                "    bb{} -> bb{} [label=\"{}\"];",
                from,
                target.as_usize(),
                fmt_edge_args(args)
            );
        }
        Terminator::If {
            true_target,
            false_target,
            true_args,
            false_args,
            ..
        } => {
            let _ = writeln!(
                dot,
                "    bb{} -> bb{} [label=\"true{}\", color=darkgreen];",
                from,
                true_target.as_usize(),
                fmt_edge_args(true_args)
            );
            let _ = writeln!(
                dot,
                "    bb{} -> bb{} [label=\"false{}\", color=firebrick];",
                from,
                false_target.as_usize(),
                fmt_edge_args(false_args)
            );
        }
        Terminator::SwitchInt {
            targets,
            otherwise,
            otherwise_args,
            ..
        } => {
            for (val, tgt, args) in targets.iter() {
                let _ = writeln!(
                    dot,
                    "    bb{} -> bb{} [label=\"switch {}{}\", color=darkblue];",
                    from,
                    tgt.as_usize(),
                    val,
                    fmt_edge_args(args)
                );
            }
            let _ = writeln!(
                dot,
                "    bb{} -> bb{} [label=\"otherwise{}\", color=darkblue, style=dashed];",
                from,
                otherwise.as_usize(),
                fmt_edge_args(otherwise_args)
            );
        }
        Terminator::Return { .. } | Terminator::Unreachable => {}
        Terminator::Call {
            target, target_args, ..
        } => {
            if let Some(t) = target {
                let _ = writeln!(
                    dot,
                    "    bb{} -> bb{} [label=\"return{}\", color=darkorchid];",
                    from,
                    t.as_usize(),
                    fmt_edge_args(target_args)
                );
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Edge emission (module-cluster mode)
// ─────────────────────────────────────────────────────────────

fn emit_edges_module(dot: &mut String, from_node: &str, func_idx: usize, term: &Terminator) {
    let to = |tgt: &BasicBlockId| format!("bb{}_{}", tgt.as_usize(), func_idx);

    match term {
        Terminator::Goto { target, args } => {
            let _ = writeln!(
                dot,
                "        {} -> {} [label=\"{}\"];",
                from_node,
                to(target),
                fmt_edge_args(args)
            );
        }
        Terminator::If {
            true_target,
            false_target,
            true_args,
            false_args,
            ..
        } => {
            let _ = writeln!(
                dot,
                "        {} -> {} [label=\"true{}\", color=darkgreen];",
                from_node,
                to(true_target),
                fmt_edge_args(true_args)
            );
            let _ = writeln!(
                dot,
                "        {} -> {} [label=\"false{}\", color=firebrick];",
                from_node,
                to(false_target),
                fmt_edge_args(false_args)
            );
        }
        Terminator::SwitchInt {
            targets,
            otherwise,
            otherwise_args,
            ..
        } => {
            for (val, tgt, args) in targets.iter() {
                let _ = writeln!(
                    dot,
                    "        {} -> {} [label=\"switch {}{}\", color=darkblue];",
                    from_node,
                    to(tgt),
                    val,
                    fmt_edge_args(args)
                );
            }
            let _ = writeln!(
                dot,
                "        {} -> {} [label=\"otherwise{}\", color=darkblue, style=dashed];",
                from_node,
                to(otherwise),
                fmt_edge_args(otherwise_args)
            );
        }
        Terminator::Return { .. } | Terminator::Unreachable => {}
        Terminator::Call {
            target, target_args, ..
        } => {
            if let Some(t) = target {
                let _ = writeln!(
                    dot,
                    "        {} -> {} [label=\"return{}\", color=darkorchid];",
                    from_node,
                    to(t),
                    fmt_edge_args(target_args)
                );
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
// MIR syntax formatters — Operand, Place, Rvalue, Literal
// ─────────────────────────────────────────────────────────────

/// Format an operand list for edge labels (block arguments).
fn fmt_edge_args(args: &ThinVec<Operand>) -> String {
    if args.is_empty() {
        String::new()
    } else {
        let items: Vec<String> = args.iter().map(fmt_operand).collect();
        format!(" ({})", items.join(", "))
    }
}

/// Format an operand in MIR syntax: `copy _N`, `move _N`, `const 42_u8`
fn fmt_operand(op: &Operand) -> String {
    match op {
        Operand::Copy(place) => format!("copy {}", fmt_place(place)),
        Operand::Move(place) => format!("move {}", fmt_place(place)),
        Operand::Constant(lit) => format!("const {}", fmt_literal(lit)),
    }
}

/// Format a place in MIR syntax: `_N`, `STATIC`, `(*place)`, `place.field`, `place[idx]`, `place as Variant`
fn fmt_place(place: &Place) -> String {
    match place {
        Place::Local(local) => fmt_local(local),
        Place::Static(name) => format!("@{}", name),
        Place::Deref(base) => format!("(*{})", fmt_place(base)),
        Place::Field { base, field_name } => format!("{}.{}", fmt_place(base), field_name),
        Place::Index { base, index } => format!("{}[{}]", fmt_place(base), fmt_place(index)),
        Place::Downcast { base, variant_name } => format!("{} as {}", fmt_place(base), variant_name),
    }
}

/// Format a MIR literal in source-like syntax: `true`, `42_i32`, `"hello"`
fn fmt_literal(lit: &MirLiteral) -> String {
    match lit {
        MirLiteral::Unit => "()".to_string(),
        MirLiteral::Bool(true) => "true".to_string(),
        MirLiteral::Bool(false) => "false".to_string(),
        MirLiteral::I8(v) => format!("{}_i8", v),
        MirLiteral::I16(v) => format!("{}_i16", v),
        MirLiteral::I32(v) => format!("{}_i32", v),
        MirLiteral::I64(v) => format!("{}_i64", v),
        MirLiteral::I128(v) => format!("{}_i128", v),
        MirLiteral::U8(v) => format!("{}_u8", v),
        MirLiteral::U16(v) => format!("{}_u16", v),
        MirLiteral::U32(v) => format!("{}_u32", v),
        MirLiteral::U64(v) => format!("{}_u64", v),
        MirLiteral::U128(v) => format!("{}_u128", v),
        MirLiteral::F32(v) => {
            let f: f32 = v.into_inner();
            format!("{}_f32", f)
        }
        MirLiteral::F64(v) => {
            let f: f64 = v.into_inner();
            format!("{}_f64", f)
        }
        MirLiteral::USize { value, .. } => format!("{}_usize", value),
        MirLiteral::Str(s) => format!("\"{}\"", s),
        MirLiteral::BStr(_) => "b\"...\"".to_string(),
    }
}

/// Format a binary operator symbol.
fn fmt_binop(op: MirBinaryOp) -> &'static str {
    match op {
        MirBinaryOp::Add => "+",
        MirBinaryOp::Sub => "-",
        MirBinaryOp::Mul => "*",
        MirBinaryOp::Div => "/",
        MirBinaryOp::Mod => "%",
        MirBinaryOp::And => "&",
        MirBinaryOp::Or => "|",
        MirBinaryOp::Xor => "^",
        MirBinaryOp::Shl => "<<",
        MirBinaryOp::Shr => ">>",
        MirBinaryOp::Rol => "rol",
        MirBinaryOp::Ror => "ror",
        MirBinaryOp::LogicAnd => "&&",
        MirBinaryOp::LogicOr => "||",
        MirBinaryOp::Lt => "<",
        MirBinaryOp::Gt => ">",
        MirBinaryOp::Lte => "<=",
        MirBinaryOp::Gte => ">=",
        MirBinaryOp::Eq => "==",
        MirBinaryOp::Ne => "!=",
    }
}

/// Format a unary operator symbol.
fn fmt_unop(op: MirUnaryOp) -> &'static str {
    match op {
        MirUnaryOp::Neg => "-",
        MirUnaryOp::Not => "!",
    }
}

/// Format a local as `_N` (Rust MIR style).
fn fmt_local(local: &LocalId) -> String {
    format!("_{}", local.as_usize())
}

/// Format a type in MIR style.
fn fmt_type(ty: &MirTypeId) -> String {
    // Access the MirType through the Deref impl
    let t: &crate::ty::MirType = &*ty;
    fmt_mir_type_raw(t)
}

/// Format a MirType without Debug noise (no truncation).
fn fmt_mir_type_raw(ty: &crate::ty::MirType) -> String {
    use crate::ty::MirType;
    match ty {
        MirType::Never => "never".to_string(),
        MirType::Unit => "()".to_string(),
        MirType::Bool => "bool".to_string(),
        MirType::U8 => "u8".to_string(),
        MirType::U16 => "u16".to_string(),
        MirType::U32 => "u32".to_string(),
        MirType::U64 => "u64".to_string(),
        MirType::U128 => "u128".to_string(),
        MirType::I8 => "i8".to_string(),
        MirType::I16 => "i16".to_string(),
        MirType::I32 => "i32".to_string(),
        MirType::I64 => "i64".to_string(),
        MirType::I128 => "i128".to_string(),
        MirType::F32 => "f32".to_string(),
        MirType::F64 => "f64".to_string(),
        MirType::USize => "usize".to_string(),
        MirType::Range => "range".to_string(),
        MirType::Str => "str".to_string(),
        MirType::Array { element_type, len } => {
            format!("[{}; {}]", fmt_type(element_type), len)
        }
        MirType::Tuple { element_types } => {
            let types: Vec<String> = element_types.iter().map(fmt_type).collect();
            format!("({})", types.join(", "))
        }
        MirType::Struct { name, .. } => name.to_string(),
        MirType::Enum { name, .. } => name.to_string(),
        MirType::Reference { exclusive, mutable, to } => {
            let amp = if *mutable { "&mut " } else { "&" };
            let excl = if *exclusive { "uniq " } else { "" };
            format!("{}{}{}", amp, excl, fmt_type(to))
        }
        MirType::Pointer { exclusive, mutable, to } => {
            let star = if *mutable { "*mut " } else { "*const " };
            let excl = if *exclusive { "uniq " } else { "" };
            format!("{}{}{}", star, excl, fmt_type(to))
        }
        MirType::SliceRef {
            exclusive,
            mutable,
            element_type,
        } => {
            let amp = if *mutable { "&mut " } else { "&" };
            let excl = if *exclusive { "uniq " } else { "" };
            format!("{}{}[{}]", amp, excl, fmt_type(element_type))
        }
        MirType::SlicePtr {
            exclusive,
            mutable,
            element_type,
        } => {
            let star = if *mutable { "*mut " } else { "*const " };
            let excl = if *exclusive { "uniq " } else { "" };
            format!("{}{}[{}]", star, excl, fmt_type(element_type))
        }
        MirType::Function {
            params,
            return_type,
            is_c_variadic,
        } => {
            let param_strs: Vec<String> = params.iter().map(|(_name, ty)| fmt_type(ty)).collect();
            let mut sig = param_strs.join(", ");
            if *is_c_variadic {
                sig.push_str(", ...");
            }
            format!("fn({}) -> {}", sig, fmt_type(return_type))
        }
    }
}

/// Format a type list for block argument display.
fn fmt_type_list(types: &ThinVec<MirTypeId>) -> String {
    let items: Vec<String> = types.iter().map(fmt_type).collect();
    items.join(", ")
}

// ─────────────────────────────────────────────────────────────
// Statement and Terminator formatting (Rust MIR style)
// ─────────────────────────────────────────────────────────────

/// Format a MIR statement in source-like syntax.
fn fmt_statement(stmt: &Statement) -> String {
    match stmt {
        Statement::Assign(place, rvalue) => {
            format!("{} = {}", fmt_place(place), fmt_rvalue(rvalue))
        }
        Statement::SetDiscriminant { place, variant_index } => {
            format!("discriminant({}) = {}", fmt_place(place), variant_index)
        }
        Statement::StorageLive(local) => format!("StorageLive({})", fmt_local(local)),
        Statement::StorageDead(local) => format!("StorageDead({})", fmt_local(local)),
    }
}

/// Format an Rvalue in MIR syntax.
fn fmt_rvalue(rv: &Rvalue) -> String {
    match rv {
        Rvalue::Use(op) => fmt_operand(op),
        Rvalue::Ref { region, place } => {
            let kind = match region {
                BorrowKind::Shared => "&",
                BorrowKind::Mutable => "&mut ",
            };
            format!("{}{}", kind, fmt_place(place))
        }
        Rvalue::Len(place) => format!("Len({})", fmt_place(place)),
        Rvalue::Cast { value, target_ty } => {
            format!("{} as {}", fmt_operand(value), fmt_type(target_ty))
        }
        Rvalue::BinaryOp { op, lhs, rhs } => {
            format!("{} {} {}", fmt_operand(lhs), fmt_binop(*op), fmt_operand(rhs))
        }
        Rvalue::CheckedBinaryOp { op, lhs, rhs } => {
            format!("Checked{}({}, {})", fmt_binop(*op), fmt_operand(lhs), fmt_operand(rhs))
        }
        Rvalue::UnaryOp { op, operand } => {
            format!("{}{}", fmt_unop(*op), fmt_operand(operand))
        }
        Rvalue::NullaryOp(null, ty) => {
            let name = match null {
                NullaryOp::SizeOf => "SizeOf",
                NullaryOp::AlignOf => "AlignOf",
            };
            format!("{}({})", name, fmt_type(ty))
        }
        Rvalue::Aggregate(kind, operands) => {
            let items: Vec<String> = operands.iter().map(fmt_operand).collect();
            match kind {
                AggregateKind::Tuple => format!("({})", items.join(", ")),
                AggregateKind::Array(_elem_ty) => {
                    format!("[{}]", items.join(", "))
                }
                AggregateKind::Struct(name, fields) => {
                    let named: Vec<String> = fields
                        .iter()
                        .zip(items.iter())
                        .map(|(f, v)| format!("{}: {}", f, v))
                        .collect();
                    format!("{} {{ {} }}", name, named.join(", "))
                }
                AggregateKind::Enum { name, variant_name, .. } => {
                    format!("{}::{} {{ {} }}", name, variant_name, items.join(", "))
                }
            }
        }
    }
}

/// Format a terminator as MIR code (shown inside the block node).
fn fmt_terminator(bb: &crate::stmt::BasicBlock) -> String {
    match &bb.terminator {
        Terminator::Goto { target, args } => {
            if args.is_empty() {
                format!("goto bb{}", target.as_usize())
            } else {
                let items: Vec<String> = args.iter().map(fmt_operand).collect();
                format!("goto bb{}({})", target.as_usize(), items.join(", "))
            }
        }
        Terminator::If {
            condition,
            true_target,
            false_target,
            ..
        } => {
            format!(
                "if {} goto bb{} else goto bb{}",
                fmt_operand(condition),
                true_target.as_usize(),
                false_target.as_usize()
            )
        }
        Terminator::SwitchInt {
            discr,
            targets,
            otherwise,
            ..
        } => {
            let mut s = format!("switch_int({}) -> [", fmt_operand(discr));
            for (i, (val, tgt, _args)) in targets.iter().enumerate() {
                if i > 0 {
                    s.push_str(", ");
                }
                let _ = write!(s, "{}: bb{}", val, tgt.as_usize());
            }
            let _ = write!(s, ", _: bb{}]", otherwise.as_usize());
            s
        }
        Terminator::Return { value } => match value {
            Some(v) => format!("return {}", fmt_operand(v)),
            None => "return".to_string(),
        },
        Terminator::Unreachable => "unreachable".to_string(),
        Terminator::Call {
            callee,
            args,
            destination,
            ..
        } => {
            let callee_str = fmt_operand(callee);
            let args_str: Vec<String> = args.iter().map(fmt_operand).collect();
            match destination {
                Some(dest) => format!("{} = call {}({})", fmt_place(dest), callee_str, args_str.join(", ")),
                None => format!("call {}({})", callee_str, args_str.join(", ")),
            }
        }
    }
}

/// Sanitize a name for use as a DOT graph/node ID.
fn sanitize_id(name: &str) -> String {
    name.chars()
        .map(|c| if c.is_alphanumeric() || c == '_' { c } else { '_' })
        .collect()
}
