// retro effect (crt)
#version 330 core

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

const float renderWidth = 800;
const float renderHeight = 600;
float offset = 0.0;

void main()
{
  float frequency = renderHeight/10.0;

  float tval = 0.0;
  vec2 uv = 0.5 + (fragTexCoord - 0.5)*(0.9 + 0.01*sin(0.5*tval));

  vec4 color = texture(texture0, fragTexCoord) * colDiffuse * fragColor;

  float modifier = 0.5;
  float sin_val = sin(uv.y * renderHeight * modifier);
  float cos_val = cos(uv.y * renderHeight * modifier);

  color.g *= (sin_val + 1) * 0.45 + 1;
  color.rb *= (cos_val + 1) * 0.30 + 1;
  // color *= sin(uv.y * renderHeight * 0.5);

  finalColor = color;
}
