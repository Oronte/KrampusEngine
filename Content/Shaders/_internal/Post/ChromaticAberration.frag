// Post-process: red and blue drift apart toward the borders, like a cheap lens.
uniform sampler2D texture;
uniform float amount; // 0.004 [0, 0.03]

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec2 shift = (uv - 0.5) * amount;
	gl_FragColor = vec4(texture2D(texture, uv + shift).r, texture2D(texture, uv).g, texture2D(texture, uv - shift).b, 1.0);
}
