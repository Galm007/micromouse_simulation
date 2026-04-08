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
make -j$(nproc)
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

<img width="1196" height="999" alt="Screenshot_20260408_131653" src="https://github.com/user-attachments/assets/bd96bbd3-2b67-49dc-9e27-94c4803866ed" />

<img width="1197" height="1001" alt="Screenshot_20260408_131731" src="https://github.com/user-attachments/assets/f84cbd13-222b-4f90-aae8-63d62f79bc42" />
