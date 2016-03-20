//
// template.hh
// CNNSim, a simple CNN simulator
//
// Created by Arpad Goretity on 19/03/2016
//
// Licensed under the 2-clause BSD License
//

#ifndef CNNSIM_TEMPLATE_HH
#define CNNSIM_TEMPLATE_HH

#include <cstdio>

#include <string>
#include <array>
#include <istream>
#include <iostream>
#include <fstream>
#include <sstream>


typedef std::array<std::array<double, 3>, 3> CouplingMat;

enum BoundaryCondition {
	Constant, // or Dirichlet. Template::virtual_cell is only valid when this is set.
	ZeroFlux, // or Neumann
	Periodic, // or 'toroidal'
	NumBoundaryConditions
};


struct Template {
	CouplingMat A;
	CouplingMat B;
	double Z;
	BoundaryCondition boundary_condition;
	double virtual_cell;
};

Template load_template_file(const char *fname);
Template load_template_stdio(std::FILE *handle);
Template load_template_stream(std::istream &stream);

void save_template_file(const char *fname, Template tem);
void save_template_stdio(std::FILE *handle, Template tem);
void save_template_stream(std::ostream &stream, Template tem);

#endif // CNNSIM_TEMPLATE_HH
