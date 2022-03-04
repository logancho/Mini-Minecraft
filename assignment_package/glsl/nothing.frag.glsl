#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

uniform sampler2D u_Texture;

in vec3 fs_UV;

out vec4 out_Col;

void main()
{
    // do nothing
    out_Col = texture(u_Texture, vec2(fs_UV.x, fs_UV.y));
}
