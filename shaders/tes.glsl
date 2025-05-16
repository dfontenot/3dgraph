#version 450 core

// reference: https://learnopengl.com/Guest-Articles/2021/Tessellation/Tessellation
layout(quads, fractional_odd_spacing, ccw) in;

out vec2 uv;

// ref: https://gist.github.com/companje/29408948f1e8be54dd5733a74ca49bb9
float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
// out vec4 tes_color;

void main() {
    // reference: https://gamedev.stackexchange.com/a/87643
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    vec4 p2 = mix(gl_in[1].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
    vec4 interpolated = mix(p1, p2, gl_TessCoord.y);

    // ref: https://www.benjoffe.com/code/tools/functions3d/examples
    // apply the function now that there are more levels of tessellation
    interpolated.z = mix(-0.5, 0.5, sin(10.0 * (pow(interpolated.x, 2) + pow(interpolated.y, 2))) / 10.0);

    // before rotation, etc. store the UV coords to use in the fragment shader
    uv = vec2(map(interpolated.x, -1.0, 1.0, 0.0, 1.0), map(interpolated.y, -1.0, 1.0, 0.0, 1.0));

    gl_Position = u_projection * u_view * u_model * interpolated;
}
