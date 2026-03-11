#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform sampler2D tex;
uniform int useTexture;
in vec2 TexCoord;

void main() {
    if (useTexture == 1) {
        FragColor = texture(tex, TexCoord);
    } else {
        FragColor = vec4(color, 1.0);
    }
}
