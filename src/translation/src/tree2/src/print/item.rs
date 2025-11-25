use crate::prelude::*;

pub(crate) fn print_attributes(
    attributes: &Option<AttributeList>,
    f: &mut std::fmt::Formatter<'_>,
) -> std::fmt::Result {
    if let Some(attributes) = attributes {
        write!(f, "{}", attributes)?;
    }
    Ok(())
}

impl std::fmt::Display for AttributeList {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.trivia[0])?;
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
                present,
                trivia,
                attributes,
                name,
                items,
            } => {
                write!(f, "{}", trivia[0])?;
                write!(f, "mod")?;
                print_attributes(attributes, f)?;
                write!(f, "{}", trivia[1])?;
                write!(f, "{}", name)?;
                write!(f, "{}", trivia[2])?;

                if present.contains(ItemModulePresent::OPEN_BRACE_PRESENT) {
                    write!(f, "{{")?;
                }

                for item in items {
                    let item = item.borrow();
                    write!(f, "{}", item)?;
                }

                if present.contains(ItemModulePresent::CLOSE_BRACE_PRESENT) {
                    write!(f, "}}")?;
                }

                Ok(())
            }
        }
    }
}
