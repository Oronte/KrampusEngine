// Covers the sprite with a color: the flash of a hit (set amount to 1, then bring it back to 0).
uniform sampler2D texture;
uniform vec4 flashColor; // 1, 1, 1, 1 the color of the flash
uniform float amount; // 0 [0, 1] 0: normal, 1: all flash color

void main()
{
	vec4 color = texture2D(texture, gl_TexCoord[0].xy) * gl_Color;
	gl_FragColor = vec4(mix(color.rgb, flashColor.rgb, amount * flashColor.a), color.a);
}
