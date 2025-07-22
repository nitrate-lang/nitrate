use super::Expr;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub struct NodeRef {
    id: u32,
}

impl NodeRef {
    fn new(id: u32) -> Self {
        NodeRef { id }
    }

    fn id(&self) -> u32 {
        self.id
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct FlatTree<'a> {
    elements: Vec<Expr<'a>>,
}

impl<'a> FlatTree<'a> {
    pub fn new() -> Self {
        FlatTree {
            elements: Vec::new(),
        }
    }

    pub fn add(&mut self, expr: Expr<'a>) -> NodeRef {
        let id = self.elements.len() as u32;
        self.elements.push(expr);
        NodeRef::new(id)
    }

    pub fn get(&self, id: NodeRef) -> Option<&Expr<'a>> {
        self.elements.get(id.id() as usize)
    }
}
