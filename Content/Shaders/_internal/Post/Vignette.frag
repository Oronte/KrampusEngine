// Post-process: darkens the borders of the screen.
uniform sampler2D texture;
uniform vec2 resolution;
uniform float intensity; // 0.6 [0, 1]
uniform float radius; // 0.75 [0, 1.5] where the darkening starts
uniform float softness; // 0.45 [0.01, 1]
uniform vec3 color; // 0, 0, 0 the color of the borders

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 scene = texture2D(texture, uv);
	vec2 p = (uv - 0.5) * vec2(resolution.x / resolution.y, 1.0);
	float light = smoothstep(radius, radius - softness, length(p));
	gl_FragColor = vec4(mix(color, scene.rgb, mix(1.0, light, intensity)), 1.0);
}
