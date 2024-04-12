# Furniture Layout Optimizer

This project is a simple furniture layout optimizer using the Metropolis-Hastings algorithm. The application allows the user to visualize a 3D room and interactively adjust the positions and orientations of a couch and a TV. The optimization algorithm attempts to find a configuration that minimizes the pairwise distance and angle costs.

## Dependencies

- FLTK
- OpenGL
- GLUT

## Controls

The user interface provides the following controls:

1. `Max Loop Counter`: Sets the maximum number of optimization iterations.
2. `Couch X Position`: Controls the X position of the couch.
3. `Couch Z Position`: Controls the Z position of the couch.
4. `TV X Position`: Controls the X position of the TV.
5. `TV Z Position`: Controls the Z position of the TV.
6. `Size of Floor`: Sets the size of the floor.
7. `Target Pairwise Distance`: Sets the target distance between the couch and the TV.
8. `Target Angle: TV to Couch`: Sets the target angle for the TV to face the couch.
9. `Target Angle: Couch to TV`: Sets the target angle for the couch to face the TV.
10. `Weight Pairwise Distance`: Sets the weight of the pairwise distance cost term.
11. `Weight Pairwise Angle`: Sets the weight of the pairwise angle cost term.
12. `Temperature`: Controls the temperature of the Metropolis-Hastings algorithm.
13. `Reset Furniture Locations`: Resets the furniture locations to random positions within the room.
14. `Optimize Furniture Locations`: Runs the optimization algorithm.
15. `Show Intermediate Results`: Toggles the visualization of intermediate optimization results.

## Usage

1. Compile the code with the appropriate dependencies.
2. Run the application.
3. Use the control widgets to interact with the scene and adjust the optimization parameters.
4. Click on "Optimize Furniture Locations" to run the optimization algorithm.
5. Monitor the optimization progress in the output window.
6. Reset the furniture locations and rerun the optimization if needed.


## Notes
- The optimization algorithm and all the other features are implemented in "sample.cpp".
- The optimization algorithm uses the Metropolis-Hastings algorithm to find a suitable configuration.
- The cost function includes pairwise distance and angle costs.
- The optimization can be stopped at any time by unchecking the "Optimize Furniture Locations" checkbox.
- I have implemented all the features except the functionality to output the cost values to a text file in a csv format.

## Created by Nipuna Vancha
