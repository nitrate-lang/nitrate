import no3::package;
import std::cpu;
import std::statics;
import std::time;
import std::units;

pub fn main(args: [str]) {
  no3::package::ConfigurationBuilder(args)              # Create a new package configuration
  .name("package-example")                              # Set the package name
  .license(template: std::statics::spdx::MIT, holders:  # Instantiate the MIT license template
    this.deduce_authors().map(fn(x) {                   # Deduce the authors from the git repository commit history
      ret [x.name(), std::time::year()];                # Return the author name and the current year
    }) as [str->std::time::Year]                        # Cast the authors list to a map of author names to years
  )
  .executable()                                         # Set the package as an executable
  .discover_dependencies()                              # Discover the package dependencies
  .for_platforms(["linux", "darwin", "windows"])        # Set the supported platforms
  .cpu_norm(std::cpu::get_host_cores().map(fn(x) {      # Optimize for these CPU features (but still run on older CPUs)
    ret x.features().join(",");                         # Return the CPU features list
  }).unique().join(","))                                # Join the unique CPU features list
  .mem_norm(min: 16GB)                                  # The optimizer will assume at least 16GB of RAM
  .disk_norm(min: 32GB)                                 # The optimizer will assume at least 32GB of disk space
  .assert_invariants()                                  # Assert that the configuration is valid
  .build()                                              # Build the package configuration
  .finish();                                            # Writes the configuration to a location provided in 'args'
}
