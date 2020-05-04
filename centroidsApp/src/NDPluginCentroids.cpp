/*
 * NDPluginCentroids.cpp
 *
 * Image processing plugin
 * Author: Stuart B. Wilkins
 *
 * Created May 1st, 2020
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <epicsString.h>
#include <epicsMutex.h>
#include <iocsh.h>

#include "NDArray.h"
#include "NDPluginCentroids.h"
#include <epicsExport.h>

#include "centroids.h"

static const char *driverName = "NDPluginCentroids";

/** Callback function that is called by the NDArray driver with new NDArray data.
  * Does image processing.
  * \param[in] pArray  The NDArray from the callback.
  */
void NDPluginCentroids::processCallbacks(NDArray *pArray)
{
  /* This function does array processing.
   * It is called with the mutex already locked.  It unlocks it during long calculations when private
   * structures don't need to be protected.
   */
  NDArray *pScratch = NULL;
  NDArray *pOutput = NULL;

  static const char* functionName = "processCallbacks";

  // Check that we are getting 2D images.
  if (pArray->ndims != 2) {
    NDPluginDriver::endProcessCallbacks(pArray, true, true);
    callParamCallbacks();
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
              "%s::%s Please use 2D images for the Centroiding plugin\n",
              driverName, functionName);
    return;
  }

  /* Call the base class method */
  NDPluginDriver::beginProcessCallbacks(pArray);

  int nDims = 2;
  size_t dims[2];
  dims[0] = pArray->dims[0].size;
  dims[1] = pArray->dims[1].size;

  // Set the parameters from the parameter library

  centroid_params<uint16_t, double> params;
  centroids_initialize_params<uint16_t, double>(&params);

  int threshold;
  getIntegerParam(NDPluginCentroidsThreshold,   &threshold);
  params.threshold = threshold;  // Avoid cast

  getIntegerParam(NDPluginCentroidsBox,          &params.box);
  getIntegerParam(NDPluginCentroidsSearchBox,    &params.search_box);
  getIntegerParam(NDPluginCentroidsCOMPhoton,    &params.com_photon_num);
  getIntegerParam(NDPluginCentroidsPixelPhoton,  &params.pixel_photon_num);
  getIntegerParam(NDPluginCentroidsPixelBgnd,    &params.pixel_bgnd_num);
  getIntegerParam(NDPluginCentroidsOverlapMax,   &params.overlap_max);
  getDoubleParam(NDPluginCentroidsSumMin,        &params.sum_min);
  getDoubleParam(NDPluginCentroidsSumMax,        &params.sum_max);

  int fit_2d, fit_1dx, fit_1dy;
  getIntegerParam(NDPluginCentroidsFitPixels2D,  &fit_2d);
  getIntegerParam(NDPluginCentroidsFitPixels1DX, &fit_1dx);
  getIntegerParam(NDPluginCentroidsFitPixels1DY, &fit_1dy);
  if (fit_2d) {
      params.fit_pixels |= CENTROIDS_FIT_2D;
  }
  if (fit_1dx) {
      params.fit_pixels |= CENTROIDS_FIT_1D_X;
  }
  if (fit_1dy) {
      params.fit_pixels |= CENTROIDS_FIT_1D_Y;
  }

  params.n = 1;
  params.x = dims[1];
  params.y = dims[0];
  params.return_map = true;
  params.return_pixels = CENTROIDS_STORE_NONE;

  if (centroids_calculate_params<uint16_t>(&params) != CENTROIDS_PARAMS_OK) {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
              "%s::%s Error in parameters\n",
              driverName, functionName);
    return;
  }

  pOutput = this->pNDArrayPool->alloc(nDims, dims, NDUInt16, 0, NULL);
  this->pNDArrayPool->convert(pArray, &pScratch, NDUInt16);

  // Setup the output

  PhotonTable<double>* photon_table(new PhotonTable<double>);
  std::vector<uint16_t>* pixels = NULL;

  /* Do the computationally expensive code with the lock released */

  this->unlock();

  size_t nphotons = centroids_process<uint16_t, double>((uint16_t*)pScratch->pData,
                                                        (uint16_t*)pOutput->pData,
                                                        photon_table,
                                                        pixels, params);

  // Take the lock again since we are accessing the parameter library and
  // these calculations are not time consuming
  this->lock();

  setIntegerParam(NDPluginCentroidsNPhotons, nphotons);

  if (pScratch != NULL) {
    pScratch->release();
  }

  NDPluginDriver::endProcessCallbacks(pOutput, false, true);
  callParamCallbacks();
}

/** Constructor for NDPluginCentroids; most parameters are simply passed to NDPluginDriver::NDPluginDriver.
 * After calling the base class constructor this method sets reasonable default values for all of the
 * parameters.
 * \param[in] portName The name of the asyn port driver to be created.
 * \param[in] queueSize The number of NDArrays that the input queue for this plugin can hold when
 *            NDPluginDriverBlockingCallbacks=0.  Larger queues can decrease the number of dropped arrays,
 *            at the expense of more NDArray buffers being allocated from the underlying driver's NDArrayPool.
 * \param[in] blockingCallbacks Initial setting for the NDPluginDriverBlockingCallbacks flag.
 *            0=callbacks are queued and executed by the callback thread; 1 callbacks execute in the thread
 *            of the driver doing the callbacks.
 * \param[in] NDArrayPort Name of asyn port driver for initial source of NDArray callbacks.
 * \param[in] NDArrayAddr asyn port driver address for initial source of NDArray callbacks.
 * \param[in] maxBuffers The maximum number of NDArray buffers that the NDArrayPool for this driver is
 *            allowed to allocate. Set this to -1 to allow an unlimited number of buffers.
 * \param[in] maxMemory The maximum amount of memory that the NDArrayPool for this driver is
 *            allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
 * \param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
 * \param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
 */
