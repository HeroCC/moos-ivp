FROM ubuntu:24.04 AS base
LABEL maintainer="Conlan Cesar <conlanc@csail.mit.edu>"

# Set the default shell to bash
SHELL ["/bin/bash", "-c"]

# Add MOOS variables to the env
ENV PATH="/home/moos/moos-ivp/bin:${PATH}"
ENV IVP_BEHAVIOR_DIRS="/home/moos/moos-ivp/lib"

# Make a user to run the MOOS apps
RUN useradd -m -p "moos" moos && usermod -a -G sudo moos

# Set the default user
USER moos

# Set the default entry directory to the moos user's home
WORKDIR "/home/moos"

# Copy in our files
COPY --chown=moos . /home/moos/moos-ivp

# If given, use attempt to generate reproducable builds
ARG SOURCE_DATE_EPOCH=""

# Install required MOOS dependencies
USER root

# Base
RUN apt-get update -y && apt-get install -y cmake build-essential lsb-release sudo && apt-get clean

# GUI
RUN apt-get update -y && apt-get install -y libtiff5-dev libfltk1.3-dev freeglut3-dev libpng-dev libjpeg-dev libxinerama-dev libxft-dev && apt-get clean

USER moos

# Build the MOOS-IvP tools
RUN cd "$HOME/moos-ivp" && ./build.sh

