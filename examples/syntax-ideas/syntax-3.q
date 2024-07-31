use "v1.0";

@copyright "Wesley Jones";
@license "CC BY-NC-SA 4.0";
@description "A simple 2d platformer game";
@language "en";

@import core;
@import math as M;
@import chrono;
@import physics<2d>;
@import window;
@import graphics<2d>;

group Terrain {
    shape: [f32];     `The ground level at each position`;

    fn Terrain() {
        register_size(&shape);
    }

} : Phy.RigidBody<Surface>(), Graphics.TexturedEntity();

fn StartGame(): bool {
    catch (Exception e) {
        trace(e);
        ret false;
    }

    const fps = 60;

    let window = Window::open(title: "Platformer", 
                              w: 800, h: 600, 
                              resizable: false, 
                              pos: Window::Centered)!;

    let world = Phy.EuclideanWorldSpaceBuilder()
        .gravity(0.0, -9.8)
        .origin(0.0, 0.0)
        .build();
    
    world.add(Terrain::builder()
                .load_texture("terrain.svg")
                .pos(0.0, 500.0)
                .friction(M.normal(0.1, 0.01))
                .ultimate(M.normal(100.0, 10.0))
                .shape_from_texture()
                .build());

    window.phy_content(world);

    window.render(fps: fps, clear_color: Color::WHITE);

    ret true;
}

fn main(): i32 {
    ret StartGame() ? 0 : 1;
}