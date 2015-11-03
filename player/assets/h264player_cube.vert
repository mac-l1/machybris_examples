attribute vec4 a_position;
attribute vec2 a_texCoord;
uniform mat4 u_m4Texture;
varying vec2 v_texCoord;

void main()
{
    gl_Position = a_position;
    v_texCoord = vec2(u_m4Texture * vec4(a_texCoord, 0.0, 1.0));
} 
