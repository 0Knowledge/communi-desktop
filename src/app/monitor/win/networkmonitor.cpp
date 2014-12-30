/*
  Copyright (C) 2008-2014 The Communi Project

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "networkmonitor.h"
#include <QtDebug>

NetworkMonitor::NetworkMonitor()
    : cookie(0), cp(NULL), manager(NULL), container(NULL), refcount(0)
{
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_NetworkListManager, NULL, CLSCTX_ALL,
                              IID_INetworkListManager, (LPVOID*)&manager);
        if (SUCCEEDED(hr)) {
            manager->AddRef();
            hr = manager->QueryInterface(IID_IConnectionPointContainer, (void**)&container);
            if (SUCCEEDED(hr)) {
                container->AddRef();
                hr = container->FindConnectionPoint(IID_INetworkListManagerEvents, &cp);
                if (SUCCEEDED(hr)) {
                    cp->AddRef();
                    hr = cp->Advise((IUnknown*)this, &cookie);
                }
            }
        }
    }
    if (FAILED(hr))
        qWarning() << "NetworkMonitor: COM failure:" << GetLastError();
    else
        qDebug() << "NetworkMonitor: up and running!";
}

NetworkMonitor::~NetworkMonitor()
{
    if (cp) {
        if (cookie)
            cp->Unadvise(cookie);
        cp->Release();
    }
    if (container)
        container->Release();
    if (manager)
        manager->Release();
    CoUninitialize();
}

ULONG STDMETHODCALLTYPE NetworkMonitor::AddRef()
{
    return ++refcount;
}

ULONG STDMETHODCALLTYPE NetworkMonitor::Release()
{
    return --refcount;
}

HRESULT STDMETHODCALLTYPE NetworkMonitor::QueryInterface(REFIID riid, void** obj)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *obj = (IUnknown*)this;
    else if (IsEqualIID(riid ,IID_INetworkListManagerEvents))
        *obj = (INetworkListManagerEvents*)this;
    else
        return E_NOINTERFACE;
    AddRef();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE NetworkMonitor::ConnectivityChanged(NLM_CONNECTIVITY connectivity)
{
    if (connectivity == NLM_CONNECTIVITY_DISCONNECTED)
        emit offline();
    else if (connectivity & NLM_CONNECTIVITY_IPV4_INTERNET ||
             connectivity & NLM_CONNECTIVITY_IPV6_INTERNET)
        emit online();
    return S_OK;
}
