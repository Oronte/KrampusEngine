// Turns the colors around the color wheel (a red enemy made blue) and changes their strength.
uniform sampler2D texture;
uniform float hue; // 0.5 [0, 1] turn of the color wheel (0.5: opposite colors)
uniform float saturation; // 1 [0, 2] strength of the colors
uniform float brightness; // 1 [0, 2]

vec3 toHsv(vec3 c)
{
	vec4 k = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, k.wz), vec4(c.gb, k.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
	float d = q.x - min(q.w, q.y);
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + 1e-10)), d / (q.x + 1e-10), q.x);
}

vec3 toRgb(vec3 c)
{
	vec4 k = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
	return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

void main()
{
	vec4 color = texture2D(texture, gl_TexCoord[0].xy) * gl_Color;
	vec3 hsv = toHsv(color.rgb);
	hsv.x = fract(hsv.x + hue);
	hsv.y = clamp(hsv.y * saturation, 0.0, 1.0);
	hsv.z = hsv.z * brightness;
	gl_FragColor = vec4(toRgb(hsv), color.a);
}
