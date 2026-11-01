// Post-process: an old cathode ray screen (curved, scanlines, colors split at the borders).
uniform sampler2D texture;
uniform vec2 resolution;
uniform float time;
uniform float curvature; // 0.12 [0, 0.5]
uniform float scanlines; // 0.35 [0, 1] darkness of the lines
uniform float flicker; // 0.03 [0, 0.2]
uniform float split; // 0.0015 [0, 0.01] colors apart at the borders

void main()
{
	vec2 uv = gl_TexCoord[0].xy * 2.0 - 1.0;
	uv *= 1.0 + curvature * dot(uv, uv) * 0.25;
	uv = uv * 0.5 + 0.5;
	if (uv.x < 0.0 || uv.y < 0.0 || uv.x > 1.0 || uv.y > 1.0)
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}
	vec2 shift = (uv - 0.5) * split;
	vec3 color = vec3(texture2D(texture, uv + shift).r, texture2D(texture, uv).g, texture2D(texture, uv - shift).b);
	float line = 0.5 + 0.5 * sin(uv.y * resolution.y * 3.14159);
	color *= 1.0 - scanlines * (1.0 - line);
	color *= 1.0 - flicker * (0.5 + 0.5 * sin(time * 60.0));
	vec2 edge = uv * (1.0 - uv);
	color *= clamp(pow(edge.x * edge.y * 16.0, 0.25), 0.0, 1.0);
	gl_FragColor = vec4(color, 1.0);
}
