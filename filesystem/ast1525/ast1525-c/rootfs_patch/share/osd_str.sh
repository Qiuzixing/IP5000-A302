#!/bin/sh

###########################################################
#  Customize following OSD messages
# To see the customization result, use this console API "/usr/local/bin/ast_sent_event -1 e_osd_test"
###########################################################
OSD_MSG0="Graphic Mode"
OSD_MSG1="Video Mode"
OSD_MSG2="Anti-Dither OFF"
OSD_MSG3="Anti-Dither 1"
OSD_MSG4="Anti-Dither 2"
OSD_MSG5="Unsupported Resolution"
OSD_MSG6="Wait Video Input"
OSD_MSG7="Requesting USB"
OSD_MSG8="Starting USB"
OSD_MSG9="Stopping USB"
OSD_MSG10="Suggest 60Hz Refresh Rate\nPlease Change Video Source Setup"
OSD_MSG11="HDCP Failed"


###########################################################
# Leave following settings as default should be fine for most of cases.
# Customize them carefully
###########################################################
OSD_FONT_SIZE="30"
# 32bits RGB color. 0xFFRRGGBB
OSD_FONT_COLOR="0xFF00FF00"
# The maximum height of OSD messages. 200 pixel is default good value. Don't change it.
OSD_MAX_Y_SIZE="200"
# From 1 to 15.  15 means no transparent.
OSD_TRANSPARENT="15"
