//
// CNN.hh
// CNNSim, a simple CNN simulator
//
// Created by Arpad Goretity on 17/03/2016
//
// Licensed under the 2-clause BSD License
//

#ifndef CNNSIM_CNN_HH
#define CNNSIM_CNN_HH

#include <cstddef>

#include <array>
#include <vector>
#include <algorithm>
#include <utility>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>

#include "util.hh"
#include "template.hh"


static int dynamic_eq(double t, const double *RESTRICT x, double *RESTRICT dxdt, void *param);

struct CNN {
private:
	const std::ptrdiff_t width;
	const std::ptrdiff_t height;
	const std::ptrdiff_t dimension;

	std::vector<double> x;
	std::vector<double> FF; // feed-forward image, precomputed

	Template tem;

	double h; // ODE solver step size
	const double t_max; // simulation time

	gsl_odeiv2_system ode;
	gsl_odeiv2_step *stepper;
	gsl_odeiv2_control *control;
	gsl_odeiv2_evolve *evolver;

public:
	CNN(
		std::ptrdiff_t w,
		std::ptrdiff_t h,
		std::vector<double> px,
		std::vector<double> u,
		Template ptem,
		double pt_max,
		double rel_tol = 1.0e-3,
		double abs_tol = 1.0e-3
	);

	CNN(const CNN &) = delete;
	CNN(CNN &&) = delete;

	~CNN();

	CNN &operator=(const CNN &) = delete;
	CNN &operator=(CNN &&) = delete;

	bool step(double *t);

	const std::vector<double> &state() const;

	friend int dynamic_eq(double t, const double *RESTRICT x, double *RESTRICT dxdt, void *param);
};

// Standard CNN nonlinearity function
static inline double y(double x)
{
	return std::max(-1.0, std::min(+1.0, x));
}

#endif // CNNSIM_CNN_HH
