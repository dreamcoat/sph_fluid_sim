#version 440 core

uniform vec2 G;
uniform float H;
uniform float MASS;
uniform float VISC;
uniform float SPIKY_GRAD;
uniform float VISC_LAP;

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

float norm(vec2 v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}

void main()
{
	const float M_PI = 3.1415926535897932384626433832795;

	uint index = gl_GlobalInvocationID.x;
	uint particle_count = gl_NumWorkGroups.x;

	Particle pi = particles[index];

	if (pi.is_active == 0)
		return;

	vec2 fpress = vec2(0.0, 0.0);
	vec2 fvisc = vec2(0.0, 0.0);
	for (int i = 0; i < particle_count; i++)
	{
		if (i == index)
			continue;

		vec2 rij = particles[i].x - pi.x;
		float r = norm(rij);

		if (r < H)
		{
			// compute pressure force contribution
			fpress += -normalize(rij)*MASS*(pi.p + particles[i].p) / (2.0 * particles[i].rho) * SPIKY_GRAD*pow(H - r, 2.0);
			// compute viscosity force contribution
			fvisc += VISC*MASS*(particles[i].v - pi.v) / particles[i].rho * VISC_LAP*(H - r);
		}
	}
	vec2 fgrav = G * pi.rho;
	pi.f = fpress + fvisc + fgrav;

	particles[index] = pi;
}
