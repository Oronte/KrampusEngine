// A line around the visible part of the picture (its transparent pixels next to opaque ones).
uniform sampler2D texture;
uniform vec2 textureSize;
uniform vec4 outlineColor; // 1, 0.85, 0.2, 1 the color of the line
uniform float thickness; // 2 [0, 8] width, in pixels of the picture

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 color = texture2D(texture, uv) * gl_Color;
	vec2 offset = thickness / textureSize;
	float around = 0.0;
	around = max(around, texture2D(texture, uv + vec2(offset.x, 0.0)).a);
	around = max(around, texture2D(texture, uv - vec2(offset.x, 0.0)).a);
	around = max(around, texture2D(texture, uv + vec2(0.0, offset.y)).a);
	around = max(around, texture2D(texture, uv - vec2(0.0, offset.y)).a);
	around = max(around, texture2D(texture, uv + offset * 0.7071).a);
	around = max(around, texture2D(texture, uv - offset * 0.7071).a);
	around = max(around, texture2D(texture, uv + vec2(offset.x, -offset.y) * 0.7071).a);
	around = max(around, texture2D(texture, uv + vec2(-offset.x, offset.y) * 0.7071).a);
	float line = clamp(around - color.a, 0.0, 1.0) * outlineColor.a;
	gl_FragColor = vec4(mix(outlineColor.rgb, color.rgb, color.a), max(color.a, line));
}
