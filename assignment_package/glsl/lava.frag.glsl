#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments


uniform sampler2D u_Texture;

in vec3 fs_UV;

out vec4 out_Col;

void main()
{
    // interpolate between red and the input color
    vec4 color = texture(u_Texture, vec2(fs_UV.x, fs_UV.y));
    vec4 red = vec4(1.f, 0.f, 0.f, 1.f);
    out_Col = red * 0.4 + color * 0.6;
}
