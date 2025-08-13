use super::expression::ExprRef;
use super::storage::{ExprKey, Storage, TypeKey};

pub struct Printable<'storage, 'a> {
    storage: &'storage Storage<'a>,
    expr: ExprKey<'a>,
}

impl<'storage, 'a> ExprKey<'a> {
    #[must_use]
    pub fn as_printable(&self, storage: &'storage Storage<'a>) -> Printable<'storage, 'a> {
        Printable {
            storage,
            expr: *self,
        }
    }
}

impl<'storage, 'a> TypeKey<'a> {
    #[must_use]
    pub fn as_printable(&self, storage: &'storage Storage<'a>) -> Printable<'storage, 'a> {
        Printable {
            storage,
            expr: (*self).into(),
        }
    }
}

#[derive(Debug)]
#[allow(dead_code)]
struct Parameter<'storage, 'a> {
    name: &'a str,
    param_type: Printable<'storage, 'a>,
    default_value: Option<Printable<'storage, 'a>>,
}

#[derive(Debug)]
#[allow(dead_code)]
struct SwitchCase<'storage, 'a> {
    value: Printable<'storage, 'a>,
    body: Printable<'storage, 'a>,
}

#[derive(Debug)]
#[allow(dead_code)]
struct ForeachBinding<'storage, 'a> {
    name: &'a str,
    type_: Option<Printable<'storage, 'a>>,
}

impl std::fmt::Debug for Printable<'_, '_> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self.expr.get(self.storage) {
            ExprRef::Bool => write!(f, "bool"),
            ExprRef::UInt8 => write!(f, "u8"),
            ExprRef::UInt16 => write!(f, "u16"),
            ExprRef::UInt32 => write!(f, "u32"),
            ExprRef::UInt64 => write!(f, "u64"),
            ExprRef::UInt128 => write!(f, "u128"),
            ExprRef::Int8 => write!(f, "i8"),
            ExprRef::Int16 => write!(f, "i16"),
            ExprRef::Int32 => write!(f, "i32"),
            ExprRef::Int64 => write!(f, "i64"),
            ExprRef::Int128 => write!(f, "i128"),
            ExprRef::Float8 => write!(f, "f8"),
            ExprRef::Float16 => write!(f, "f16"),
            ExprRef::Float32 => write!(f, "f32"),
            ExprRef::Float64 => write!(f, "f64"),
            ExprRef::Float128 => write!(f, "f128"),

            ExprRef::InferType => write!(f, "_"),
            ExprRef::TypeName(x) => f.debug_struct("TypeName").field("name", &x).finish(),

            ExprRef::RefinementType(x) => f
                .debug_struct("RefinementType")
                .field("base", &x.base().as_printable(self.storage))
                .field("width", &x.width().map(|w| w.as_printable(self.storage)))
                .field("min", &x.min().map(|m| m.as_printable(self.storage)))
                .field("max", &x.max().map(|m| m.as_printable(self.storage)))
                .finish(),

            ExprRef::TupleType(x) => f
                .debug_struct("TupleType")
                .field(
                    "elements",
                    &x.elements()
                        .iter()
                        .map(|e| e.as_printable(self.storage))
                        .collect::<Vec<_>>(),
                )
                .finish(),

            ExprRef::ArrayType(x) => f
                .debug_struct("ArrayType")
                .field("element", &x.element().as_printable(self.storage))
                .field("count", &x.count().as_printable(self.storage))
                .finish(),

            ExprRef::MapType(x) => f
                .debug_struct("MapType")
                .field("key", &x.key().as_printable(self.storage))
                .field("value", &x.value().as_printable(self.storage))
                .finish(),

            ExprRef::SliceType(x) => f
                .debug_struct("SliceType")
                .field("element", &x.element().as_printable(self.storage))
                .finish(),

