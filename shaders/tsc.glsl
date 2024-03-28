#version 450 core

layout (vertices=4) out;
in vec4 theColor[];
out vec4 vertex_color[];

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // once per patch
    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 3.0;
        gl_TessLevelOuter[1] = 5.0;
        gl_TessLevelOuter[2] = 3.0;
        gl_TessLevelOuter[3] = 5.0;

        gl_TessLevelInner[0] = 9.0;
        gl_TessLevelInner[1] = 7.0;
    }

    vertex_color[gl_InvocationID] = theColor[gl_InvocationID];
}
