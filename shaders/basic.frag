#version 430
in vec4 fColor;
layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 brightColor;

void main(void) {
 fragColor = fColor;
 float brightness = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
 brightness > 0.1 ? (brightColor = vec4(fragColor.x, fragColor.y, fragColor.z, 1.0)) : (brightColor = vec4(0.0, 0.0, 0.0, 1.0));
 //brightness > 0.3 ? (brightColor = vec4(0.0, 0.5, 1.0, 1.0)) : (brightColor = vec4(0.0, 0.0, 0.0, 1.0));
 //fColor = vec4(0.0, 0.5, 1.0, 1.0);
}