            ExprRef::FunctionType(x) => f
                .debug_struct("FunctionType")
                .field(
                    "parameters",
                    &x.parameters()
                        .iter()
                        .map(|param| Parameter {
                            name: param.name(),
                            param_type: param.param_type().as_printable(self.storage),
                            default_value: param
                                .default_value()
                                .map(|v| v.as_printable(self.storage)),
                        })
                        .collect::<Vec<_>>(),
                )
                .field("return_type", &x.return_type().as_printable(self.storage))
                .field(
                    "attributes",
                    &x.attributes()
                        .iter()
                        .map(|a| a.as_printable(self.storage))
                        .collect::<Vec<_>>(),
                )
                .finish(),

            ExprRef::ManagedRefType(x) => f
                .debug_struct("ManagedRefType")
                .field("target", &x.target().as_printable(self.storage))
                .field("is_mutable", &x.is_mutable())
                .finish(),

            ExprRef::UnmanagedRefType(x) => f
                .debug_struct("UnmanagedRefType")
                .field("target", &x.target().as_printable(self.storage))
                .field("is_mutable", &x.is_mutable())
                .finish(),

            ExprRef::GenericType(x) => f
                .debug_struct("GenericType")
                .field("base", &x.base().as_printable(self.storage))
                .field(
                    "args",
                    &x.arguments()
                        .iter()
                        .map(|(n, arg)| (n, arg.as_printable(self.storage)))
                        .collect::<Vec<_>>(),
                )
                .finish(),

            ExprRef::OpaqueType(x) => f
                .debug_struct("OpaqueType")
                .field("identity", &x.get())
                .finish(),

            ExprRef::Discard => write!(f, "Discard"),

            ExprRef::BooleanLit(x) => f.debug_struct("BooleanLit").field("value", &x).finish(),

            ExprRef::IntegerLit(x) => f
                .debug_struct("IntegerLit")
                .field("value", &x.get_u128())
                .field("kind", &x.kind())
                .finish(),

            ExprRef::FloatLit(x) => f.debug_struct("FloatLit").field("value", &x).finish(),

            ExprRef::StringLit(x) => f
                .debug_struct("StringLit")
                .field("value", &x.get())
                .finish(),

            ExprRef::BStringLit(x) => f
                .debug_struct("BStringLit")
                .field("value", &x.get())
                .finish(),

            ExprRef::ListLit(x) => f
                .debug_list()
                .entries(x.elements().iter().map(|e| e.as_printable(self.storage)))
                .finish(),

            ExprRef::ObjectLit(x) => f
                .debug_map()
                .entries(
                    x.get()
                        .iter()
                        .map(|(k, v)| (k, v.as_printable(self.storage))),
                )
                .finish(),

            ExprRef::UnaryExpr(x) => f
                .debug_struct("UnaryExpr")
                .field("operand", &x.operand().as_printable(self.storage))
                .field("operator", &x.operator())
                .field("is_postfix", &x.is_postfix())
                .finish(),

            ExprRef::BinExpr(x) => f
                .debug_struct("BinExpr")
                .field("left", &x.left().as_printable(self.storage))
                .field("right", &x.right().as_printable(self.storage))
                .field("operator", &x.op())
                .finish(),

            ExprRef::Statement(x) => f
                .debug_struct("Statement")
                .field("expr", &x.get().as_printable(self.storage))
                .finish(),

            ExprRef::Block(x) => f
                .debug_struct("Block")
                .field(
                    "elements",
                    &x.elements()
                        .iter()
                        .map(|s| s.as_printable(self.storage))
                        .collect::<Vec<_>>(),
                )
                .finish(),

            ExprRef::Function(x) => {
                let parameters = x
                    .parameters()
                    .iter()
                    .map(|param| Parameter {
                        name: param.name(),
                        param_type: param.param_type().as_printable(self.storage),
                        default_value: param.default_value().map(|v| v.as_printable(self.storage)),
                    })
                    .collect::<Vec<_>>();

                f.debug_struct("Function")
                    .field("parameters", &parameters)
                    .field(
                        "return_type",
                        &x.return_type().map(|t| t.as_printable(self.storage)),
                    )
                    .field(
                        "attributes",
                        &x.attributes()
                            .iter()
                            .map(|a| a.as_printable(self.storage))
                            .collect::<Vec<_>>(),
                    )
                    .field("name", &x.name())
                    .field("definition", &x.definition())
                    .finish()
            }

