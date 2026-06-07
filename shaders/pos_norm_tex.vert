#version 430

// VBO
layout( location = 0 ) in vec3 vs_in_pos;
layout( location = 1 ) in vec3 vs_in_norm;
layout( location = 2 ) in vec2 vs_in_tex;

out vec3 vs_out_pos;
out vec3 vs_out_modelpos;
out vec3 vs_out_norm;
out vec2 vs_out_tex;

// layout(std140, binding = 0) uniform UBO {
// 	mat4 world;
// 	mat4 worldIT;
// 	mat4 viewProj;
// 
// 	vec3 light_dir;
// 	vec3 La;
// 	vec3 Ld;
// };

uniform mat4 world;
uniform mat4 world_inv_trans;
uniform mat4 view_proj;

void main()
{
	gl_Position = view_proj * world * vec4(vs_in_pos, 1);

	vs_out_modelpos = vs_in_pos;
	vs_out_pos  = (world   * vec4(vs_in_pos,  1)).xyz;
	vs_out_norm = (world_inv_trans * vec4(vs_in_norm, 0)).xyz;
	vs_out_tex = vs_in_tex;
}