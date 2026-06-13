#version 460

// source: lab code

in vec3 vs_out_pos;

out vec4 fs_out_col;

// const vec4 ground_color = vec4(0.5, 0.2, 0.2, 1);
// const vec4 sky_color = vec4(0.2, 0.3, 0.7, 1);

uniform samplerCube skybox_tex;

void main()
{
	/*
	// Procedural 1:
	fs_out_col = vec4( vs_out_pos, 1);
	
	// Procedural 2:
	vec3 dir = normalize(vs_out_pos);
	fs_out_col = mix( groundColor, skyColor, dir.y * 0.5 + 0.5 ); // From y [-1, 1] -> y [0, 1]
	*/

	// Skybox texture
	fs_out_col = texture(skybox_tex, vs_out_pos);
}