            ExprRef::Variable(x) => f
                .debug_struct("Variable")
                .field("kind", &x.kind())
                .field("is_mutable", &x.is_mutable())
                .field(
                    "attributes",
                    &x.attributes()
                        .iter()
                        .map(|a| a.as_printable(self.storage))
                        .collect::<Vec<_>>(),
                )
                .field("name", &x.name())
                .field("type", &x.get_type().map(|t| t.as_printable(self.storage)))
                .field("value", &x.value().map(|v| v.as_printable(self.storage)))
                .finish(),

            ExprRef::Identifier(name) => write!(f, "{name}"),

            ExprRef::Scope(x) => f
                .debug_struct("Scope")
                .field("name", &x.scope().to_string())
                .field(
                    "attributes",
                    &x.attributes()
                        .iter()
                        .map(|a| a.as_printable(self.storage))
                        .collect::<Vec<_>>(),
                )
                .field("body", &x.block().as_printable(self.storage))
                .finish(),

            ExprRef::If(x) => f
                .debug_struct("If")
                .field("condition", &x.condition().as_printable(self.storage))
                .field("then_branch", &x.then_branch().as_printable(self.storage))
                .field(
                    "else_branch",
                    &x.else_branch().map(|b| b.as_printable(self.storage)),
                )
                .finish(),

            ExprRef::WhileLoop(x) => f
                .debug_struct("WhileLoop")
                .field("condition", &x.condition().as_printable(self.storage))
                .field("body", &x.body().as_printable(self.storage))
                .finish(),

            ExprRef::DoWhileLoop(x) => f
                .debug_struct("DoWhileLoop")
                .field("condition", &x.condition().as_printable(self.storage))
                .field("body", &x.body().as_printable(self.storage))
                .finish(),

            ExprRef::Switch(x) => f
                .debug_struct("Switch")
                .field("condition", &x.condition().as_printable(self.storage))
                .field(
                    "cases",
                    &x.cases()
                        .iter()
                        .map(|(value, body)| SwitchCase {
                            value: value.as_printable(self.storage),
                            body: body.as_printable(self.storage),
                        })
                        .collect::<Vec<_>>(),
                )
                .field(
                    "default_case",
                    &x.default_case().map(|d| d.as_printable(self.storage)),
                )
                .finish(),

            ExprRef::Break(x) => f.debug_struct("Break").field("label", &x.label()).finish(),

            ExprRef::Continue(x) => f
                .debug_struct("Continue")
                .field("label", &x.label())
                .finish(),

            ExprRef::Return(x) => f
                .debug_struct("Return")
                .field("value", &x.value().map(|v| v.as_printable(self.storage)))
                .finish(),

            ExprRef::ForEach(x) => f
                .debug_struct("ForEach")
                .field(
                    "bindings",
                    &x.bindings()
                        .iter()
                        .map(|(name, type_)| ForeachBinding {
                            name,
                            type_: type_.map(|t| t.as_printable(self.storage)),
                        })
                        .collect::<Vec<_>>(),
                )
                .field("iterable", &x.iterable().as_printable(self.storage))
                .field("body", &x.body().as_printable(self.storage))
                .finish(),

            ExprRef::Await(x) => f
                .debug_struct("Await")
                .field("expression", &x.expression().as_printable(self.storage))
                .finish(),

            ExprRef::Assert(x) => f
                .debug_struct("Assert")
                .field("condition", &x.condition().as_printable(self.storage))
                .field(
                    "message",
                    &x.message().map(|m| m.as_printable(self.storage)),
                )
                .finish(),
        }
    }
}
