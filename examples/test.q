// @use "v1.0";

pub "lua" subsystem module {
  group Info {
    id: std::uuid,
    name: string,
    version: (u16, u16, u16),
    desc: string,
    authors: {string},
    spdx_license: string,
    dependencies: {std::uuid},
    categories: {string},
    tags: {string},
  };

  /**
    * Load a module by its id.
    * @param id The id of the module to load.
    * @return True if the module was loaded successfully, false otherwise.
    */
  fn impure noexcept tsafe load(id: std::uuid): bool;
  
  /**
    * Unload a module by its id.
    * @param id The id of the module to unload.
    * @return True if the module was unloaded successfully, false otherwise.
    */
  fn impure noexcept tsafe unload(id: std::uuid): bool;
  
  /**
    * Get the list of loaded modules.
    * @return The list of loaded modules.
    */
  fn impure noexcept tsafe list(): {std::uuid};
  
  /**
    * Get information about a module by its id.
    * @param id The id of the module to get information about.
    * @return The information about the module, or null if the module is not loaded.
    */
  fn impure noexcept tsafe info(id: std::uuid): Info?;
  
  /**
    * Check if a module is loaded.
    * @param id The id of the module to check.
    * @return True if the module is loaded, false otherwise.
    */
  fn impure noexcept tsafe contains(id: std::uuid): bool;
  
  /**
    * Check if a module is available.
    * @param id The id of the module to check.
    * @return True if the module is available, false otherwise.
    */
  fn impure noexcept tsafe available(id: std::uuid): bool;
} impl [stable];
