precision mediump float;

varying vec2 v_texCoord;

uniform sampler2D y_texture;
uniform sampler2D uv_texture;

void main(void) {
/*
	float r, g, b, y, u, v;

	y = texture2D(y_texture, v_texCoord).r;
	u = texture2D(uv_texture, v_texCoord).a - 0.5;
	v = texture2D(uv_texture, v_texCoord).r - 0.5;

	r = y + 1.13983*v;
	g = y - 0.39465*u - 0.58060*v;
	b = y + 2.03211*u;

	gl_FragColor = vec4(r, g, b, 1.0);
*/
	mediump vec3 yuv;
	lowp vec3 rgb;

	yuv.x = texture2D(y_texture, v_texCoord).r;
	yuv.yz = texture2D(uv_texture, v_texCoord).rg - vec2(0.5, 0.5);

	// BT.601, which is the standard for SDTV is provided as a reference
	/*
	rgb = mat3( 1, 1, 1,
		0, -.39465, 2.03211,
		1.13983, -.58060, 0) * yuv;
	*/
	// Using BT.709 which is the standard for HDTV
	rgb = mat3( 	1,  	 1, 	1,
			0, -.21482, 2.12798,
		  1.28033, -.38059, 0) * yuv;

	gl_FragColor = vec4(rgb, 1);
}
