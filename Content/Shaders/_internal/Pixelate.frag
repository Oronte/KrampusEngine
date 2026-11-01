// Bigger pixels (sprites and post-process).
uniform sampler2D texture;
uniform vec2 textureSize;
uniform float pixelSize; // 4 [1, 64] size of a big pixel, in pixels of the picture

void main()
{
	vec2 cell = vec2(max(pixelSize, 1.0)) / textureSize;
	vec2 uv = (floor(gl_TexCoord[0].xy / cell) + 0.5) * cell;
	gl_FragColor = texture2D(texture, uv) * gl_Color;
}
