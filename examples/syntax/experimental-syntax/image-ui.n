// @import orbit::image;

@use "v1.0";

fn main(): i32 {
	var catPic: PngImage = ImageLoader<PngImage>::load("C:\Users\Me\Pictures\mycat.png");  ~> Load cat image
	var window: UIScreen = 																  ~> Create GUI Window
				Window::create(
				bg: "#adadadad", fg: "#04040404", 
				fontFamily: "Poppins", fontSize: 12,
				fps: "40+ADP"
				);
	window << (UIImage::from(catPic).scale(mode: "full"));  ~> Add image to window
	window.render(action: fn (scr: !UIScreen)     					~> Enter render loop and set callback
	{
    scr.SetTitle(f"Frame: {scr.frame_ctr}");
  });

	ret 0;
}