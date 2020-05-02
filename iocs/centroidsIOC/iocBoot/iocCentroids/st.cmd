#!../../bin/linux-x86_64/centroidsApp
< envPaths

epicsEnvSet("ADSIMDETECTOR", "$(AREA_DETECTOR)/ADSimDetector")

errlogInit(20000)

dbLoadDatabase("$(TOP)/dbd/centroidsApp.dbd")
centroidsApp_registerRecordDeviceDriver(pdbbase)

epicsEnvSet("PREFIX",     "CENTROIDS:")
epicsEnvSet("PORT",       "SIMDET1")
epicsEnvSet("CENTPORT",   "CENTROIDS1")
epicsEnvSet("QSIZE",      "20")
epicsEnvSet("XSIZE",      "2048")
epicsEnvSet("YSIZE",      "2048")
epicsEnvSet("NCHANS",     "2048")
epicsEnvSet("CBUFFS",     "500")

epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(ADCORE)/db")

# Create a simDetector driver
# simDetectorConfig(const char *portName, int maxSizeX, int maxSizeY, int dataType,
#                   int maxBuffers, int maxMemory, int priority, int stackSize)
simDetectorConfig("$(PORT)", $(XSIZE), $(YSIZE), 1, 0, 0)
# To have the rate calculation use a non-zero smoothing factor use the following line
#dbLoadRecords("simDetector.template",     "P=$(PREFIX),R=cam1:,PORT=$(PORT),ADDR=0,TIMEOUT=1,RATE_SMOOTH=0.2")
dbLoadRecords("$(ADSIMDETECTOR)/db/simDetector.template","P=$(PREFIX),R=cam1:,PORT=$(PORT),ADDR=0,TIMEOUT=1")

# Create a NDPluginCentroids centroids plugin
NDCentroidsConfigure("$(CENTPORT)", $(QSIZE), 0, "$(PORT)", 0, 0)

# Load Records
dbLoadRecords("$(ADPLUGINCENTROIDS)/db/NDCentroids.template",   "P=$(PREFIX),R=centroids1:,PORT=$(CENTPORT),ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")

# Create a standard arrays plugin
NDStdArraysConfigure("Image1", 5, 0, "$(PORT)", 0, 0)
dbLoadRecords("NDStdArrays.template", "P=$(PREFIX),R=image1:,PORT=Image1,NDARRAY_PORT=$(CENTPORT),ADDR=0,TIMEOUT=1,TYPE=Int16,FTVL=SHORT,NELEMENTS=2361600")

# Load all other plugins using commonPlugins.cmd
< $(ADCORE)/iocBoot/commonPlugins.cmd

set_requestfile_path("$(ADPLUGINCENTROIDS)/centroidsApp/Db")
set_requestfile_path("$(ADCORE)","ADApp/Db")

iocInit()

# save things every thirty seconds
create_monitor_set("auto_settings.req", 30,"P=$(PREFIX),D=centroids1:")
dbl > $(TOP)/records.dbl


