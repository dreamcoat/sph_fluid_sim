#include "sph_sim.h"


sph_sim::sph_sim(GLsizei window_size[2]) :
	m_window_size{ window_size[0], window_size[1] },
	boundary_size(800, 800),

	next_free_particle_index(0),

	particles(MAX_PARTICLES),

	G(0.0f, G_SCALE * /*-9.8f*/-6),

	REST_DENS(1000.f),
	GAS_CONST(2000.f),
	H(16.f),
	HSQ(H*H),
	MASS(65.f),
	VISC(250.f),
	DT(/*0.0008f*/0.00087f),

	POLY6(315.f / (65.f*(float)M_PI*pow(H, 9.f))),
	SPIKY_GRAD(-45.f / ((float)M_PI*pow(H, 6.f))),
	VISC_LAP(45.f / ((float)M_PI*pow(H, 6.f))),

	EPS(H),
	BOUND_DAMPING(-0.5f)
{
}

void sph_sim::draw_particles()
{
	draw_particles_sha.use();

	glViewport(0, 0, m_window_size[0], m_window_size[1]);

	glEnable(GL_PROGRAM_POINT_SIZE);

	glBindVertexArray(particles_vao);
	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glUniform2f(particle_vs_boundary_size_unif, boundary_size[0], boundary_size[1]);
	glDrawArrays(GL_POINTS, 0, next_free_particle_index);
	glFinish();
}

