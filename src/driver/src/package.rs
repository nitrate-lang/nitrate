use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize, PartialEq)]
pub struct Dependency {
    #[serde(rename = "@name")]
    name: String,
    #[serde(rename = "@major")]
    major: u32,
    #[serde(rename = "@minor")]
    minor: u32,
    #[serde(rename = "@patch")]
    patch: u32,
}

#[derive(Debug, Serialize, Deserialize, PartialEq)]
pub struct Dependencies {
    #[serde(rename = "dependency")]
    dependency: Option<Vec<Dependency>>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq)]
pub struct Version {
    #[serde(rename = "@major")]
    major: u32,
    #[serde(rename = "@minor")]
    minor: u32,
    #[serde(rename = "@patch")]
    patch: u32,
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
        std::path::Path::new("src").join("main.nit")
    }
}
