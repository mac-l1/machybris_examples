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

#include "system.h"
#include "EGLNativeTypeHybris.h"
#include "utils/log.h"
#include "guilib/gui3d.h"

#include "utils/StringUtils.h"
#include <stdio.h>

CEGLNativeTypeHybris::CEGLNativeTypeHybris()
{
  m_nativeWindow = NULL;
  m_hwNativeWindow = NULL;
  m_swNativeWindow = NULL;
}

CEGLNativeTypeHybris::~CEGLNativeTypeHybris() {
}

bool CEGLNativeTypeHybris::CheckCompatibility() {
  return true;
}

void CEGLNativeTypeHybris::Initialize() {
}

void CEGLNativeTypeHybris::Destroy() {
}

static struct SfClient* sf_client = NULL;
struct SfSurface* sf_surface = NULL;

bool CEGLNativeTypeHybris::CreateNativeDisplay() {
  struct SfClient* sf_client = sf_client_create();
  if (!sf_client) {
    printf("Problem creating sf client ... aborting now.");
    return false;
  }

  RESOLUTION_INFO res;
  if (!GetNativeResolution(&res))
    return false;

  SfSurfaceCreationParameters pars = { 0, 0, res.iWidth, res.iHeight, 
                                       -1, INT_MAX, 1.0f, 1, "MACs SPMC" };
  sf_surface = sf_surface_create(sf_client, &pars);
  if (!sf_surface) {
    printf("Problem creating sf surface ... aborting now.");
    return false;
  }

  m_nativeDisplay = EGL_DEFAULT_DISPLAY;
  return true;
}

bool CEGLNativeTypeHybris::CreateNativeWindow() {
/*
  sf_blank(0);
  sleep(1);
  sf_unblank(0);
*/
  m_swNativeWindow = (ANativeWindow *)sf_surface_get_egl_native_window(sf_surface);
  return true;
}

bool CEGLNativeTypeHybris::GetNativeDisplay(XBNativeDisplayType **nativeDisplay) const {
  if (!nativeDisplay)
    return false;

  *nativeDisplay = (XBNativeDisplayType*)&m_nativeDisplay;
  return true;
}

bool CEGLNativeTypeHybris::GetNativeWindow(XBNativeWindowType **nativeWindow) const {
  if (!nativeWindow)
    return false;

  *nativeWindow = (XBNativeWindowType*) &m_swNativeWindow;
  return (m_swNativeWindow != NULL);
}

bool CEGLNativeTypeHybris::DestroyNativeDisplay() {
  return true;
}

bool CEGLNativeTypeHybris::DestroyNativeWindow() {
  m_nativeWindow = NULL;
  return true;
}

bool CEGLNativeTypeHybris::GetNativeResolution(RESOLUTION_INFO *res) const {
  static int iWidth = 0;
  static int iHeight = 0;
  static int iRefreshRate = 0;
 
  if( !iWidth || !iHeight || !iRefreshRate )
  {
    iWidth = 1920;
    iHeight = 1080;
    iRefreshRate = 60;

    FILE *in;
    char buff[512];

    if(!(in = popen("cat /sys/class/display/*HDMI/mode", "r"))){
      printf("can't read display mode ... assuming default now."); 
      fflush( stdout );
    } else {
      if( fscanf(in, "%ux%up-%u", &iWidth, &iHeight, &iRefreshRate ) != 3 ) {
        iWidth = 1920;
        iHeight = 1080;
        iRefreshRate = 60;
      }

      while(fgets(buff, sizeof(buff), in)!=NULL) {
        (void)0;
      }
      pclose(in);
    }
  }

  res->iWidth        = iWidth;
  res->iHeight       = iHeight;
  res->fRefreshRate  = (float)iRefreshRate;

  res->dwFlags       = D3DPRESENTFLAG_PROGRESSIVE;
  res->iScreen       = 0;
  res->bFullScreen   = true;
  res->iSubtitles    = (int)(0.965 * res->iHeight);
  res->fPixelRatio   = 1.0f;
  res->iScreenWidth  = res->iWidth;
  res->iScreenHeight = res->iHeight;
  res->strMode       = StringUtils::Format("%dx%d @ %.2f%s - Full Screen", res->iScreenWidth, res->iScreenHeight, res->fRefreshRate,
  res->dwFlags & D3DPRESENTFLAG_INTERLACED ? "i" : "");
  CLog::Log(LOGNOTICE,"Current resolution: %s\n",res->strMode.c_str());
  return true;
}

bool CEGLNativeTypeHybris::SetNativeResolution(const RESOLUTION_INFO &res) {
  return false;
}

bool CEGLNativeTypeHybris::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions) {
  RESOLUTION_INFO res;
  if (GetNativeResolution(&res) && res.iWidth > 1 && res.iHeight > 1)
  {
    resolutions.push_back(res);
    return true;
  }
  return false;
}

bool CEGLNativeTypeHybris::GetPreferredResolution(RESOLUTION_INFO *res) const {
  if (GetNativeResolution(res))
    return true;
  return false;
}

bool CEGLNativeTypeHybris::ShowWindow(bool show) {
  return true;
}
