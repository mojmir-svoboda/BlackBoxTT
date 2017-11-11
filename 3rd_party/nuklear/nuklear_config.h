#pragma once

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include <cstring>

struct nk_d3d11_vertex {
	float position[2];
	float uv[2];
	nk_byte col[4];
};

//#include "nuklear_d3d11.h"

inline void
nk_d3d11_get_projection_matrix(int width, int height, float *result)
{
	const float L = 0.0f;
	const float R = (float)width;
	const float T = 0.0f;
	const float B = (float)height;
	float matrix[4][4] =
	{
		{    2.0f / (R - L),              0.0f, 0.0f, 0.0f },
		{              0.0f,    2.0f / (T - B), 0.0f, 0.0f },
		{              0.0f,              0.0f, 0.5f, 0.0f },
		{ (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f },
	};
	memcpy(result, matrix, sizeof(matrix));
}
