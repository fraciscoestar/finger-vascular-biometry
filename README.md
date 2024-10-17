# Development of a biometric system based on the vascular anatomy of the index finger
> [!WARNING]
> This document is still being actively written. Information may not be precise nor complete yet.

This repository aims to provide an open-source implementation of a biometric system with low-cost hardware. To learn more about the project, please refer to the [original work](https://github.com/fraciscoestar/finger-vascular-biometry/blob/main/docs/TFG.pdf). The system consists of the following elements:
- A [LABISTS](https://labists.com/products/labists-raspberry-pi-4g-ram-32gb-card) kit of a Raspberry Pi 4 4GB. Another kit may be used to build the project but the 3D-printed enclosure is designed to use the case provided in the kit.
- A Raspberry Pi NoIR Camera Module. Another camera may also be used as long as it does not include an IR filter, although, changing this piece of hardware is not recommended because the source code will need tweaks.
- A custom-made 3D printed enclosure to fit all the parts.
- A custom-made PCB to control the IR lights from the Raspberry Pi.
- A custom made cable to connect the PCB to the Raspberry Pi 4. It is a 4 pin JST-XH connector on one end and a 4 pin dupont on the other end.

# Installation
The compilation of this project is non-trivial because OpenCV must be installed manually. The project has been tested in Ubuntu and Raspbian (Raspberry Pi 4 OS), but it may also be able to compile in Windows and Mac OS as long as all the dependencies are installed. A guide to install OpenCV 4.5.2 is provided for both Ubuntu and Raspbian:

## OpenCV installation
### OpenCV installation (Ubuntu)
First, we should update the OS and install some apps that will help us later.
``` bash
sudo apt-get update && sudo apt-get upgrade
sudo apt install -y software-properties-common
sudo apt install -y apt-file
```
Now we install the OpenCV4 dependencies.
``` bash
sudo apt-get install build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install python3.5-dev python3-numpy libtbb2 libtbb-dev
sudo apt-get install libjpeg-dev libpng-dev libtiff5-dev libjasper-dev libdc1394-22-dev libeigen3-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev sphinx-common libtbb-dev yasm libfaac-dev libopencore-amrnb-dev libopencore-amrwb-dev libopenexr-dev libgstreamer-plugins-base1.0-dev libavutil-dev libavfilter-dev libavresample-dev
```
Now we can download the OpenCV 4.5.2 source code and prepare it to install it. You can download the files to another direction if you wish.
``` bash
sudo -s
cd /opt
wget https://github.com/opencv/opencv/archive/4.5.2.zip
unzip 4.5.2.zip
mv opencv-4.5.2/ opencv/
rm 4.5.2.zip
wget https://github.com/opencv/opencv_contrib/archive/4.5.2.zip
unzip 4.5.2.zip
mv opencv_contrib-4.5.2/ opencv_contrib/
rm 4.5.2.zip
```
Now we can proceed with the compilation.
``` bash
cd opencv
mkdir release
cd release
cmake -D BUILD_TIFF=ON -D WITH_CUDA=OFF -D ENABLE_AVX=OFF -D WITH_OPENGL=OFF -D WITH_OPENCL=OFF -D WITH_IPP=OFF -D WITH_TBB=ON -D BUILD_TBB=ON -D WITH_EIGEN=OFF -D WITH_V4L=OFF -D WITH_VTK=OFF -D BUILD_TESTS=OFF -D BUILD_PERF_TESTS=OFF -D OPENCV_ENABLE_NONFREE=ON -D OPENCV_GENERATE_PKGCONFIG=ON -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D OPENCV_EXTRA_MODULES_PATH=/opt/opencv_contrib/modules /opt/opencv/
make -j8
make install
ldconfig
exit
```
Lastly, we can check if the installation was done correctly if the following command returns `4.5.2`.
``` bash
pkg-config –-modversion opencv4
```

### OpenCV installation (Raspbian)
The installation is similar to the installation in Ubuntu, but the process takes a lot longer and have more steps. First, we need to update the EEPROM firmware of the Raspberry Pi. and resize the swap file of the system.
``` bash
sudo rpi-eeprom-update -a
sudo reboot
```
We also need to make the swapfile a lot bigger temporarily. We modify CONF_MAXSWAP and set it to 4096 in `/sbin/dphys-swapfile`.
``` bash
sudo nano /sbin/dphys-swapfile
sudo reboot
```
Now we install the dependencies.
``` bash
sudo apt-get install build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install python3.5-dev python3-numpy libtbb2 libtbb-dev
sudo apt-get install libjpeg-dev libpng-dev libtiff5-dev libjasper-dev libdc1394-22-dev libeigen3-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev sphinx-common libtbb-dev yasm libfaac-dev libopencore-amrnb-dev libopencore-amrwb-dev libopenexr-dev libgstreamer-plugins-base1.0-dev libavutil-dev libavfilter-dev libavresample-dev
sudo apt-get install gfortran libgif-dev libcanberra-gtk* libgtk-3-dev libv4l-dev libopenblas-dev libatlas-base-dev libblas-dev liblapack-dev libhdf5-dev protobuf-compiler qt5-default
```
The next step is downloading the OpenCV 4.5.2 source code.
``` bash
cd ~
wget https://github.com/opencv/opencv/archive/4.5.2.zip
unzip 4.5.2.zip
mv opencv-4.5.2/ opencv/
rm 4.5.2.zip
wget https://github.com/opencv/opencv_contrib/archive/4.5.2.zip
unzip 4.5.2.zip
mv opencv_contrib-4.5.2/ opencv_contrib/
rm 4.5.2.zip
```
Now we proceed with the installation.
``` bash
cd opencv
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules -D ENABLE_NEON=ON -D ENABLE_VFPV3=ON -D WITH_OPENMP=ON -D WITH_OPENCL=OFF -D BUILD_ZLIB=ON -D BUILD_TIFF=ON -D WITH_FFMPEG=ON -D WITH_TBB=ON -D BUILD_TBB=ON -D BUILD_TESTS=OFF -D WITH_EIGEN=OFF -D WITH_GSTREAMER=OFF -D WITH_V4L=ON -D WITH_LIBV4L=ON -D WITH_VTK=OFF -D BUILD_EXAMPLES=OFF -D WITH_QT=ON -D OPENCV_ENABLE_NONFREE=ON -D INSTALL_C_EXAMPLES=OFF -D INSTALL_PYTHON_EXAMPLES=OFF -D BUILD_opencv_python3=TRUE -D OPENCV_GENERATE_PKGCONFIG=ON ..
make -j4
sudo make install
sudo ldconfig
make clean
sudo apt-get update
```
Lastly, we can reset the swap file to its default size. We modify CONF_MAXSWAP and set it to 100 in `/sbin/dphys-swapfile`.
``` bash
sudo nano /sbin/dphys-swapfile
sudo reboot
```

## Installation of the project
Once OpenCV4 is installed, compiling the project should be an easy job. First, you need to clone this repository and prepare it for building with CMake.
``` bash
cd ~
git clone https://github.com/fraciscoestar/finger-vascular-biometry.git
cd finger-vascular-biometry
mkdir build
cd build
```
CMake will try to figure out whether you are compiling the project in a Raspberry Pi or not. Some features will be modified or completely missing if building on another system, naturally. To build the project, just do the following. 
``` bash
cmake ..
make -j4
```
