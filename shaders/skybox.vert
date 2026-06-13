#version 460

// source: lab code

layout (location = 0) in vec3 vs_in_pos;

out vec3 vs_out_pos;

uniform mat4 world;
uniform mat4 view_proj;

void main()
{
	gl_Position = (view_proj * world * vec4(vs_in_pos, 1)).xyww;	// [x,y,w,w] => after homogeneous division [x/w, y/w, 1]

	vs_out_pos = vs_in_pos;
}
