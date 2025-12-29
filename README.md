# Micromouse Simulation

Graphical simulation for a micromouse maze solving algorithm, based on [Juing-Huei Su](https://github.com/suhulhu)'s diagonal maze solving algorithm.

Features Include:
- Save and load mazes with .maz files.
- Edit mazes with the built-in maze editor.
- Run a diagonal maze solving algorithm.

How to run (Linux with make):
```
git clone https://github.com/Galm007/micromouse_simulation && cd micromouse_simulation
mkdir build && cd build
cp ../resources/* .
cmake ..
make -j8
./micromouse_simulation
```

How to run (Windows with Visual Studio):
```
git clone https://github.com/Galm007/micromouse_simulation && cd micromouse_simulation
mkdir build && cd build
cp ../resources/* .
cmake ..
start devenv micromouse_simulation.sln

# NOTE: Make sure to set Visual Studio's startup project to micromouse_simulation before pressing the run button
```

<img width="1198" height="998" alt="Screenshot_20251228_170958" src="https://github.com/user-attachments/assets/d76626aa-ba4c-4d90-89e3-4a9bee733e82" />

<img width="1197" height="997" alt="Screenshot_20251228_170911" src="https://github.com/user-attachments/assets/4a8f2c46-16ac-499a-b4b2-e9d839b818a8" />
