precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_flipTexture;
uniform sampler2D s_flopTexture;
uniform int i_flipFlop;

void main(void) {
    vec4 v4Texel;
    if( i_flipFlop == 0 ) {
        v4Texel = texture2D(s_flipTexture, v_texCoord);
    } else {
        v4Texel = texture2D(s_flopTexture, v_texCoord);
    }
    gl_FragColor = v4Texel;
}
