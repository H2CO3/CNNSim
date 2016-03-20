#!/bin/sh

# An example of batch-processing images
# This computation tries to find the path in a maze
# by progressively erasing dead ends.

# inputs/maze_64.png is the initial image
cp ../inputs/maze_64.png out.png

for i in {1..100}; do
		# Erase dead ends
		../CNN -s ../inputs/black_64.png -i out.png -t ../templates/delete_dead_end -d 10 -o out.png
		# Preserve the starting point and end point of the maze
		../CNN -s out.png -i ../inputs/maze_start_end.png -t ../templates/log_and -d 10 -o out.png
done

