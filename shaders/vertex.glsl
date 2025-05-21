#version 410 core

layout(location = 0) in vec3 position;

// panning controls
uniform float u_offset_x;
uniform float u_offset_y;

void main() {
    // gl_PointSize = 5.0f;
    gl_Position = vec4(position.x + u_offset_x, position.y + u_offset_y, position.z, 1.0f);
}