void sph_sim::render()
{
	glClearColor(0.9f, 0.9f, 0.9f, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	draw_particles();
}

void sph_sim::step_particles()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, particle_index_buf_bind, particles_vbo);

	density_pressure_sha.use();
	glUniform1f(density_pressure_H_unif, H);
	glUniform1f(density_pressure_REST_DENS_unif, REST_DENS);
	glUniform1f(density_pressure_GAS_CONST_unif, GAS_CONST);
	glUniform1f(density_pressure_MASS_unif, MASS);
	glUniform1f(density_pressure_POLY6_unif, POLY6);
	glDispatchCompute(next_free_particle_index, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

	forces_sha.use();
	glUniform2f(forces_G_unif, G[0], G[1]);
	glUniform1f(forces_H_unif, H);
	glUniform1f(forces_MASS_unif, MASS);
	glUniform1f(forces_VISC_unif, VISC);
	glUniform1f(forces_SPIKY_GRAD_unif, SPIKY_GRAD);
	glUniform1f(forces_VISC_LAP_unif, VISC_LAP);
	glDispatchCompute(next_free_particle_index, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

	integrate_sha.use();
	glUniform2f(integrate_boundary_size_unif, boundary_size[0], boundary_size[1]);
	glUniform1f(integrate_DT_unif, DT);
	glUniform1f(integrate_H_unif, H);
	glUniform1f(integrate_BOUND_DAMPING_unif, BOUND_DAMPING);
	glDispatchCompute(next_free_particle_index, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void sph_sim::init_particles()
{
	for (auto& p : particles)
		p = Particle(0.0f, 0.0f, 0.0f, 0.0f, false);

	// Create initial dam of particles.
	for (float y = H; y < boundary_size[1] - EPS*2.f; y += H)
		for (float x = EPS; x <= boundary_size[0] / 2; x += H)
			if (next_free_particle_index < DAM_PARTICLES)
			{
				float jitter = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				particles[next_free_particle_index++] = Particle(x + jitter, y, true);
			}

	// particle's vao
	glGenVertexArrays(1, &particles_vao);
	glBindVertexArray(particles_vao);

	// particle buffer
	glGenBuffers(1, &particles_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW);

	// Add attributes/uniforms and initialise the shader.
	draw_particles_sha.add_attribute("position");
	draw_particles_sha.add_uniform("boundary_size");
	draw_particles_sha.init_vs_fs_from_file("shaders/particle_vs.glsl", "shaders/particle_fs.glsl");

	// After initialization the attribute/uniform locations can be retrieved.
	GLuint pos_attrib = draw_particles_sha.get_attribute("position");
	particle_vs_boundary_size_unif = draw_particles_sha.get_uniform("boundary_size");

	// Position attribute.
	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)0);
	glEnableVertexAttribArray(pos_attrib);

	density_pressure_sha.add_uniform("H");
	density_pressure_sha.add_uniform("REST_DENS");
	density_pressure_sha.add_uniform("GAS_CONST");
	density_pressure_sha.add_uniform("MASS");
	density_pressure_sha.add_uniform("POLY6");
	density_pressure_sha.init_cs_from_file("shaders/sph_density_pressure_cs.glsl");
	density_pressure_H_unif = density_pressure_sha.get_uniform("H");
	density_pressure_REST_DENS_unif = density_pressure_sha.get_uniform("REST_DENS");
	density_pressure_GAS_CONST_unif = density_pressure_sha.get_uniform("GAS_CONST");
	density_pressure_MASS_unif = density_pressure_sha.get_uniform("MASS");
	density_pressure_POLY6_unif = density_pressure_sha.get_uniform("POLY6");

	forces_sha.add_uniform("H");
	forces_sha.add_uniform("G");
	forces_sha.add_uniform("MASS");
	forces_sha.add_uniform("VISC");
	forces_sha.add_uniform("SPIKY_GRAD");
	forces_sha.add_uniform("VISC_LAP");
	forces_sha.init_cs_from_file("shaders/sph_forces_cs.glsl");
	forces_H_unif = forces_sha.get_uniform("H");
	forces_G_unif = forces_sha.get_uniform("G");
	forces_MASS_unif = forces_sha.get_uniform("MASS");
	forces_VISC_unif = forces_sha.get_uniform("VISC");
	forces_SPIKY_GRAD_unif = forces_sha.get_uniform("SPIKY_GRAD");
	forces_VISC_LAP_unif = forces_sha.get_uniform("VISC_LAP");

	integrate_sha.add_uniform("boundary_size");
	integrate_sha.add_uniform("DT");
	integrate_sha.add_uniform("H");
	integrate_sha.add_uniform("BOUND_DAMPING");
	integrate_sha.init_cs_from_file("shaders/sph_integrate_cs.glsl");
	integrate_boundary_size_unif = integrate_sha.get_uniform("boundary_size");
	integrate_DT_unif = integrate_sha.get_uniform("DT");
	integrate_H_unif = integrate_sha.get_uniform("H");
	integrate_BOUND_DAMPING_unif = integrate_sha.get_uniform("BOUND_DAMPING");
}

void sph_sim::add_particle_block()
{
	if (next_free_particle_index >= MAX_PARTICLES)
		std::cout << "maximum number of particles reached" << std::endl;
	else
	{
		std::vector<Particle> particle_block;
		unsigned int placed = 0;
		for (float y = boundary_size[1] / 1.5f - boundary_size[1] / 5.f; y < boundary_size[1] / 1.5f + boundary_size[1] / 5.f; y += H*0.95f)
			for (float x = boundary_size[0] / 2.f - boundary_size[1] / 5.f; x <= boundary_size[0] / 2.f + boundary_size[1] / 5.f; x += H*0.95f)
				if (placed < BLOCK_PARTICLES && (next_free_particle_index + placed) <= MAX_PARTICLES)
				{
					float jitter = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
					particle_block.push_back(Particle(x + jitter, y + jitter, 1));
					placed++;
				}

		glBindVertexArray(particles_vao);
		glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, next_free_particle_index * sizeof(Particle), placed * sizeof(Particle), particle_block.data());

		next_free_particle_index += placed;
	}
}

void sph_sim::resize_window(GLsizei window_size[2])
{
	m_window_size[0] = window_size[0];
	m_window_size[1] = window_size[1];
}