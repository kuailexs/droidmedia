/*
 * Copyright (C) 2014-2015-2015 Jolla Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authored by: Mohammed Hassan <mohammed.hassan@jolla.com>
 */

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <CameraService.h>
#include <AudioPolicyService.h>
#include <binder/MemoryHeapBase.h>
#include <MediaPlayerService.h>
#if ANDROID_MAJOR >= 6
#include <binder/BinderService.h>
#include <camera/ICameraService.h>
#include <binder/IInterface.h>
#include <cutils/multiuser.h>
#endif

// echo "persist.camera.shutter.disable=1" >> /system/build.prop

using namespace android;

#define BINDER_SERVICE_CHECK_INTERVAL 500000


#if ANDROID_MAJOR >= 6

#include <camera/ICameraServiceProxy.h>

class FakeCameraServiceProxy : public BinderService<FakeCameraServiceProxy>,
                        public BnCameraServiceProxy
{
public:
    static char const *getServiceName() {
        return "media.camera.proxy";
    }

    void pingForUserUpdate() {
    }

    void notifyCameraState(String16 cameraId, CameraState newCameraState) {
    }
};
#endif

int
main(int, char**)
{
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();

    MediaPlayerService::instantiate();
    CameraService::instantiate();
    AudioPolicyService::instantiate();

#if ANDROID_MAJOR >= 6
    FakeCameraServiceProxy::instantiate();
    // Camera service needs to be told which users may use the camera
    sp<IBinder> binder;
    do {
    	binder = sm->getService(String16("media.camera"));
        if (binder != NULL) {
            break;
        }
        ALOGW("DroidMedia: Camera service is not yet available, waiting...");
        usleep(BINDER_SERVICE_CHECK_INTERVAL);
    } while (true);

    sp<ICameraService> gCameraService = interface_cast<ICameraService>(binder);
    ALOGD("DroidMedia: Allowing use of the camera for users root and bin");
    int32_t users[2];
    users[0] = 0; users[1] = 1;
    gCameraService->notifySystemEvent(1, users, 2);
#endif

    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();

    return 0;
}
