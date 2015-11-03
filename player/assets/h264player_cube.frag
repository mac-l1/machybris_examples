precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D y_texture;

void main(void) {
    vec4 v4Texel = texture2D(y_texture, v_texCoord);
    gl_FragColor = v4Texel;
}
