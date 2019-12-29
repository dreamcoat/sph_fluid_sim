#pragma once

#include <GL/glew.h>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include <Eigen/Dense>

#include "exception.h"
#include "gl_shader.h"

#define GLT_MANUAL_VIEWPORT
#define GLT_IMPLEMENTATION
#include "gltext.h"

#include "gl_shader.h"

using namespace std;
using namespace Eigen;


struct Particle
{
	Particle() :
		x{ 0.0f, 0.0f },
		v{ 0.0f, 0.0f },
		f{ 0.0f, 0.0f },
		rho(0.0f),
		p(0.0f),
		active(0) {}

	Particle(float posx, float posy, bool activate) :
		x{ posx, posy },
		v{ 0.0f, 0.0f },
		f{ 0.0f, 0.0f },
		rho(0.0f),
		p(0.0f),
		active(activate ? 1 : 0) {}

	Particle(float posx, float posy, float velx, float vely, bool activate) :
		x{ posx, posy },
		v{ velx, vely },
		f{ 0.0f, 0.0f },
		rho(0.0f),
		p(0.0f),
		active(activate ? 1 : 0) {}

	float x[2];		// position
	float v[2];		// velocity
	float f[2];		// force
	float rho;		// density
	float p;		// pressure
	int active;
	int pad;
};

class sph_sim
{
public:
	sph_sim(GLsizei window_size[2]);

	void render();
	void init_particles();
	void step_particles();

	int particle_count() const { return next_free_particle_index; }

	void add_particle_block();

	void resize_window(GLsizei window_size[2]);

private:
	void draw_particles();

	const static int MAX_PARTICLES = 256 * 256;
	const static int BLOCK_PARTICLES = 32 * 32;
	const static int DAM_PARTICLES = 150 * 150;

	const float G_SCALE = 12000;

	// solver parameters
	Vector2f G;	// external (gravitational) forces
	const float REST_DENS; // rest density
	const float GAS_CONST; // const for equation of state
	const float H; // kernel radius
	const float HSQ; // radius^2 for optimization
	const float MASS; // assume all particles have the same mass
	const float VISC; // viscosity constant
	const float DT; // integration timestep

	 // smoothing kernels defined in M�ller and their gradients
	const float POLY6;
	const float SPIKY_GRAD;
	const float VISC_LAP;

	// simulation parameters
	const float EPS;	// boundary epsilon
	const float BOUND_DAMPING;

	GLsizei m_window_size[2];

	const Vector2f boundary_size;
	
	std::vector<Particle> particles;
	int next_free_particle_index;

	GLuint particles_vao;
	GLuint particles_vbo;

	GLuint particle_index_buf_bind = 0;

	gl_shader draw_particles_sha;
	GLuint particle_vs_boundary_size_unif;

	gl_shader density_pressure_sha;
	GLuint density_pressure_H_unif;
	GLuint density_pressure_REST_DENS_unif;
	GLuint density_pressure_GAS_CONST_unif;
	GLuint density_pressure_MASS_unif;
	GLuint density_pressure_POLY6_unif;

	gl_shader forces_sha;
	GLuint forces_H_unif;
	GLuint forces_MASS_unif;
	GLuint forces_G_unif;
	GLuint forces_VISC_unif;
	GLuint forces_SPIKY_GRAD_unif;
	GLuint forces_VISC_LAP_unif;

	gl_shader integrate_sha;
	GLuint integrate_boundary_size_unif;
	GLuint integrate_DT_unif;
	GLuint integrate_H_unif;
	GLuint integrate_BOUND_DAMPING_unif;
};