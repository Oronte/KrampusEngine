// Makes the picture wave (water, heat, a flag).
uniform sampler2D texture;
uniform float time;
uniform float amplitude; // 0.02 [0, 0.2] how far it moves, part of the picture
uniform float frequency; // 12 [0, 60] number of waves
uniform float speed; // 3 [0, 20]

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	uv.x += sin(uv.y * frequency + time * speed) * amplitude;
	gl_FragColor = texture2D(texture, uv) * gl_Color;
}
