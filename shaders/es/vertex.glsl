#version 300 es

// xy plane only
layout(location = 0) in vec2 position;
out highp vec2 uv;

// ref: https://gist.github.com/companje/29408948f1e8be54dd5733a74ca49bb9
float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

const float eps = 0.00001;
float skip_zero(float x) {
    if (x > eps || x < -eps) {
        return x;
    }
    else if (x >= 0.0) {
        return eps;
    }
    else {
        return -eps;
    }
}

// panning controls
uniform float u_offset_x;
uniform float u_offset_y;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

// function params
uniform float u_z_mult;

void main() {
    uv = vec2(map(position.x, -1.0, 1.0, 0.0, 1.0), map(position.y, -1.0, 1.0, 0.0, 1.0));
    float z =
        map(sin(10.0 * (pow(position.x, 2.0) + pow(position.y, 2.0))) / skip_zero(u_z_mult), -1.0, 1.0, -0.5, 0.5);

    gl_Position = u_projection * u_view * u_model * vec4(position.x + u_offset_x, position.y + u_offset_y, z, 1.0f);
}
