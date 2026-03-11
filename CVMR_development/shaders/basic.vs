#version 330 core
layout(location = 0) in vec3 aPos;
// --- image texturing additions ---
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
// --- end image texturing additions ---

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // --- image texturing additions ---
    TexCoord = aTexCoord;
    // --- end image texturing additions ---
}
