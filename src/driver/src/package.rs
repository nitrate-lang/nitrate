use serde::{Deserialize, Serialize};

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
    pub dependency: Option<Vec<Dependency>>,
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
    pub name: String,

    #[serde(rename = "version")]
    pub version: Version,

    #[serde(rename = "edition")]
    pub edition: u16,

    #[serde(rename = "dependencies")]
    pub dependencies: Dependencies,
}

impl Package {
    pub fn from_xml(xml: &str) -> Result<Self, serde_xml_rs::Error> {
        serde_xml_rs::from_str(xml)
    }
}
