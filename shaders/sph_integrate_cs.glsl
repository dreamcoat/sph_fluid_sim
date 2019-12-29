#version 440 core

uniform float DT;
uniform float H;
uniform float BOUND_DAMPING;
uniform vec2 boundary_size;

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

void main()
{
	const float EPS = H; // boundary epsilon

	uint index = gl_GlobalInvocationID.x;

	Particle p = particles[index];

	if (p.is_active == 0)
		return;

	// forward Euler integration
	p.v += DT*p.f / p.rho;
	p.x += DT*p.v;

	// enforce boundary conditions
	if (p.x.x - EPS < 0.0)
	{
		p.v.x *= BOUND_DAMPING;
		p.x.x = EPS;
	}
	if (p.x.x + EPS > boundary_size.x)
	{
		p.v.x *= BOUND_DAMPING;
		p.x.x = boundary_size.x - EPS;
	}
	if (p.x.y - EPS < 0.0)
	{
		p.v.y *= BOUND_DAMPING;
		p.x.y = EPS;
	}
	if (p.x.y + EPS > boundary_size.y)
	{
		p.v.y *= BOUND_DAMPING;
		p.x.y = boundary_size.y - EPS;
	}

	particles[index] = p;
}