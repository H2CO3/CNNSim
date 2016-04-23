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
#include <cassert>

#include "CNN.hh"
#include "imgproc.hh"


// Get a matrix element, subject to boundary conditions
static double get_matrix_element(
	const double *RESTRICT mat,
	std::ptrdiff_t r,
	std::ptrdiff_t c,
	std::ptrdiff_t width,
	std::ptrdiff_t height,
	Template tem
)
{
	// regular case, inner cells
	if (0 <= r && r < height && 0 <= c && c < width) {
		return mat[to_index(r, c, width)];
	}

	// boundary: r < 0 || r >= height || c < 0 || c >= width
	switch (tem.boundary_condition) {
		case Constant:
			return tem.virtual_cell;

		case ZeroFlux:
			if (r < 0) {
				r = 0;
			} else if (r >= height) {
				r = height - 1;
			}

			if (c < 0) {
				c = 0;
			} else if (c >= width) {
				c = width - 1;
			}

			return mat[to_index(r, c, width)];

		case Periodic:
			if (r < 0) {
				r += height;
			} else if (r >= height) {
				r -= height;
			}

			if (c < 0) {
				c += width;
			} else if (c >= width) {
				c -= width;
			}

			return mat[to_index(r, c, width)];

	default:
		assert(0 && "unreachable: invalid boundary condition");
		return 0;
	}
}

// Compute 3x3 neighborhood of the cell at (r, c)
template<typename Fn>
static double compute_neighborhood(
	std::ptrdiff_t r,
	std::ptrdiff_t c,
	std::ptrdiff_t width,
	std::ptrdiff_t height,
	CouplingMat M,
	Fn func
)
{
	double result = 0.0;
	const std::ptrdiff_t template_halfsize = M.size() / 2;

	for (std::ptrdiff_t off_r = -template_halfsize; off_r <= +template_halfsize; off_r++) {
		for (std::ptrdiff_t off_c = -template_halfsize; off_c <= +template_halfsize; off_c++) {
			auto r_p = r + off_r;
			auto c_p = c + off_c;

			result += func(r_p, c_p) * M[off_r + 1][off_c + 1];
		}
	}

	return result;
}

// The actual CNN dynamic equation
int CNN::dynamic_eq(double t, const double *RESTRICT x, double *RESTRICT dxdt, void *param)
{
	auto *cnn = static_cast<CNN *>(param);

	const auto width = cnn->width;
	const auto height = cnn->height;
	const auto tem = cnn->tem;
	const auto FF = cnn->FF;

	const std::ptrdiff_t template_halfsize = tem.A.size() / 2;

	for (std::ptrdiff_t r = 0; r < height; r++) {
		for (std::ptrdiff_t c = 0; c < width; c++) {
			std::ptrdiff_t i = to_index(r, c, width);

			double diff = FF[i] - x[i];

			if (
				   r < template_halfsize
				|| c < template_halfsize
				|| r >= height - template_halfsize
				|| c >= width - template_halfsize
			) {
				// Boundary Condition
				// This is separated from the regular case because it contains quite a few
				// conditionals - and I don't want those to be right in the middle of the
				// tight nested inner loops of neighborhood calculation.
				// Note: I've measured this, and leaving off this condition makes simulation
				// about 2 times slower, meaning that the compiler is not unswitching the loop.
				diff += compute_neighborhood(
					r, c, width, height, tem.A,
					[=](auto r_p, auto c_p) {
						double xij = get_matrix_element(x, r_p, c_p, width, height, tem);
						return y(xij);
					}
				);
			} else {
				// Inner cells - just compute regularly
				diff += compute_neighborhood(
					r, c, width, height, tem.A,
					[=](auto r_p, auto c_p) {
						auto j = to_index(r_p, c_p, width);
						return y(x[j]);
					}
				);
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
	double pt_max,
	double rel_tol,
	double abs_tol
):
	width(w),
	height(h),
	dimension(width * height),
	x(std::move(px)),
	FF(dimension),
	tem(ptem),
	h(rel_tol * abs_tol),
	t_max(pt_max),
	ode { 0 },
	stepper(nullptr),
	control(nullptr),
	evolver(nullptr)
{
	// Rudimentary sanity checking
	assert(x.size() == dimension && "you lied about the size of the initial state");
	assert(u.size() == dimension && "you lied about the size of the input image");

	// Precompute Feed-Forward Image
	for (std::ptrdiff_t r = 0; r < height; r++) {
		for (std::ptrdiff_t c = 0; c < width; c++) {

			double cell = compute_neighborhood(
				r, c, width, height, tem.B,
				[=, &u](auto r_p, auto c_p) {
					return get_matrix_element(&u[0], r_p, c_p, width, height, tem);
				}
			);

			auto i = to_index(r, c, width);
			FF[i] = cell + tem.Z;
		}
	}

	// Set up ODE solver
	ode.function = dynamic_eq;
	ode.jacobian = nullptr;
	ode.dimension = dimension;
	ode.params = this;

	// Runge-Kutta-Fehlberg method of order 4-5
	stepper = gsl_odeiv2_step_alloc(gsl_odeiv2_step_rkf45, dimension);
	control = gsl_odeiv2_control_standard_new(abs_tol, rel_tol, 1, 1);
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

void CNN::extract_output(GrayscaleImage *output)
{
	output->width = width;
	output->height = height;
	output->buf.resize(dimension);
	std::transform(x.begin(), x.end(), output->buf.begin(), CNN::y);
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

void CNN::run()
{
	double t = 0.0;
	while (step(&t)) {
		// no-op
	}
}

void CNN::run_with_handler(std::function<bool(double)> handler)
{
	bool keep_running = true;
	double t = 0.0;

	while (keep_running && step(&t)) {
		keep_running = handler(t);
	}
}
