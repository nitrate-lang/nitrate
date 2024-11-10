@use "v1.0";

subsystem linux::kern::DRM: [linux::kern::drm::util, linux::kern::vbuf_api] {
  group VModeResolution {
    pub width: usize,
    pub height: usize,

    static fn to_cute(this: VModeResolution, ss: !string) {
      ss << f"{this.width}x{this.height}";
    } with [lang];

    static fn from_cute(ss: string): VModeResolution? {
      let w: usize;
      let h: usize;

      if ss.scan("{usize}x{usize}", w, h) {
        ret VModeResolution( width: w, height: h );
      } else {
        ret "Failed to parse VModeResolution";
      }
    } with [lang];
  };

  group VModeColorDepth {
    pub depth: u16,
    pub bpp: u16,

    static fn to_cute(this: VModeColorDepth, ss: !string) {
      ss << f"{this.bpp}bps";
    } with [lang];

    static fn from_cute(ss: string): VModeColorDepth? {
      let bpp: u16;

      if ss.scan("{u16}bps", bpp) {
        ret VModeColorDepth(depth: bpp, bpp: bpp);
      } else {
        ret "Failed to parse VModeColorDepth";
      }
    } with [lang];
  };

  group VModeName {
    pub res: VModeResolution,
    pub color: VModeColorDepth,
    pub refresh: u16,

    static fn to_cute(this: VModeName, ss: !string) {
      ss << f"[{this.res}:{this.color}]/{this.refresh}Hz";
    } with [lang];

    static fn from_cute(ss: string): VModeName? {
      let res: VModeResolution;
      let color: VModeColorDepth;
      let refresh: u16;

      if ss.scan("[{VModeResolution}:{VModeColorDepth}]/{u16}Hz", res, color, refresh) {
        ret VModeName(res: res, color: color, refresh: refresh);
      } else {
        ret "Failed to parse VModeName";
      }
    } with [lang];
  };

  group VBufLFB {
    pub base: *void,
    pub stride: usize,
    pub height: usize,
  };

  group IDirectRenderingManager {
    /* ========== (DE)INIT DIRECT RENDERING MANAGER ========== */
    pub fn impure noexcept drm_init(): kcode;
    pub fn impure noexcept drm_deinit(): kcode;

    /* ================= VIDEO BUFFER API V1 ================= */
    pub fn impure noexcept vbuf_lfb_create(): kcode;
    pub fn impure noexcept vbuf_lfb_destroy(): kcode;
    pub fn quasipure noexcept vbuf_lfb_get(): VBufLFB?;

    /* ================== VIDEO MODE API V1 ================== */
    pub fn impure noexcept vmode_set(mode: VModeName): kcode;
    pub fn quasipure noexcept vmode_get(): VModeName;
  } with [interface];
}
