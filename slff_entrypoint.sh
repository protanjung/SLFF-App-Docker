#!/bin/bash
set -e

source "/opt/ros/$ROS_DISTRO/setup.bash"
source "$HOME/slff/devel/setup.bash"

exec "$@"
