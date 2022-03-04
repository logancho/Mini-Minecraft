#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

in vec4 vs_Pos;
in vec3 vs_UV;

out vec3 fs_UV;

void main()
{
    fs_UV = vs_UV;

    //built-in things to pass down the pipeline
    gl_Position = vs_Pos;

}
