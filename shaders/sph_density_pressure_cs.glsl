#version 440 core

uniform float H;
uniform float REST_DENS;
uniform float GAS_CONST;
uniform float MASS;
uniform float POLY6;

struct Particle
{
	vec2 x;		// position
	vec2 v;		// velocity
	vec2 f;		// force
	float rho;	// density
	float p;	// pressure
	int is_active;
	int pad;
};

// Bind the particle buffer to index 0.
layout(std430, binding = 0) buffer ParticleBuffer
{
	Particle particles[];
};

// Declare the group size.
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

float squared_norm(vec2 v)
{
	return v.x * v.x + v.y * v.y;
}

void main()
{
	const float M_PI = 3.1415926535897932384626433832795;
	const float HSQ = H*H; // radius^2 for optimization

	uint index = gl_GlobalInvocationID.x;
	uint particle_count = gl_NumWorkGroups.x;

	Particle pi = particles[index];

	if (pi.is_active == 0)
		return;

	pi.rho = 0.0;
	for (int i = 0; i < particle_count; i++)
	{
		if (particles[i].is_active == 0)
			continue;

		vec2 rij = particles[i].x - pi.x;
		float r2 = squared_norm(rij);

		if (r2 < HSQ)
		{
			// this computation is symmetric
			pi.rho += MASS*POLY6*pow(HSQ - r2, 3.0);
		}
	}
	pi.p = GAS_CONST*(pi.rho - REST_DENS);

	particles[index] = pi;
}