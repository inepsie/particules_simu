#version 430

layout (location = 0) out vec4 fragColor;

in vec2 vsoTexCoord;

uniform sampler2D tex;
uniform vec2 pas;
uniform bool horizontal;
uniform bool first_pass;
uniform int level_max;

vec4 flouGauss() {
 int level = 0;
 int level_min = 0;
 const float w[7] = {0.006, 0.061, 0.242, 0.383, 0.242, 0.061, 0.006};
 float h_offset[7] = {-3.0 * pas.x, -2.0 * pas.x, -pas.x, 0.0, pas.x, 2.0 * pas.x, 3.0 * pas.x};
 float v_offset[7] = {-3.0 * pas.y, -2.0 * pas.y, -pas.y, 0.0, pas.y, 2.0 * pas.y, 3.0 * pas.y};
 vec3 res;
 vec4 c;
 if(first_pass) level = level_max;
 if(first_pass) level_min = 0;
 while(level >= level_min){
  if(horizontal){
   for(int i=0 ; i<7 ; ++i){
    c = texture(tex, vsoTexCoord + vec2(h_offset[i], 0.0), level);
    res += c.rgb * w[i];
   }
  }
  else{
   for(int i=0 ; i<7 ; ++i){
    c = texture(tex, vsoTexCoord + vec2(0.0, v_offset[i]), level);
    res += c.rgb * w[i];
   }
  }
  --level;
 }
 //res = res / float((level + 1));
 //res = res * 50.5;
 return vec4(res, 1.0);
}

void main(void) {
 fragColor = flouGauss();
}
