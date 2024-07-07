@use "v1.0";

subsystem std {
  group GenericResource {
    m_writer: std::IWriter;
    m_reader: std::IReader;
    m_seeker: std::ISeeker;
    m_rtype: std::ResourceType;
    m_tag: std::Uuid;
    
    pub {
      fn quasipure get_writer(): std::writer => ret this.m_writer;
      fn quasipure get_reader(): std::reader => ret this.m_reader;
      fn quasipure get_seeker(): std::seeker => ret this.m_seeker;

      fn quasipure get_rtype(): std::ResourceType => ret this.m_rtype;
      fn quasipure get_tag(): std::Uuid => ret this.m_tag;

      fn quasipure read(): [u8] {
        let mut data: [u8] = [];
        this.m_reader.read(data);
        ret data;
      }

      fn retropure read(data: ![u8]) {
        this.m_reader.read(data);
      }

      fn tsafe write(data: [u8]) {
        this.m_writer.write(data);
      }

      fn tsafe seek(offset: isize, whence: std::SeekWhence = std::SeekWhence::Start) {
        this.m_seeker.seek(offset, whence);
      }
    }

  } impl [std::IWriter, std::IReader, std::IResource];

  pub fn quasipure fetch(uri: string): GenericResource? {
    let parsed = std::Uri::parse(uri);

    throwif !parsed, std::UriParseError(f"Failed to parse URI: {uri}");

    let scheme = parsed.get_scheme();

    switch (scheme) {
      case "file": {
        let file = std::File::open(parsed.get_path());

        throwif !file, std::FileOpenError(f"Failed to open file: {parsed.get_path()}");

        let reader = file.get_reader();
        let writer = file.get_writer();
        let seeker = file.get_seeker();

        let rsrc = GenericResource {
          m_writer: writer,
          m_reader: reader,
          m_seeker: seeker,
          m_rtype: std::ResourceType::File,
          m_tag: std::Uuid::new(),
        };

        ret rsrc;
      }

      case "dict":
      case "ftp":
      case "ftps":
      case "gopher":
      case "http":
      case "https":
      case "imap":
      case "imaps":
      case "ldap":
      case "ldaps":
      case "pop3":
      case "pop3s":
      case "rtmp":
      case "rtsp":
      case "scp":
      case "sftp":
      case "smb":
      case "smbs":
      case "smtp":
      case "smtps":
      case "telnet":
      case "tftp": {
        let client = std::HttpClient::open(uri);

        throwif !client, std::HttpClientOpenError(f"Failed to open HTTP client: {uri}");

        let reader = client.get_reader();
        let writer = client.get_writer();
        let seeker = client.get_seeker();

        let rsrc = GenericResource {
          m_writer: writer,
          m_reader: reader,
          m_seeker: seeker,
          m_rtype: std::ResourceType::NetworkResource,
          m_tag: std::Uuid::new(),
        };

        ret rsrc;
      }
    }
  }
}

fn main() {
  let rsrc = std::fetch("https://example.com/data.txt");

  // if !rsrc {
  //   std::println("Failed to fetch resource");
  //   ret;
  // }

  // std::println("Resource fetched successfully");
  // std::println(f"Resource tag: {rsrc.get_tag()}");
  // std::println(f"Resource type: {rsrc.get_rtype()}");
  // std::println(f"Resource data: {rsrc.read().str()}");
}