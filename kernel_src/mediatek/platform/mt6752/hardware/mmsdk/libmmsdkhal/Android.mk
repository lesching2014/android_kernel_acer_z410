# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

################################################################################
#
################################################################################
LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SRC_FILES := ./mmsdkHAL.cpp \
                   ./imageTransformDevice.cpp \
                   ./gestureDevice.cpp \
                   ./effectDevice.cpp \


#-----------------------------------------------------------
LOCAL_C_INCLUDES += system/core/include 
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(MTK_PATH_PLATFORM)/hardware/include

#arcsoft camera feature
LOCAL_LDFLAGS += $(MY_ADAPTER_C_INCLUDES_PATH)/Arcsoft/Denoise/lib/libarcsoft_smart_denoise.a
LOCAL_LDFLAGS += $(MY_ADAPTER_C_INCLUDES_PATH)/Arcsoft/Nightshot/lib/libarcsoft_night_shot.a
LOCAL_LDFLAGS += $(MY_ADAPTER_C_INCLUDES_PATH)/Arcsoft/Nightshot/lib/libarcsoft_dynamic_light.a
LOCAL_LDFLAGS += $(MY_ADAPTER_C_INCLUDES_PATH)/Arcsoft/Platform/lib/mpbase.a

#-----------------------------------------------------------
# for fb 
LOCAL_WHOLE_STATIC_LIBRARIES +=  libcam.camadapter.scenario.shot
LOCAL_WHOLE_STATIC_LIBRARIES += libsched
LOCAL_SHARED_LIBRARIES += libcam_mmp
LOCAL_SHARED_LIBRARIES += libstlport
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcamdrv
LOCAL_SHARED_LIBRARIES += libcam.camshot
LOCAL_SHARED_LIBRARIES += libcam_utils
LOCAL_SHARED_LIBRARIES += libcam1_utils
LOCAL_SHARED_LIBRARIES += libcam.exif
LOCAL_SHARED_LIBRARIES += libaed 
LOCAL_SHARED_LIBRARIES += libcam.paramsmgr
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcam.halsensor
LOCAL_SHARED_LIBRARIES += libcamera_client libcamera_client_mtk
LOCAL_SHARED_LIBRARIES += libcam_hwutils


#end for fb 

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libcutils 
LOCAL_SHARED_LIBRARIES += liblog 
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libfeatureio 
#SImager
LOCAL_SHARED_LIBRARIES += libcam.iopipe 

#-----------------------------------------------------------
LOCAL_MODULE := mmsdk.default
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_CFLAGS += -Wall -Wextra -fvisibility=hidden

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
