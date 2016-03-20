# CNNSim: A tiny, fast, crude CNN simulator written in C++

### Usage example:

    ./CNN -s inputs/test_128.png -i inputs/test_128.png -t templates/hollow -d 500 -r 1e-2 -a 1e-1

The meaning of the parameters is as follows:

* `-s`, `--state`: **Required.** Should point to the name of a PNG file that will be used as the initial
                   state of the CNN.
* `-i`, `--input`: **Required.** A PNG file that will be used as the "input image" (feed-forward).
                   It must have the same dimensions as the state.
* `-t`, `--template`: **Required.** The name of a template file containing at least the following information:
	- `A`: the feedback matrix
	- `B`: the feed-forward matrix
	- `Z`: the bias (also known as `I` in some contexts)
	- `C`: the kind of boundary condition, and, if applicable, the value of boundary cells

	For the precise format of template files, see the examples in `templates/`.

* `-d`, `--duration`: **Required.** Duration (end time) of the simulation.
* `-r`, `--rel-tol`: **Optional.** Relative tolerance of the numerical solution of the state equation.
                     Defaults to `1.0e-3`.
* `-a`, `--abs-tol`: **Optional.** Absolute tolerance of the numerical solution of the state equation.
                     Defaults to `1.0e-3`.

### Building

* On Unix-like systems, you can just type `make`.
* On Windows: who knows? I don't use Windows.

