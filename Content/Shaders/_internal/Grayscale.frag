// Removes the colors (sprites and post-process).
uniform sampler2D texture;
uniform float amount; // 1 [0, 1] 0: normal colors, 1: all gray

void main()
{
	vec4 color = texture2D(texture, gl_TexCoord[0].xy) * gl_Color;
	float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
	gl_FragColor = vec4(mix(color.rgb, vec3(gray), amount), color.a);
}
