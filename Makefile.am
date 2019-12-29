AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = sph_sim
sph_sim_SOURCES = \
    src/main.cpp \
    src/gl_shader.cpp \
    src/sph_sim.cpp
sph_sim_CXXFLAGS = -Wall -std=c++11 -Ilib/eigen `pkg-config --cflags glfw3 glew`
sph_sim_LDFLAGS = `pkg-config --libs glfw3 glew`

