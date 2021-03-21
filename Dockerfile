FROM ros:melodic

# Install ROS dependencies
RUN apt-get update && apt-get install --no-install-recommends -y \
    # ros-melodic-rosbridge-suite \
    # ros-melodic-vision-opencv \
    # ros-melodic-image-common \
    # ros-melodic-web-video-server \
    && rm -rf /var/lib/apt/lists/*

# Install other dependencies
RUN apt-get update && apt-get install --no-install-recommends -y \
    libserialport-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Make directory for application and application data
RUN /bin/bash -c 'mkdir -p /root/slff-data'
RUN /bin/bash -c 'mkdir -p /root/slff'

# Copy application source
COPY ./src /root/slff/src
COPY ./slff_entrypoint.sh /slff_entrypoint.sh

# Compile application source
RUN /bin/bash -c 'source "/opt/ros/$ROS_DISTRO/setup.bash" && cd /root/slff && catkin_make'

# Make entrypoint become executable
RUN /bin/bash -c 'chmod +x slff_entrypoint.sh'

# Entrypoint
ENTRYPOINT ["/slff_entrypoint.sh"]