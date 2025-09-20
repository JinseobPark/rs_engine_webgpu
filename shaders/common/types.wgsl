// Common types and structures

struct Particle {
    position: vec3f,
    velocity: vec3f,
    density: f32,
    pressure: f32,
    force: vec3f,
    _pad: f32,
}

struct ClothParticle {
    position: vec3f,
    old_position: vec3f,
    velocity: vec3f,
    mass: f32,
    pinned: u32,
    _pad: vec3f,
}

struct SimulationParams {
    dt: f32,
    particle_count: u32,
    gravity: vec3f,
    damping: f32,
}