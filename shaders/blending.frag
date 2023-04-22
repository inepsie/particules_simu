#version 430

in vec2 vsoTexCoord;

uniform sampler2D scene;
uniform sampler2D bloom;

layout (location = 0) out vec4 fragColor;

void main(void) {
 //fragColor = vec4(1.0, 0.0, 0.0, 1.0);
 fragColor = texture(scene, vsoTexCoord) + texture(bloom, vsoTexCoord);
 //fragColor = texture(scene, vsoTexCoord);
 //fragColor = texture(bloom, vsoTexCoord);
}
