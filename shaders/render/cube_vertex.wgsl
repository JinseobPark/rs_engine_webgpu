struct VertexInput {
    @location(0) position: vec3f,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
}

struct Uniforms {
    view_proj: mat4x4f,
    model: mat4x4f,
    time: f32,
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;

    // Transform position
    let world_pos = uniforms.model * vec4f(input.position, 1.0);
    output.position = uniforms.view_proj * world_pos;

    // Generate color based on position and time for animation
    output.color = vec3f(
        abs(sin(input.position.x + uniforms.time)),
        abs(sin(input.position.y + uniforms.time * 1.2)),
        abs(sin(input.position.z + uniforms.time * 0.8))
    );

    return output;
}