//
// template.cc
// CNNSim, a simple CNN simulator
//
// Created by Arpad Goretity on 19/03/2016
//
// Licensed under the 2-clause BSD License
//

#include "template.hh"


Template load_template_file(const char *fname)
{
	std::ifstream stream(fname);
	return load_template_stream(stream);
}

Template load_template_stdio(std::FILE *handle)
{
	std::fseek(handle, 0, SEEK_END);
	std::string buf(std::ftell(handle), '\0');
	std::fseek(handle, 0, SEEK_SET);
	std::fread(&buf[0], buf.size(), 1, handle);
	std::istringstream stream(buf);
	return load_template_stream(stream);
}

Template load_template_stream(std::istream &stream)
{
	std::string name;
	Template tem;

	auto read_coupling_mat = [&](CouplingMat &mat) {
		for (auto &row : mat) {
			for (auto &elem : row) {
				stream >> elem;
			}
		}
	};

	while (stream >> name) {
		switch (name[0]) {
		case 'A':
			read_coupling_mat(tem.A);
			break;
		case 'B':
			read_coupling_mat(tem.B);
			break;
		case 'Z':
			stream >> tem.Z;
			break;
		}
	}

	return tem;
}

void save_template_file(const char *fname, Template tem)
{
	std::ofstream stream(fname);
	save_template_stream(stream, tem);
}

void save_template_stdio(std::FILE *handle, Template tem)
{
	std::ostringstream stream;
	save_template_stream(stream, tem);
	auto str = stream.str();
	std::fwrite(str.c_str(), str.size(), 1, handle);
}

void save_template_stream(std::ostream &stream, Template tem)
{
	auto write_coupling_mat = [&](const char *name, CouplingMat mat) {
		stream << name << "\n";

		for (auto row : mat) {
			for (auto elem : row) {
				stream << "\t" << elem;
			}
			stream << "\n";
		}

		stream << "\n";
	};

	write_coupling_mat("A", tem.A);
	write_coupling_mat("B", tem.B);

	stream << "Z\n\t" << tem.Z << "\n";
}
