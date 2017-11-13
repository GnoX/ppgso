#version 330
// The final color
out vec4 FragmentColor;

// Additional overall color when not using per-vertex Color input
uniform vec3 Color;

void main() {
  // Just pass the color to the output
  FragmentColor = vec4(Color, 1.0f);
}
