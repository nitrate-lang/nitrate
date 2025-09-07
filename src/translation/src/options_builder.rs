use crate::TranslationOptions;

#[derive(Default)]
pub struct TranslationOptionsBuilder<'a> {
    options: TranslationOptions<'a>,
}

impl<'a> TranslationOptionsBuilder<'a> {
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

    pub fn build(self) -> Option<TranslationOptions<'a>> {
        Some(self.options)
    }
}
