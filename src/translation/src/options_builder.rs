use crate::TranslationOptions;

#[derive(Default)]
pub struct TranslationOptionsBuilder {
    options: TranslationOptions,
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
