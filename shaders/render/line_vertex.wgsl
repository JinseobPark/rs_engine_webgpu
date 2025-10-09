// Line vertex shader for bounding box rendering

struct Uniforms {
    viewProj: mat4x4<f32>,
    model: mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

struct VertexInput {
    @location(0) position: vec3<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec3<f32>,
}

@vertex
fn main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    
    // Transform vertex position
    let worldPos = uniforms.model * vec4<f32>(input.position, 1.0);
    output.position = uniforms.viewProj * worldPos;
    
    // Yellow/orange color for selection highlight
    output.color = vec3<f32>(1.0, 0.8, 0.0);
    
    return output;
}
