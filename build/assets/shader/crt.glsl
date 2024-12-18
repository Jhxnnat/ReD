// retro effect (crt)
#version 330 core

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

//---//
uniform float renderHeight;

void main()
{
    vec2 uv = fragTexCoord;
    uv = uv * 2.0 - 1.0;
    float curvature = 6.0;
    vec2 offset = uv.yx / curvature;
    uv = uv + uv * offset * offset;
    uv = uv * 0.5f + 0.5f;

    vec4 color = texture(texture0, uv) * colDiffuse * fragColor;
    if (uv.x <= 0.0 || 1.0 <= uv.x || uv.y <= 0.0 || 1.0 <= uv.y) color = vec4(0.0);

    float modifier = 1.0;
    float sin_val = sin(uv.y * renderHeight * modifier);
    float cos_val = cos(uv.y * renderHeight * modifier);
    color.g *= (sin_val + 1) * 0.25 + 1;
    color.rb *= (cos_val + 1) * 0.15 + 1;

    finalColor = color;
}
