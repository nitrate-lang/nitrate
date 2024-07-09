@use "v1.0";

region UdpPacket {
  /** User Datagram Protocol **
   *  =========================
   *  The User Datagram Protocol (UDP) is one of the core members
   *  of the Internet Protocol Suite, the set of network protocols
   *  used for the Internet. With UDP, computer applications can
   *  send messages, in this case packets, to other hosts on an
   *  Internet Protocol (IP) network. Prior communications are not
   *  required in order to set up communication channels or data paths.
   *  =================================================================
   *
   *  @srcport:  The source port number.
   *  @dstport:  The destination port number.
   *  @length:   The length of the UDP packet payload in bytes.
   *  @checksum: The checksum of the UDP packet as specified in RFC 768.
   *
   *  @method sync: Update the length and checksum of the UDP packet.
   */

  pub srcport:  u16 = 0,
  pub dstport:  u16 = 0,
  pub length:   u16 = 0,
  pub checksum: u16 = 0,

  fn update(payload: [u8]) {
    this.length += payload.len() as u16;
    this.checksum = "implement checksum algorithm here".to_bytes()[0: 2] as u16;
  } req {
    in this.length + payload.len() <= 0xFFFF;
  } meta {
    do warnif<low>(unused(this) && unused(this.finalize),
                  "Did you forget to call update()? "
                  "UdpPacket::update() should be called "
                  "prior to sending the packet.");
  }

  fn finalize(payload: [u8]) {
    this.update(payload);
    this.checksum = "implement checksum algorithm here".to_bytes()[0: 2] as u16;
  } req {
    in this.length + payload.len() <= 0xFFFF;
  } meta {
    do warnif<low>(unused(),
                  "Did you forget to call finalize()? "
                  "UdpPacket::finalize() should be called "
                  "prior to sending the packet.");
  }
} impl [auto];

/*

VS C Code:

struct UdpPacket {
  uint16_t srcport;
  uint16_t dstport;
  uint16_t length;
  uint16_t checksum;
} __attribute__((packed));

*/