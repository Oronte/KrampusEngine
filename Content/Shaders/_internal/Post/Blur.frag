// Post-process: a soft blur of the whole screen (a pause menu, a dream).
uniform sampler2D texture;
uniform vec2 resolution;
uniform float radius; // 2 [0, 8] distance between the samples, in pixels

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec2 offset = radius / resolution;
	vec4 sum = vec4(0.0);
	float total = 0.0;
	for (int x = -2; x <= 2; ++x)
	{
		for (int y = -2; y <= 2; ++y)
		{
			float weight = 1.0 / (1.0 + float(x * x + y * y));
			sum += texture2D(texture, uv + vec2(float(x), float(y)) * offset) * weight;
			total += weight;
		}
	}
	gl_FragColor = vec4((sum / total).rgb, 1.0);
}
