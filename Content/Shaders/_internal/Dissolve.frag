// Burns the sprite away with a glowing edge (animate amount from 0 to 1).
uniform sampler2D texture;
uniform float amount; // 0.4 [0, 1] 0: whole, 1: gone
uniform vec4 edgeColor; // 1, 0.55, 0.1, 1 the color of the burning edge
uniform float edgeWidth; // 0.08 [0, 0.3]
uniform float scale; // 8 [1, 40] number of holes across the picture

float hash(vec2 p)
{
	return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p)
{
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), u.x), mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 color = texture2D(texture, uv) * gl_Color;
	float n = noise(uv * scale) * 0.75 + noise(uv * scale * 2.0) * 0.25;
	if (n < amount)
		discard;
	float burn = smoothstep(amount, amount + edgeWidth, n);
	gl_FragColor = vec4(mix(edgeColor.rgb, color.rgb, burn), color.a);
}
