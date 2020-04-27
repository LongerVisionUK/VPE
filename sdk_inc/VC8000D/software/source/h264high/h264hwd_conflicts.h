#ifndef __H264_CONFILICTS__
#define __H264_CONFILICTS__

#define H264_RENAME_FUNC(func) H264##func

#define AllocateAsicBuffers H264_RENAME_FUNC(AllocateAsicBuffers)
#define ReleaseAsicBuffers H264_RENAME_FUNC(ReleaseAsicBuffers)
#define InitList H264_RENAME_FUNC(InitList)
#define ReleaseList H264_RENAME_FUNC(ReleaseList)
#define AllocateIdUsed H264_RENAME_FUNC(AllocateIdUsed)
#define AllocateIdFree H264_RENAME_FUNC(AllocateIdFree)
#define ReleaseId H264_RENAME_FUNC(ReleaseId)
#define GetDataById H264_RENAME_FUNC(GetDataById)
#define GetIdByData H264_RENAME_FUNC(GetIdByData)
#define IncrementRefUsage H264_RENAME_FUNC(IncrementRefUsage)
#define DecrementRefUsage H264_RENAME_FUNC(DecrementRefUsage)
#define MarkHWOutput H264_RENAME_FUNC(MarkHWOutput)
#define ClearHWOutput H264_RENAME_FUNC(ClearHWOutput)
#define MarkTempOutput H264_RENAME_FUNC(MarkTempOutput)
#define FinalizeOutputAll H264_RENAME_FUNC(FinalizeOutputAll)
#define ClearOutput H264_RENAME_FUNC(ClearOutput)
#define PopFreeBuffer H264_RENAME_FUNC(PopFreeBuffer)
#define PushFreeBuffer H264_RENAME_FUNC(PushFreeBuffer)
#define GetFreePicBuffer H264_RENAME_FUNC(GetFreePicBuffer)
#define GetFreeBufferCount H264_RENAME_FUNC(GetFreeBufferCount)
#define SetFreePicBuffer H264_RENAME_FUNC(SetFreePicBuffer)
#define IncrementDPBRefCount H264_RENAME_FUNC(IncrementDPBRefCount)
#define DecrementDPBRefCount H264_RENAME_FUNC(DecrementDPBRefCount)
#define IsBufferReferenced H264_RENAME_FUNC(IsBufferReferenced)
#define IsBufferOutput H264_RENAME_FUNC(IsBufferOutput)
#define MarkOutputPicCorrupt H264_RENAME_FUNC(MarkOutputPicCorrupt)
#define PushOutputPic H264_RENAME_FUNC(PushOutputPic)
#define PeekOutputPic H264_RENAME_FUNC(PeekOutputPic)
#define PopOutputPic H264_RENAME_FUNC(PopOutputPic)
#define RemoveTempOutputAll H264_RENAME_FUNC(RemoveTempOutputAll)
#define RemoveOutputAll H264_RENAME_FUNC(RemoveOutputAll)
#define IsOutputEmpty H264_RENAME_FUNC(IsOutputEmpty)
#define WaitOutputEmpty H264_RENAME_FUNC(WaitOutputEmpty)
#define WaitListNotInUse H264_RENAME_FUNC(WaitListNotInUse)
#define MarkIdAllocated H264_RENAME_FUNC(MarkIdAllocated)
#define MarkIdFree H264_RENAME_FUNC(MarkIdFree)
#define SetAbortStatusInList H264_RENAME_FUNC(SetAbortStatusInList)
#define ClearAbortStatusInList H264_RENAME_FUNC(ClearAbortStatusInList)
#define ResetOutFifoInList H264_RENAME_FUNC(ResetOutFifoInList)
#define Ceil H264_RENAME_FUNC(Ceil)
#define ScalingList H264_RENAME_FUNC(ScalingList)

#endif
