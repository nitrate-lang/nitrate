use crate::{prelude::*, store::FunctionId, ty::FunctionAttribute};
use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub enum Function {
    External {
        attributes: Vec<FunctionAttribute>,
        name: IString,
        parameters: Vec<TypeId>,
        return_type: TypeId,
    },

    Internal {
        attributes: Vec<FunctionAttribute>,
        name: IString,
        parameters: Vec<TypeId>,
        return_type: TypeId,
        body: BlockId,
    },

    Closure {
        callee: FunctionId,
        captures: Vec<ValueId>,
    },

    Indirect {
        callee: ValueId,
    },
}

impl Dump for Function {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Function::External {
                attributes,
                name,
                parameters,
                return_type,
            } => {
                write!(o, "fn ")?;
                if !attributes.is_empty() {
                    write!(o, "[")?;
                    for (attr, i) in attributes.iter().zip(0..) {
                        if i != 0 {
                            write!(o, ", ")?;
                        }
                        attr.dump(ctx, o)?;
                    }
                    write!(o, "] ")?;
                }
                write!(o, "{}(", name)?;
                for (param, i) in parameters.iter().zip(0..) {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    ctx.store[param].dump(ctx, o)?;
                }
                write!(o, ") -> ")?;
                ctx.store[return_type].dump(ctx, o)
            }

            Function::Internal {
                attributes,
                name,
                parameters,
                return_type,
                body,
            } => {
                write!(o, "fn ")?;
                if !attributes.is_empty() {
                    write!(o, "[")?;
                    for (attr, i) in attributes.iter().zip(0..) {
                        if i != 0 {
                            write!(o, ", ")?;
                        }
                        attr.dump(ctx, o)?;
                    }
                    write!(o, "] ")?;
                }
                write!(o, "{}(", name)?;
                for (param, i) in parameters.iter().zip(0..) {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    ctx.store[param].dump(ctx, o)?;
                }
                write!(o, ") -> ")?;
                ctx.store[return_type].dump(ctx, o)?;
                write!(o, " ")?;
                ctx.store[body].dump(ctx, o)
            }

            Function::Closure { callee, captures } => {
                write!(o, "closure ")?;
                write!(o, " [")?;
                for (capture, i) in captures.iter().zip(0..) {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    ctx.store[capture].dump(ctx, o)?;
                }
                write!(o, "] ")?;
                ctx.store[callee].dump(ctx, o)
            }

            Function::Indirect { callee } => {
                write!(o, "fn ")?;
                ctx.store[callee].dump(ctx, o)
            }
        }
    }

    fn dump_trunk(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Function::External { name, .. } => {
                write!(o, "fn {name}")
            }

            Function::Internal { name, .. } => {
                write!(o, "fn {name}")
            }

            Function::Closure { captures, callee } => {
                write!(o, "fn closure ")?;

                if !captures.is_empty() {
                    write!(o, "[")?;
                    for (capture, i) in captures.iter().zip(0..) {
                        if i != 0 {
                            write!(o, ", ")?;
                        }
                        ctx.store[capture].dump_trunk(ctx, o)?;
                    }
                    write!(o, "] ")?;
                }

                ctx.store[callee].dump_trunk(ctx, o)
            }

            Function::Indirect { callee } => {
                write!(o, "fn ")?;
                ctx.store[callee].dump_trunk(ctx, o)
            }
        }
    }
}
