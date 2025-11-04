use nitrate_translation::llvm::OptLevel;
use serde::{Deserialize, Serialize};
use xml_doc::ReadOptions;

#[derive(Debug, Serialize, Deserialize, PartialEq)]
pub struct Dependency {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@major")]
    pub major: u32,
    #[serde(rename = "@minor")]
    pub minor: u32,
    #[serde(rename = "@patch")]
    pub patch: u32,
}

#[derive(Debug, Serialize, Deserialize, PartialEq)]
pub struct Dependencies {
    #[serde(rename = "dependency")]
    dependency: Option<Vec<Dependency>>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq)]
pub struct Version {
    #[serde(rename = "@major")]
    pub major: u32,
    #[serde(rename = "@minor")]
    pub minor: u32,
    #[serde(rename = "@patch")]
    pub patch: u32,
}

#[derive(Debug, Serialize, Deserialize, PartialEq)]
#[serde(rename = "package")]
pub struct Package {
    #[serde(rename = "name")]
    name: String,

    #[serde(rename = "version")]
    version: Version,

    #[serde(rename = "edition")]
    edition: u16,

    #[serde(rename = "dependencies")]
    dependencies: Dependencies,
}

impl Package {
    pub fn from_xml(xml: &str) -> Result<Self, serde_xml_rs::Error> {
        serde_xml_rs::from_str(xml)
    }

    pub fn name(&self) -> &str {
        &self.name
    }

    pub fn version(&self) -> (u32, u32, u32) {
        (self.version.major, self.version.minor, self.version.patch)
    }

    pub fn edition(&self) -> u16 {
        self.edition
    }

    pub fn entrypoint(&self) -> std::path::PathBuf {
        std::path::Path::new("src").join("entry.nit")
    }

    pub fn optimization_level(&self) -> OptLevel {
        OptLevel::Default
    }

    pub fn xml_serialize(&self) -> String {
        const XMLNS_XSI: &str = "http://www.w3.org/2001/XMLSchema-instance";
        const XSI_NO_NAMESPACE_SCHEMA_LOCATION: &str =
            "https://static.nitrate.dev/no3_package_config.xsd";

        let serialized = serde_xml_rs::to_string(self).unwrap();
        let mut document =
            xml_doc::Document::parse_str_with_opts(&serialized, ReadOptions::default()).unwrap();

        let root = document.root_element().unwrap();
        root.set_attribute(&mut document, "xmlns:xsi", XMLNS_XSI);
        root.set_attribute(
            &mut document,
            "xsi:noNamespaceSchemaLocation",
            XSI_NO_NAMESPACE_SCHEMA_LOCATION,
        );

        document.write_str().unwrap()
    }
}

pub struct PackageBuilder {
    name: String,
    version: Version,
    edition: u16,
    dependencies: Vec<Dependency>,
}

impl PackageBuilder {
    pub fn new(name: String) -> Self {
        Self {
            name,
            version: Version {
                major: 0,
                minor: 1,
                patch: 0,
            },
            edition: 2026,
            dependencies: Vec::new(),
        }
    }

    pub fn version(mut self, version: Version) -> Self {
        self.version = version;
        self
    }

    pub fn edition(mut self, edition: u16) -> Self {
        self.edition = edition;
        self
    }

    pub fn add_dependency(mut self, dependency: Dependency) -> Self {
        self.dependencies.push(dependency);
        self
    }

    pub fn build(self) -> Package {
        Package {
            name: self.name,
            version: self.version,
            edition: self.edition,
            dependencies: Dependencies {
                dependency: if self.dependencies.is_empty() {
                    None
                } else {
                    Some(self.dependencies)
                },
            },
        }
    }
}
