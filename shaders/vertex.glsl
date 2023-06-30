#version 410

void main() {
  gl_Position = vec4(in_Position.x, in_Position.y, 0.0, 1.0);
}
