use crate::prelude::*;

impl std::fmt::Display for AttributeList {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if let Some(trivia) = &self.trivia {
            write!(f, "{}", trivia)?;
        }

        write!(f, "[")?;
        for (i, attr) in self.attributes.iter().enumerate() {
            let expr = attr.borrow();
            write!(f, "{}", expr)?;
            if i + 1 != self.attributes.len() {
                write!(f, ",")?;
            }
        }
        write!(f, "]")
    }
}

impl std::fmt::Display for Item {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Item::Root { items } => {
                for item in items {
                    let item = item.borrow();
                    write!(f, "{}", item)?;
                }
                Ok(())
            }

            Item::Module {
                source_offset: _,
                trivia,
                attributes,
                name,
                items,
            } => {
                if let Some(trivia) = &trivia[0] {
                    write!(f, "{}", trivia)?;
                }

                write!(f, "mod")?;

                if let Some(attributes) = attributes {
                    write!(f, "{}", attributes)?;
                }

                if let Some(trivia) = &trivia[1] {
                    write!(f, "{}", trivia)?;
                }

                write!(f, "{}", name)?;

                if let Some(trivia) = &trivia[2] {
                    write!(f, "{}", trivia)?;
                }

                write!(f, "{{")?;
                for item in items {
                    let item = item.borrow();
                    write!(f, "{}", item)?;
                }
                write!(f, "}}")?;

                Ok(())
            }
        }
    }
}
