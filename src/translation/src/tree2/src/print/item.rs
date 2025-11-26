use crate::prelude::*;

pub(crate) fn print_trivia(
    trivia: &Option<Trivia>,
    f: &mut std::fmt::Formatter<'_>,
) -> std::fmt::Result {
    if let Some(trivia) = trivia {
        write!(f, "{}", trivia)?;
    }
    Ok(())
}

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
        print_trivia(&self.trivia[0], f)?;
        write!(f, "[")?;

        for (i, attr) in self.attributes.iter().enumerate() {
            let expr = attr.borrow();
            write!(f, "{}", expr)?;

            if i + 1 != self.attributes.len()
                || self
                    .flags
                    .contains(AttributeListFlags::TRAILING_COMMA_PRESENT)
            {
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

            Item::Trivia { trivia } => print_trivia(trivia, f),

            Item::Module {
                source_offset: _,
                trivia,
                attributes,
                name,
                items,
            } => {
                print_trivia(&trivia[0], f)?;
                write!(f, "mod")?;
                print_attributes(attributes, f)?;
                print_trivia(&trivia[1], f)?;
                write!(f, "{}", name)?;
                print_trivia(&trivia[2], f)?;
                write!(f, "{{")?;

                for item in items {
                    let item = item.borrow();
                    write!(f, "{}", item)?;
                }

                write!(f, "}}")
            }
        }
    }
}
