// SPH Density Calculation
// Platform-specific defines will be injected by C++:
// #define MAX_PARTICLES 32768
// #define WORKGROUP_SIZE 64

@group(0) @binding(0) var<storage, read_write> particles: array<Particle>;

@compute @workgroup_size(WORKGROUP_SIZE, 1, 1)
fn main(@builtin(global_invocation_id) id: vec3u) {
    let particle_id = id.x;
    if (particle_id >= MAX_PARTICLES) { return; }

    var density = 0.0;
    let h = 2.0; // Smoothing radius

    for (var i = 0u; i < MAX_PARTICLES; i++) {
        let distance = length(particles[particle_id].position - particles[i].position);
        if (distance < h) {
            // Poly6 kernel
            let q = distance / h;
            let kernel = 315.0 / (64.0 * 3.14159 * pow(h, 9.0)) * pow(h * h - distance * distance, 3.0);
            density += kernel;
        }
    }

    particles[particle_id].density = density;
    particles[particle_id].pressure = density * 0.1; // Simple pressure calculation
}