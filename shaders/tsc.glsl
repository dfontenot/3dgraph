#version 410 core

layout (vertices=4) out;
out vec4 vertex_color[];
uniform uint u_tess_level;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // once per patch
    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = u_tess_level;
        gl_TessLevelOuter[1] = u_tess_level;
        gl_TessLevelOuter[2] = u_tess_level;
        gl_TessLevelOuter[3] = u_tess_level;

        gl_TessLevelInner[0] = u_tess_level;
        gl_TessLevelInner[1] = u_tess_level;
    }
}