NDPluginCentroids::NDPluginCentroids(const char *portName, int queueSize, int blockingCallbacks,
    const char *NDArrayPort, int NDArrayAddr,
    int maxBuffers, size_t maxMemory,
    int priority, int stackSize)
  /* Invoke the base class constructor */
  : NDPluginDriver(portName, queueSize, blockingCallbacks,
      NDArrayPort, NDArrayAddr, 1, maxBuffers, maxMemory,
      asynInt32ArrayMask | asynFloat64ArrayMask | asynGenericPointerMask,
      asynInt32ArrayMask | asynFloat64ArrayMask | asynGenericPointerMask,
      0, 1, priority, stackSize, 1)
{
  char versionString[20];
  //static const char *functionName = "NDPluginCentroids";

  // Create parameter library

  createParam(NDPluginCentroidsThresholdString,
    asynParamInt32, &NDPluginCentroidsThreshold);
  createParam(NDPluginCentroidsBoxString,
    asynParamInt32, &NDPluginCentroidsBox);
  createParam(NDPluginCentroidsSearchBoxString,
    asynParamInt32, &NDPluginCentroidsSearchBox);
  createParam(NDPluginCentroidsPixelPhotonString,
    asynParamInt32, &NDPluginCentroidsPixelPhoton);
  createParam(NDPluginCentroidsPixelBgndString,
    asynParamInt32, &NDPluginCentroidsPixelBgnd);
  createParam(NDPluginCentroidsCOMPhotonString,
    asynParamInt32, &NDPluginCentroidsCOMPhoton);
  createParam(NDPluginCentroidsOverlapMaxString,
    asynParamInt32, &NDPluginCentroidsOverlapMax);
  createParam(NDPluginCentroidsSumMinString,
    asynParamFloat64, &NDPluginCentroidsSumMin);
  createParam(NDPluginCentroidsSumMaxString,
    asynParamFloat64, &NDPluginCentroidsSumMax);
  createParam(NDPluginCentroidsFitPixels2DString,
    asynParamInt32, &NDPluginCentroidsFitPixels2D);
  createParam(NDPluginCentroidsFitPixels1DXString,
    asynParamInt32, &NDPluginCentroidsFitPixels1DX);
  createParam(NDPluginCentroidsFitPixels1DYString,
    asynParamInt32, &NDPluginCentroidsFitPixels1DY);
  createParam(NDPluginCentroidsNPhotonsString,
    asynParamInt32, &NDPluginCentroidsNPhotons);

  /* Set the plugin type string */
  setStringParam(NDPluginDriverPluginType, "NDPluginCentroids");

  epicsSnprintf(versionString, sizeof(versionString), "%s.%s.%s",
      CENTROIDS_GIT_REV, CENTROIDS_GIT_BRANCH,
      CENTROIDS_GIT_VERSION);
  setStringParam(NDDriverVersion, versionString);

  /* Try to connect to the array port */
  connectToArrayPort();
}

/** Configuration command */
extern "C" int NDCentroidsConfigure(const char *portName, int queueSize, int blockingCallbacks,
    const char *NDArrayPort, int NDArrayAddr,
    int maxBuffers, size_t maxMemory,
    int priority, int stackSize)
{
  NDPluginCentroids *pPlugin = new NDPluginCentroids(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr,
      maxBuffers, maxMemory, priority, stackSize);
  return pPlugin->start();
}

/* EPICS iocsh shell commands */
static const iocshArg initArg0 = { "portName",iocshArgString};
static const iocshArg initArg1 = { "frame queue size",iocshArgInt};
static const iocshArg initArg2 = { "blocking callbacks",iocshArgInt};
static const iocshArg initArg3 = { "NDArrayPort",iocshArgString};
static const iocshArg initArg4 = { "NDArrayAddr",iocshArgInt};
static const iocshArg initArg5 = { "maxBuffers",iocshArgInt};
static const iocshArg initArg6 = { "maxMemory",iocshArgInt};
static const iocshArg initArg7 = { "priority",iocshArgInt};
static const iocshArg initArg8 = { "stackSize",iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
  &initArg1,
  &initArg2,
  &initArg3,
  &initArg4,
  &initArg5,
  &initArg6,
  &initArg7,
  &initArg8};
static const iocshFuncDef initFuncDef = {"NDCentroidsConfigure",9,initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
  NDCentroidsConfigure(args[0].sval, args[1].ival, args[2].ival,
      args[3].sval, args[4].ival, args[5].ival,
      args[6].ival, args[7].ival, args[8].ival);
}

extern "C" void NDCentroidsRegister(void)
{
  iocshRegister(&initFuncDef,initCallFunc);
}

extern "C" {
  epicsExportRegistrar(NDCentroidsRegister);
}
