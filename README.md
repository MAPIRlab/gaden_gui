# Gaden GUI
Graphical frontend for [gaden](https://github.com/MAPIRlab/gaden_core). Gaden GUI is a standalone desktop application, and does not require ROS. It uses [DearIMGUI](https://github.com/ocornut/imgui) to handle layout and rendering.

## Installation
Clone the repository with
```
git clone git@github.com:MAPIRlab/gaden_gui.git
```

You can then compile with
```
mkdir build
cd build
cmake ..
make -j12
```

You do not need to pre-download the `gaden_core` library, as this project uses CMake's `FetchContent` to automatically clone its dependecies into the build directory.
