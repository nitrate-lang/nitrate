use crate::TranslationOptions;

#[derive(Debug, Clone)]
pub struct TranslationOptionsBuilder {
    options: TranslationOptions,
}

impl Default for TranslationOptionsBuilder {
    fn default() -> Self {
        Self {
            options: TranslationOptions {
                source_name_for_debug_messages: String::default(),
            },
        }
    }
}

impl TranslationOptionsBuilder {
    pub fn default_debug_build_options() -> Self {
        let b = Self::default();
        // Set default debug options here
        b
    }

    pub fn default_release_build_options() -> Self {
        let b = Self::default();
        // Set default release options here
        b
    }

    pub fn build(self) -> Option<TranslationOptions> {
        Some(self.options)
    }
}
