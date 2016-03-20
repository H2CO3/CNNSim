//
// CNN.cc
// CNNSim, a simple CNN simulator
//
// Created by Arpad Goretity on 17/03/2016
//
// Licensed under the 2-clause BSD License
//

#include <cstdio>
#include <cmath>
#include <cstring>

#include "CNN.hh"
#include "imgproc.hh"


// The actual CNN dynamic equation
static int dynamic_eq(double t, const double *__restrict__ x, double *__restrict__ dxdt, void *param)
{
	auto *cnn = static_cast<CNN *>(param);

	auto width = cnn->width;
	auto height = cnn->height;
	auto tem = cnn->tem;
	auto FF = cnn->FF;

	for (std::ptrdiff_t r = 0; r < height; r++) {
		for (std::ptrdiff_t c = 0; c < width; c++) {
			std::ptrdiff_t i = to_index(r, c, width);

			// boundary
			if (r == 0 || c == 0 || r == height - 1 || c == width - 1) {
				// TODO: add Neumann and periodic boundary conditions
				dxdt[i] = 0;
				continue;
			}

			double diff = FF[i] - x[i];

			// Compute 3x3 neighborhood
			for (std::ptrdiff_t off_r = -1; off_r <= +1; off_r++) {
				for (std::ptrdiff_t off_c = -1; off_c <= +1; off_c++) {
					std::ptrdiff_t r_p = r + off_r;
					std::ptrdiff_t c_p = c + off_c;
					std::ptrdiff_t j = to_index(r_p, c_p, width);

					diff += tem.A[off_r + 1][off_c + 1] * y(x[j]);
				}
			}

			dxdt[i] = diff;
		}
	}

	return GSL_SUCCESS;
}


CNN::CNN(
	std::ptrdiff_t w,
	std::ptrdiff_t h,
	std::vector<double> px,
	std::vector<double> u,
	Template ptem,
	double pt_max
):
	width(w),
	height(h),
	dimension(width * height),
	x(std::move(px)),
	FF(dimension),
	tem(ptem),
	h(1.0e-6),
	t_max(pt_max),
	ode { 0 },
	stepper(nullptr),
	control(nullptr),
	evolver(nullptr)
{
	// Precompute Feed-Forward Image
	// TODO: add non-constant boundary conditions (Zero-Flux, Toroidal)
	for (std::ptrdiff_t r = 1; r < height - 1; r++) {
		for (std::ptrdiff_t c = 1; c < width - 1; c++) {
			std::ptrdiff_t i = to_index(r, c, width);

			double ci = tem.Z;

			for (std::ptrdiff_t off_r = -1; off_r <= +1; off_r++) {
				for (std::ptrdiff_t off_c = -1; off_c <= +1; off_c++) {
					std::ptrdiff_t r_p = r + off_r;
					std::ptrdiff_t c_p = c + off_c;
					std::ptrdiff_t j = to_index(r_p, c_p, width);

					ci += tem.B[off_r + 1][off_c + 1] * u[j];
				}
			}

			FF[i] = ci;
		}
	}

	// Set up ODE solver
	ode.function = dynamic_eq;
	ode.jacobian = nullptr;
	ode.dimension = dimension;
	ode.params = this;

	// Runge-Kutta-Fehlberg method of order 4-5
	stepper = gsl_odeiv2_step_alloc(gsl_odeiv2_step_rkf45, dimension);
	control = gsl_odeiv2_control_standard_new(1.0e-3, 1.0e-3, 1, 1);
	evolver = gsl_odeiv2_evolve_alloc(dimension);
}

CNN::~CNN()
{
	gsl_odeiv2_evolve_free(evolver);
	gsl_odeiv2_control_free(control);
	gsl_odeiv2_step_free(stepper);
}

const std::vector<double> &CNN::state() const
{
	return x;
}

bool CNN::step(double *t)
{
	int status = gsl_odeiv2_evolve_apply(
		evolver,
		control,
		stepper,
		&ode,
		t,
		t_max,
		&h,
		&x[0]
	);

	return status == GSL_SUCCESS && *t < t_max;
}
