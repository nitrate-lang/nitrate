// ─────────────────────────────────────────────────────────────
// MIR CFG Graphviz Dot Export
// ─────────────────────────────────────────────────────────────
//
// Provides Graphviz DOT-format export for MIR control-flow graphs,
// modeled after Rust's `rustc -Z dump-mir`. Each function's basic blocks
// become nodes; terminators become directed edges. Block arguments are
// shown as edge labels.
//
// Usage:
//   use nitrate_mir::prelude::*;
//   let dot = mir_func.emit_dot();
//   std::fs::write("func.dot", dot)?;
//   let dot = mir_module.emit_dot();
//   std::fs::write("module.dot", dot)?;

use crate::func::{MirFunction, MirModule};
use crate::operand::Operand;
use crate::stmt::{Statement, Terminator};
use crate::store::{BasicBlockId, MirTypeId};
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
    /// Functions with bodies emit the full CFG with statements, terminators,
    /// and block argument edge labels.
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

            // ── Emit nodes ──
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
        let _ = writeln!(label, "  // entry");
    }
    if !bb.args.is_empty() {
        let _ = writeln!(label, "  // args: {}", fmt_type_list(&bb.args));
    }
    for stmt in bb.statements.iter() {
        let _ = writeln!(label, "  {}", fmt_statement(stmt));
    }
    let _ = writeln!(label, "  [{}]", fmt_terminator_short(&bb.terminator));

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
                fmt_arg_list(args)
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
                "    bb{} -> bb{} [label=\"true {}\", color=darkgreen];",
                from,
                true_target.as_usize(),
                fmt_arg_list(true_args)
            );
            let _ = writeln!(
                dot,
                "    bb{} -> bb{} [label=\"false {}\", color=firebrick];",
                from,
                false_target.as_usize(),
                fmt_arg_list(false_args)
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
                    "    bb{} -> bb{} [label=\"switch {} {}\", color=darkblue];",
                    from,
                    tgt.as_usize(),
                    val,
                    fmt_arg_list(args)
                );
            }
            let _ = writeln!(
                dot,
                "    bb{} -> bb{} [label=\"otherwise {}\", color=darkblue, style=dashed];",
                from,
                otherwise.as_usize(),
                fmt_arg_list(otherwise_args)
            );
        }
        Terminator::Return { .. } | Terminator::Unreachable => {}
        Terminator::Call {
            target, target_args, ..
        } => {
            if let Some(t) = target {
                let _ = writeln!(
                    dot,
                    "    bb{} -> bb{} [label=\"return {}\", color=darkorchid];",
                    from,
                    t.as_usize(),
                    fmt_arg_list(target_args)
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
                fmt_arg_list(args)
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
                "        {} -> {} [label=\"true {}\", color=darkgreen];",
                from_node,
                to(true_target),
                fmt_arg_list(true_args)
            );
            let _ = writeln!(
                dot,
                "        {} -> {} [label=\"false {}\", color=firebrick];",
                from_node,
                to(false_target),
                fmt_arg_list(false_args)
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
                    "        {} -> {} [label=\"switch {} {}\", color=darkblue];",
                    from_node,
                    to(tgt),
                    val,
                    fmt_arg_list(args)
                );
            }
            let _ = writeln!(
                dot,
                "        {} -> {} [label=\"otherwise {}\", color=darkblue, style=dashed];",
                from_node,
                to(otherwise),
                fmt_arg_list(otherwise_args)
            );
        }
        Terminator::Return { .. } | Terminator::Unreachable => {}
        Terminator::Call {
            target, target_args, ..
        } => {
            if let Some(t) = target {
                let _ = writeln!(
                    dot,
                    "        {} -> {} [label=\"return {}\", color=darkorchid];",
                    from_node,
                    to(t),
                    fmt_arg_list(target_args)
                );
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Formatting helpers
// ─────────────────────────────────────────────────────────────

fn fmt_arg_list(args: &ThinVec<Operand>) -> String {
    if args.is_empty() {
        String::new()
    } else {
        let items: Vec<String> = args.iter().map(|op| format!("{op:?}")).collect();
        format!("[{}]", items.join(", "))
    }
}

fn fmt_type_list(types: &ThinVec<MirTypeId>) -> String {
    let items: Vec<String> = types.iter().map(|ty| format!("{ty:?}")).collect();
    items.join(", ")
}

fn fmt_statement(stmt: &Statement) -> String {
    match stmt {
        Statement::Assign(p, rv) => format!("{p:?} = {rv:?}"),
        Statement::SetDiscriminant { place, variant_index } => {
            format!("set_discriminant({place:?}, {variant_index})")
        }
        Statement::StorageLive(local) => format!("StorageLive({local:?})"),
        Statement::StorageDead(local) => format!("StorageDead({local:?})"),
    }
}

fn fmt_terminator_short(term: &Terminator) -> String {
    match term {
        Terminator::Goto { target, .. } => format!("goto bb{}", target.as_usize()),
        Terminator::If {
            true_target,
            false_target,
            ..
        } => format!("if bb{}, else bb{}", true_target.as_usize(), false_target.as_usize()),
        Terminator::SwitchInt { .. } => "switch".to_string(),
        Terminator::Return { .. } => "return".to_string(),
        Terminator::Unreachable => "unreachable".to_string(),
        Terminator::Call {
            callee, destination, ..
        } => match destination {
            Some(_) => format!("call {callee:?} -> ..."),
            None => format!("call {callee:?} (diverging)"),
        },
    }
}

fn sanitize_id(name: &str) -> String {
    name.chars()
        .map(|c| if c.is_alphanumeric() || c == '_' { c } else { '_' })
        .collect()
}
