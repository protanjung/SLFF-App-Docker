#!/usr/bin/python3

from prometheus_client import start_http_server, Counter, Gauge, Histogram, Summary

import rospy
from slff.msg import peripheral_status

slff_peripheral_status = Gauge("slff_peripheral_status", "Peripheral status", ["peripheral"])

# =============================================================================
# -----------------------------------------------------------------------------
# =============================================================================


def cllbck_tim_100hz(event):
    if(exporter_routine() == -1):
        rospy.signal_shutdown("signal_shutdown")

# =============================================================================
# -----------------------------------------------------------------------------
# =============================================================================


def cllbck_sub_peripheral_status(msg):
    slff_peripheral_status.labels(peripheral="gto").set(msg.gto)
    slff_peripheral_status.labels(peripheral="rfid").set(msg.rfid)

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
    global tim_100hz
    # Subscriber
    global sub_peripheral_status

    rospy.init_node('exporter')

    # Timer
    tim_100hz = rospy.Timer(rospy.Duration(0.01), cllbck_tim_100hz)
    # Subscriber
    sub_peripheral_status = rospy.Subscriber('peripheral_status', peripheral_status, cllbck_sub_peripheral_status)

    if(exporter_init() == -1):
        rospy.signal_shutdown("signal shutdown")

    rospy.spin()
