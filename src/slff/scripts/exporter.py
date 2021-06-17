#!/usr/bin/python3

from prometheus_client import start_http_server, Counter, Gauge, Histogram, Summary

import rospy
from slff.msg import exporter_peripheral_status
from slff.msg import exporter_version
from slff.msg import exporter_uptime

slff_peripheral_status = Gauge("slff_peripheral_status", "Peripheral status", ["peripheral"])
slff_version = Gauge("slff_version", "Version", ["ver"])
slff_uptime = Gauge("slff_uptime", "Uptime")

# =============================================================================
# -----------------------------------------------------------------------------
# =============================================================================


def cllbck_tim_1hz(event):
    if(exporter_routine() == -1):
        rospy.signal_shutdown("signal_shutdown")

# =============================================================================
# -----------------------------------------------------------------------------
# =============================================================================


def cllbck_sub_exporter_peripheral_status(msg):
    slff_peripheral_status.labels(peripheral="gto").set(msg.gto)
    slff_peripheral_status.labels(peripheral="rfid").set(msg.rfid)


def cllbck_sub_exporter_version(msg):
    slff_version.labels(ver=msg.version).set(1)


def cllbck_sub_exporter_uptime(msg):
    slff_uptime.set(msg.uptime)

# =============================================================================
# -----------------------------------------------------------------------------
# =============================================================================


def exporter_init():
    start_http_server(8100)

    return 0


def exporter_routine():

    return 0

# =============================================================================
# -----------------------------------------------------------------------------
# =============================================================================


if __name__ == "__main__":
    # Timer
    global tim_1hz
    # Subscriber
    global sub_exporter_peripheral_status
    global sub_exporter_version

    rospy.init_node('exporter')

    # Timer
    tim_1hz = rospy.Timer(rospy.Duration(1), cllbck_tim_1hz)
    # Subscriber
    sub_exporter_peripheral_status = rospy.Subscriber('exporter/peripheral_status', exporter_peripheral_status, cllbck_sub_exporter_peripheral_status)
    sub_exporter_version = rospy.Subscriber('exporter/version', exporter_version, cllbck_sub_exporter_version)
    sub_exporter_uptime = rospy.Subscriber('exporter/uptime', exporter_uptime, cllbck_sub_exporter_uptime)

    if(exporter_init() == -1):
        rospy.signal_shutdown("signal shutdown")

    rospy.spin()
