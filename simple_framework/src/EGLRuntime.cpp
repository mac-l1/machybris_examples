/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "EGLRuntime.h"
#include "Platform.h"

#include <cstdlib>

#include <android-config.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stddef.h>
#include <hwcomposer_window.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include <malloc.h>
#include <sync/sync.h>

class HWComposer : public HWComposerNativeWindow
{
	private:
		hwc_layer_1_t *fblayer;
		hwc_composer_device_1_t *hwcdevice;
		hwc_display_contents_1_t **mlist;
	protected:
		void present(HWComposerNativeWindowBuffer *buffer);

	public:

		HWComposer(unsigned int width, unsigned int height, unsigned int format, hwc_composer_device_1_t *device, hwc_display_contents_1_t **mList, hwc_layer_1_t *layer);
		void set();	
};

HWComposer::HWComposer(unsigned int width, unsigned int height, unsigned int format, hwc_composer_device_1_t *device, hwc_display_contents_1_t **mList, hwc_layer_1_t *layer) : HWComposerNativeWindow(width, height, format)
{
	fblayer = layer;
	hwcdevice = device;
	mlist = mList;
}

void HWComposer::present(HWComposerNativeWindowBuffer *buffer)
{
	int oldretire = mlist[0]->retireFenceFd;
	mlist[0]->retireFenceFd = -1;
	fblayer->handle = buffer->handle;
	fblayer->acquireFenceFd = getFenceBufferFd(buffer);
	fblayer->releaseFenceFd = -1;
	int err = hwcdevice->prepare(hwcdevice, HWC_NUM_DISPLAY_TYPES, mlist);
	assert(err == 0);

	err = hwcdevice->set(hwcdevice, HWC_NUM_DISPLAY_TYPES, mlist);
	//assert(err == 0);
	setFenceBufferFd(buffer, fblayer->releaseFenceFd);

	if (oldretire != -1)
	{   
		sync_wait(oldretire, -1);
		close(oldretire);
	}
} 

namespace MaliSDK
{
    EGLDisplay EGLRuntime::display;
    EGLContext EGLRuntime::context;
    EGLSurface EGLRuntime::surface;
    EGLConfig EGLRuntime::config;

    void EGLRuntime::initializeEGL(OpenGLESVersion requestedAPIVersion)
    {
        Platform* platform = Platform::getInstance();

        EGLBoolean success = EGL_FALSE;

	EGLint num_config;
	EGLint attr[] = {       // some attributes to set up our egl-interface
		EGL_BUFFER_SIZE, 32,
		EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint ctxattr[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLBoolean rv;
	int err;	

        hw_module_t const* module = NULL;
        err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
        assert(err == 0);
        framebuffer_device_t* fbDev = NULL;
        framebuffer_open(module, &fbDev);

	hw_module_t *hwcModule = 0;
	hwc_composer_device_1_t *hwcDevicePtr = 0;

	err = hw_get_module(HWC_HARDWARE_MODULE_ID, (const hw_module_t **) &hwcModule);
	assert(err == 0);

	err = hwc_open_1(hwcModule, &hwcDevicePtr);
	assert(err == 0);

        //framebuffer_close(fbDev);

	hwcDevicePtr->blank(hwcDevicePtr, 0, 0);
        hwcDevicePtr->eventControl(hwcDevicePtr, 0, HWC_EVENT_VSYNC, 1);

	uint32_t configs[5];
	size_t numConfigs = 5;

	err = hwcDevicePtr->getDisplayConfigs(hwcDevicePtr, 0, configs, &numConfigs);
	assert (err == 0);

	int32_t attr_values[2];
	uint32_t attributes[] = { HWC_DISPLAY_WIDTH, HWC_DISPLAY_HEIGHT, HWC_DISPLAY_NO_ATTRIBUTE }; 

	hwcDevicePtr->getDisplayAttributes(hwcDevicePtr, 0,
			configs[0], attributes, attr_values);

	size_t size = sizeof(hwc_display_contents_1_t) + 2 * sizeof(hwc_layer_1_t);
	hwc_display_contents_1_t *list = (hwc_display_contents_1_t *) malloc(size);
	hwc_display_contents_1_t **mList = (hwc_display_contents_1_t **) malloc(HWC_NUM_DISPLAY_TYPES * sizeof(hwc_display_contents_1_t *));
	const hwc_rect_t r = { 0, 0, attr_values[0], attr_values[1] };

	int counter = 0;
	for (; counter < HWC_NUM_DISPLAY_TYPES; counter++)
		mList[counter] = list;

	hwc_layer_1_t *layer = &list->hwLayers[0];
	memset(layer, 0, sizeof(hwc_layer_1_t));
	layer->compositionType = HWC_FRAMEBUFFER;
	layer->hints = 0;
	layer->flags = 0;
	layer->handle = 0;
	layer->transform = 0;
	layer->blending = HWC_BLENDING_NONE;
	layer->sourceCrop = r;
	layer->displayFrame = r;
	layer->visibleRegionScreen.numRects = 1;
	layer->visibleRegionScreen.rects = &layer->displayFrame;
	layer->acquireFenceFd = -1;
	layer->releaseFenceFd = -1;
        layer->planeAlpha = 0xFF;
	layer = &list->hwLayers[1];
	memset(layer, 0, sizeof(hwc_layer_1_t));
	layer->compositionType = HWC_FRAMEBUFFER_TARGET;
	layer->hints = 0;
	layer->flags = 0;
	layer->handle = 0;
	layer->transform = 0;
	layer->blending = HWC_BLENDING_NONE;
	layer->sourceCrop = r;
	layer->displayFrame = r;
	layer->visibleRegionScreen.numRects = 1;
	layer->visibleRegionScreen.rects = &layer->displayFrame;
	layer->acquireFenceFd = -1;
	layer->releaseFenceFd = -1;
        layer->planeAlpha = 0xFF;

	list->retireFenceFd = -1;
	list->flags = HWC_GEOMETRY_CHANGED;
	list->numHwLayers = 2;


	HWComposer *win = new HWComposer(attr_values[0], attr_values[1], HAL_PIXEL_FORMAT_RGBA_8888, hwcDevicePtr, mList, &list->hwLayers[1]);

	display = eglGetDisplay(NULL);
	assert(eglGetError() == EGL_SUCCESS);
	assert(display != EGL_NO_DISPLAY);

	rv = eglInitialize(display, 0, 0);
	assert(eglGetError() == EGL_SUCCESS);
	assert(rv == EGL_TRUE);

	eglChooseConfig((EGLDisplay) display, attr, &config, 1, &num_config);
	assert(eglGetError() == EGL_SUCCESS);
	assert(rv == EGL_TRUE);

	surface = eglCreateWindowSurface((EGLDisplay) display, config, (EGLNativeWindowType) static_cast<ANativeWindow *> (win), NULL);
	assert(eglGetError() == EGL_SUCCESS);
	assert(surface != EGL_NO_SURFACE);

	context = eglCreateContext((EGLDisplay) display, config, EGL_NO_CONTEXT, ctxattr);
	assert(eglGetError() == EGL_SUCCESS);
	assert(context != EGL_NO_CONTEXT);

        /* Unconditionally bind to OpenGL ES API as we exit this function, since it's the default. */
        eglBindAPI(EGL_OPENGL_ES_API);
    }

    void EGLRuntime::setEGLSamples(EGLint requiredEGLSamples)
    {
//        configAttributes[1] = requiredEGLSamples;
    }

    void EGLRuntime::terminateEGL(void)
    {
        /* Shut down EGL. */
        eglBindAPI(EGL_OPENGL_ES_API);
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
    }
}
