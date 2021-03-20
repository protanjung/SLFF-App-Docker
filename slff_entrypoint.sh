#!/bin/bash
set -e

# setup ros environment
source "/opt/ros/$ROS_DISTRO/setup.bash"
source "$HOME/slff/devel/setup.bash"
# launch ros nodes
roslaunch slff slff.launch
exec "$@"
