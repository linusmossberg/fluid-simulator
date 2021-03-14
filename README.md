# Fluid Simulator

<img src="https://user-images.githubusercontent.com/15798094/111071530-e1bc1680-84d6-11eb-8408-0e916b919a80.gif" width="100%"/>

## Building

Start by cloning the program and all submodules using the following command:
<pre><code>git clone <b>--recursive</b> https://github.com/linusmossberg/fluid-simulator</code></pre>
Build files can then be generated with [CMake](https://cmake.org/) using:
```sh
cd light-field-renderer
mkdir build
cd build
cmake ..
```
These should work automatically, but a few additional packages are required on certain platforms. On Debian/Ubuntu, these can be installed using:
```sh
apt-get install xorg-dev libglu1-mesa-dev
```
and on RedHat/Fedora:
```sh
sudo dnf install mesa-libGLU-devel libXi-devel libXcursor-devel libXinerama-devel libXrandr-devel xorg-x11-server-devel