# Make sure apt-get is updated
sudo apt-get update
sudo apt-get upgrade

# Install prereqs for OpenCV
sudo apt-get install build-essential checkinstall cmake pkg-config yasm libtiff4-dev libjpeg-dev libjasper-dev libavcodec-dev libavformat-dev libswscale-dev  libdc1394-22-dev libxine-dev libgstreamer0.10-dev  libgstreamer-plugins-base0.10-dev  libv4l-dev python-dev python-numpy libqt4-dev libgtk2.0-dev

# Install boost library
sudo apt-get install libboost.*1.48.0 libboost.*1.48-dev 
#sudo apt-get install libboost-thread-dev

# Install OpenCV
sudo apt-get install libopencv-dev


