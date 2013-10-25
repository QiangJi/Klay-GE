LOCAL_PATH := $(call my-dir)
SEVENZIP_PATH := $(LOCAL_PATH)
SEVENZIP_SRC_PATH := $(LOCAL_PATH)/../../../../CPP/7zip/Bundles/Format7zF

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(SEVENZIP_SRC_PATH)/../../../
LOCAL_PATH := $(SEVENZIP_SRC_PATH)
LOCAL_CFLAGS := -DNO_ASM -DNO_REGISTRY -DEXTRACT_ONLY -D_7ZIP_LARGE_PAGES -D_7ZIP_ST
LOCAL_SRC_FILES := \
		../../../Common/CRC.cpp \
		../../../Common/IntToString.cpp \
		../../../Common/MyVector.cpp \
		../../../Common/MyString.cpp \
		../../../Common/NewHandler.cpp \
		../../../Common/StringConvert.cpp \
		../../../Common/StringToInt.cpp \
		../../../Common/Wildcard.cpp \
		../../../Windows/PropVariant.cpp \
		../../../Windows/Synchronization.cpp \
		../../../Windows/System.cpp \
		../../Common/CreateCoder.cpp \
		../../Common/CWrappers.cpp \
		../../Common/FilterCoder.cpp \
		../../Common/InBuffer.cpp \
		../../Common/LimitedStreams.cpp \
		../../Common/LockedStream.cpp \
		../../Common/MethodId.cpp \
		../../Common/OutBuffer.cpp \
		../../Common/ProgressUtils.cpp \
		../../Common/StreamBinder.cpp \
		../../Common/StreamObjects.cpp \
		../../Common/StreamUtils.cpp \
		../../Common/VirtThread.cpp \
		../../Archive/ArchiveExports.cpp \
		../../Archive/DllExports2.cpp \
		../../Archive/Common/CoderMixer2.cpp \
		../../Archive/Common/CoderMixer2MT.cpp \
		../../Archive/Common/CoderMixer2ST.cpp \
		../../Archive/Common/CrossThreadProgress.cpp \
		../../Archive/Common/ItemNameUtils.cpp \
		../../Archive/Common/OutStreamWithCRC.cpp \
		../../Archive/Common/ParseProperties.cpp \
		../../Archive/7z/7zCompressionMode.cpp \
		../../Archive/7z/7zDecode.cpp \
		../../Archive/7z/7zExtract.cpp \
		../../Archive/7z/7zFolderOutStream.cpp \
		../../Archive/7z/7zHandler.cpp \
		../../Archive/7z/7zHeader.cpp \
		../../Archive/7z/7zIn.cpp \
		../../Archive/7z/7zProperties.cpp \
		../../Archive/7z/7zRegister.cpp \
		../../Compress/Bcj2Coder.cpp \
		../../Compress/Bcj2Register.cpp \
		../../Compress/BcjCoder.cpp \
		../../Compress/BcjRegister.cpp \
		../../Compress/BranchCoder.cpp \
		../../Compress/BranchMisc.cpp \
		../../Compress/BranchRegister.cpp \
		../../Compress/ByteSwap.cpp \
		../../Compress/BZip2Crc.cpp \
		../../Compress/BZip2Decoder.cpp \
		../../Compress/BZip2Register.cpp \
		../../Compress/CodecExports.cpp \
		../../Compress/CopyCoder.cpp \
		../../Compress/CopyRegister.cpp \
		../../Compress/Lzma2Decoder.cpp \
		../../Compress/Lzma2Register.cpp \
		../../Compress/LzmaDecoder.cpp \
		../../Compress/LzmaRegister.cpp \
		../../Compress/BitlDecoder.cpp \
		../../Compress/LzOutWindow.cpp \
		../../Compress/PpmdDecoder.cpp \
		../../Compress/PpmdRegister.cpp \
		../../Crypto/7zAes.cpp \
		../../Crypto/7zAesRegister.cpp \
		../../Crypto/MyAes.cpp \
		../../../../C/7zCrc.c \
		../../../../C/7zCrcOpt.c \
		../../../../C/Aes.c \
		../../../../C/AesOpt.c \
		../../../../C/Alloc.c \
		../../../../C/Bra.c \
		../../../../C/Bra86.c \
		../../../../C/BraIA64.c \
		../../../../C/CpuArch.c \
		../../../../C/Delta.c \
		../../../../C/Lzma2Dec.c \
		../../../../C/LzmaDec.c \
		../../../../C/Ppmd7.c \
		../../../../C/Ppmd7Dec.c \
		../../../../C/Sha256.c \
		../../../../C/Threads.c \


LOCAL_MODULE := 7zxa

include $(BUILD_STATIC_LIBRARY)
