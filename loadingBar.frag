#version 330 core

in  vec2  vTexCoord;
in  float vTexU;
out vec4  color;

uniform sampler2D texSampler;
uniform float time;    // 0 to 1

void main() {
    if (vTexU > time) {
        discard;    
        // color = vec4(0,0,0,0); return;
    }

    // use the texture
    vec2  stretchedUV = vec2( vTexCoord.x / max(time, 0.0001), vTexCoord.y );
    color = texture(texSampler, stretchedUV);

    // alpha test
    if (color.a < 0.01) discard;
}