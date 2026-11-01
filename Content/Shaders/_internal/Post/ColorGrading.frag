// Post-process: the mood of the picture (brightness, contrast, colors, a tint).
uniform sampler2D texture;
uniform float brightness; // 0 [-0.5, 0.5]
uniform float contrast; // 1.1 [0, 2]
uniform float saturation; // 1.1 [0, 2]
uniform vec3 tint; // 1, 0.97, 0.92 multiplies every color (warm, cold...)
uniform float gamma; // 1 [0.4, 2.5]

void main()
{
	vec3 color = texture2D(texture, gl_TexCoord[0].xy).rgb;
	color = (color - 0.5) * contrast + 0.5 + brightness;
	float gray = dot(color, vec3(0.299, 0.587, 0.114));
	color = mix(vec3(gray), color, saturation) * tint;
	color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma));
	gl_FragColor = vec4(color, 1.0);
}
