// Copyright 2015 The Chromium Embedded Framework Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/osr/browser_platform_delegate_osr_linux.h"

#include "libcef/browser/browser_host_impl.h"

CefBrowserPlatformDelegateOsrLinux::CefBrowserPlatformDelegateOsrLinux(
    scoped_ptr<CefBrowserPlatformDelegateNative> native_delegate)
    : CefBrowserPlatformDelegateOsr(native_delegate.Pass()) {
}

CefWindowHandle
    CefBrowserPlatformDelegateOsrLinux::GetHostWindowHandle() const {
  return native_delegate_->window_info().parent_window;
}

