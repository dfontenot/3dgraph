#version 410 core

layout(location=0) in vec3 position;
//layout(location=1) in vec3 vertex_colors;

uniform float offset_x;
uniform float offset_y;

void main() {
  gl_PointSize = 5.0f;
  gl_Position = vec4(position.x + offset_x, position.y + offset_y, position.z, 1.0f);
}
