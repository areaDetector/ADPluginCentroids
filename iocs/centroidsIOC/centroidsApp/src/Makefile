TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

# Note: we must built the application without GraphicsMagick.
USE_GRAPHICSMAGICK=NO

PROD_NAME = centroidsApp
PROD_IOC_Linux  += $(PROD_NAME)

# <name>.dbd will be created from <name>Include.dbd
DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += NDPluginCentroids.dbd
$(PROD_NAME)_DBD += $(AREA_DETECTOR)/ADSimDetector/dbd/simDetectorSupport.dbd

PROD_SRCS += $(PROD_NAME)_registerRecordDeviceDriver.cpp $(PROD_NAME)Main.cpp

# Add locally compiled object code
PROD_LIBS += NDPluginCentroids
PROD_LIBS += centroids

PROD_LIBS += simDetector
simDetector_DIR = $(AREA_DETECTOR)/ADSimDetector/lib/linux-x86_64

include $(ADCORE)/ADApp/commonDriverMakefile

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

