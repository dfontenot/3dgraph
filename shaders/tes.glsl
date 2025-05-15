#version 450 core

// reference: https://learnopengl.com/Guest-Articles/2021/Tessellation/Tessellation
layout (quads, fractional_odd_spacing, ccw) in;
out vec4 tes_color;

void main() {
    // reference: https://gamedev.stackexchange.com/a/87643
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    vec4 p2 = mix(gl_in[1].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
    gl_Position = mix(p1, p2, gl_TessCoord.y);
}
