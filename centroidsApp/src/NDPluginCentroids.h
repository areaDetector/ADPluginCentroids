#ifndef NDPluginCentroids_H
#define NDPluginCentroids_H

#include "NDPluginDriver.h"

/* Output data type */
#define NDPluginCentroidsThresholdString    "THRESHOLD"
#define NDPluginCentroidsBoxString          "BOX"
#define NDPluginCentroidsSearchBoxString    "SEARCH_BOX"
#define NDPluginCentroidsPixelPhotonString  "PIXEL_PHOTON"
#define NDPluginCentroidsPixelBgndString    "PIXEL_BGND"
#define NDPluginCentroidsCOMPhotonString    "PIXEL_COM"
#define NDPluginCentroidsOverlapMaxString   "OVERLAP_MAX"
#define NDPluginCentroidsSumMinString       "SUM_MIN"
#define NDPluginCentroidsSumMaxString       "SUM_MAX"
#define NDPluginCentroidsFitPixels2DString  "FIT_2D"
#define NDPluginCentroidsFitPixels1DXString "FIT_1D_X"
#define NDPluginCentroidsFitPixels1DYString "FIT_1D_Y"
#define NDPluginCentroidsNPhotonsString     "N_PHOTONS"
#define NDPluginCentroidsParamsValidString  "PARAMS_VALID"
#define NDPluginCentroidsStatusMsgString    "STATUS_MSG"


/** Does image processing operations.
 */
class NDPluginCentroids : public NDPluginDriver {
public:
    NDPluginCentroids(const char *portName, int queueSize,
                      int blockingCallbacks,
                      const char *NDArrayPort, int NDArrayAddr,
                      int maxBuffers, size_t maxMemory,
                      int priority, int stackSize);

    /* These methods override the virtual methods in the base class */
    void processCallbacks(NDArray *pArray);

protected:
    int NDPluginCentroidsThreshold;
    int NDPluginCentroidsBox;
    int NDPluginCentroidsSearchBox;
    int NDPluginCentroidsPixelPhoton;
    int NDPluginCentroidsPixelBgnd;
    int NDPluginCentroidsCOMPhoton;
    int NDPluginCentroidsOverlapMax;
    int NDPluginCentroidsSumMin;
    int NDPluginCentroidsSumMax;
    int NDPluginCentroidsFitPixels2D;
    int NDPluginCentroidsFitPixels1DX;
    int NDPluginCentroidsFitPixels1DY;
    int NDPluginCentroidsNPhotons;
    int NDPluginCentroidsParamsValid;
    int NDPluginCentroidsStatusMsg;

private:

};

#endif
