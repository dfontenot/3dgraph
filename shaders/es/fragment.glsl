#version 300 es

layout(location = 0) out vec4 frag_color;
in vec2 uv;

void main() {
    // ref: https://stackoverflow.com/a/64657127
    vec2 repeat = vec2(5.0, 5.0); // repeat in x and y direction
    float result = mod(dot(vec2(1.0), step(vec2(0.5), fract(uv * repeat))), 2.0);
    frag_color = mix(vec4(0.0, 0.0, 1.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), result);
}
