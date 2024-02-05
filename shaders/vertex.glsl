#version 410 core

layout(location=0) in vec2 position;
//layout(location=1) in vec3 vertex_colors;

void main() {
  gl_Position = vec4(position.x, position.y, 0.0, 1.0f);
}
