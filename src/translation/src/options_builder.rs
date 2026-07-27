use crate::TranslationOptions;

#[derive(Default)]
pub struct TranslationOptionsBuilder {
    options: TranslationOptions,
}

impl TranslationOptionsBuilder {
    pub fn default_debug_build_options() -> Self {
        
        // Set default debug options here
        Self::default()
    }

    pub fn default_release_build_options() -> Self {
        
        // Set default release options here
        Self::default()
    }

    pub fn build(self) -> Option<TranslationOptions> {
        Some(self.options)
    }
}
