#pragma once

/*
 *      Copyright (C) 2011-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <hwcomposerwindow/hwcomposer_window.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include <hybris/surface_flinger/surface_flinger_compatibility_layer.h>
extern struct SfSurface* sf_surface;
#include <EGL/egl.h>
// mac_l1: hack to override eglCreateWindowSurface and eglMakeCurrent
#define eglCreateWindowSurface( d, c, w, n ) sf_surface_get_egl_surface(sf_surface)
#define eglMakeCurrent( d, s1, s2, c ) ( sf_surface_make_current(sf_surface), EGL_TRUE )

#include "EGLNativeType.h"
#include "threads/Thread.h"

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
};

class CEGLNativeTypeHybris : public CEGLNativeType
{
public:
  CEGLNativeTypeHybris();
  virtual ~CEGLNativeTypeHybris();
  virtual std::string GetNativeName() const { return "hybris"; };
  virtual bool  CheckCompatibility();
  virtual void  Initialize();
  virtual void  Destroy();
  virtual int   GetQuirks() { return 0; };

  virtual bool  CreateNativeDisplay();
  virtual bool  CreateNativeWindow();
  virtual bool  GetNativeDisplay(XBNativeDisplayType **nativeDisplay) const;
  virtual bool  GetNativeWindow(XBNativeWindowType **nativeWindow) const;

  virtual bool  DestroyNativeWindow();
  virtual bool  DestroyNativeDisplay();

  virtual bool  GetNativeResolution(RESOLUTION_INFO *res) const;
  virtual bool  SetNativeResolution(const RESOLUTION_INFO &res);
  virtual bool  ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions);
  virtual bool  GetPreferredResolution(RESOLUTION_INFO *res) const;

  virtual bool  ShowWindow(bool show);

private:
  hw_module_t                *m_hwcModule;
  hwc_display_contents_1_t   **m_bufferList;
  hwc_composer_device_1_t    *m_hwcDevicePtr;
  HWComposerNativeWindow     *m_hwNativeWindow;
  ANativeWindow              *m_swNativeWindow;
};
