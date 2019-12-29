#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>

#include "sph_sim.h"

#include "exception.h"

#define GLT_MANUAL_VIEWPORT
#define GLT_IMPLEMENTATION
#include "gltext.h"

using namespace std;

GLsizei window_size[] = { 800,800 };
GLFWwindow* window;

int frame_count = 0;

sph_sim sph(window_size);

// Simulation info text.
GLTtext *sim_info_text;

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		sph.add_particle_block();
	}
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
	window_size[0] = width;
	window_size[1] = height;
	sph.resize_window(window_size);
}

int main(int argc, char** argv)
{
	if (!glfwInit()) {
		cerr << "ERROR: could not start GLFW3" << endl;
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	window = glfwCreateWindow(window_size[0], window_size[1], "SPH GPU", NULL, NULL);
	if (!window)
	{
		cerr << "ERROR: could not create GLFW3 window." << endl;
		return 0;
	}

	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		cerr << "Error: " << glewGetErrorString(err) << endl;
		return 1;
	}

	try
	{
		// Initialize glText
		gltInit();

		std::stringstream ss_text_info("FPS:\nParticles:");
		sim_info_text = gltCreateText();
		gltSetText(sim_info_text, ss_text_info.str().c_str());

		sph.init_particles();

		double previous_time = glfwGetTime();

		while (!glfwWindowShouldClose(window))
		{
			// step sim and render particles
			sph.step_particles();
			sph.render();
			frame_count++;

			gltViewport(window_size[0], window_size[1]);

			// draw info text
			gltBeginDraw();
			gltColor(0.0f, 1.0f, 0.5f, 1.0f);
			gltDrawText2D(sim_info_text, 10, 10, 1.0f);
			gltEndDraw();

			double current_time = glfwGetTime();

			// one second elapsed
			if (current_time - previous_time >= 1.0)
			{
				ss_text_info = std::stringstream();
				ss_text_info << "FPS: " << frame_count << "\nParticles: " << sph.particle_count();
				gltSetText(sim_info_text, ss_text_info.str().c_str());

				frame_count = 0;
				previous_time = current_time;
			}

			glfwSwapBuffers(window);

			glfwPollEvents();
		}
	}
	catch (unrecoverable_except& e)
	{
		cerr << "unrecoverable exception: " << e.what() << endl;
	}

	gltDeleteText(sim_info_text);
	gltTerminate();

	return 0;
}

