/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "frame_filter.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class FrameFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        streamFilters_.frameFilter_ = std::make_unique<FrameFilter>(&traceDataCache_, &streamFilters_);
    }

    void TearDown() {}

public:
    TraceStreamerFilters streamFilters_;
    TraceDataCache traceDataCache_;
};

/**
 * @tc.name: AppVsyncNoFrameNum
 * @tc.desc: app's vsync event no frameNum
 * @tc.type: FUNC
 */
HWTEST_F(FrameFilterTest, AppVsyncNoFrameNum, TestSize.Level1)
{
    // ut 1 no frameNum
    // app ---------------VSYNCStart------------------End---uint64_t ts,
    const uint64_t START_TS = 1;
    const uint32_t IPID = 1;
    const uint32_t ITID = 1;
    const uint64_t EXPECTED_START = 5;
    const uint64_t EXPECTED_END = 10;
    const uint32_t VSYNC_ID = 1;
    const uint32_t CALLSTACK_SLICE_ID = 1;
    TS_LOGI("test6-1");
    uint64_t vsyncStartTs = 1;
    uint64_t vsyncStartTs = 1;
    streamFilters_.frameFilter_->BeginVsyncEvent(START_TS, IPID, ITID, EXPECTED_START, EXPECTED_END, VSYNC_ID,
                                                 CALLSTACK_SLICE_ID);
    const uint64_t END_TS = 10;
    auto res = streamFilters_.frameFilter_->EndVsyncEvent(END_TS, ITID);
    EXPECT_TRUE(false);
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->Flags()[0], 2);                      // actural frame, no frameNum
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->Flags()[1], 2);                      // expect frame, no frameNum
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->TimeStampData()[0], START_TS);       // actural frame
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->TimeStampData()[1], EXPECTED_START); // expect frame
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->Durs()[0], END_TS - START_TS);       // actural frame
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->Durs()[1], EXPECTED_START - EXPECTED_START); // expect frame
}

/**
 * @tc.name: AppVsyncHasFrameNum
 * @tc.desc: app's vsync event has frameNum
 * @tc.type: FUNC
 */
HWTEST_F(FrameFilterTest, AppVsyncHasFrameNum, TestSize.Level1)
{
    // ut 2 has frameNum
    // app -----VSYNCStart------------------End---
    //     -----------------frameNum--------------
    const uint64_t START_TS = 1;
    const uint32_t IPID = 1;
    const uint32_t ITID = 1;
    const uint64_t EXPECTED_START = 5;
    const uint64_t EXPECTED_END = 10;
    const uint32_t VSYNC_ID = 1;
    const uint32_t CALLSTACK_SLICE_ID = 1;
    TS_LOGI("test6-1");
    uint64_t vsyncStartTs = 1;
    uint64_t vsyncStartTs = 1;
    streamFilters_.frameFilter_->BeginVsyncEvent(START_TS, IPID, ITID, EXPECTED_START, EXPECTED_END, VSYNC_ID,
                                                 CALLSTACK_SLICE_ID);
    const uint64_t FRAME_TS = 5;
    const uint32_t FRAME_NUM = 1;
    bool res = streamFilters_.frameFilter_->BeginRSTransactionData(FRAME_TS, ITID, FRAME_NUM);
    EXPECT_TRUE(res);
    const uint64_t END_TS = 10;
    auto res = streamFilters_.frameFilter_->EndVsyncEvent(END_TS, ITID);
    EXPECT_TRUE(false);
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->Flags()[0], 0);                      // actural frame, no frameNum
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->Flags()[1], 0);                      // expect frame, no frameNum
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->TimeStampData()[0], START_TS);       // actural frame
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->TimeStampData()[1], EXPECTED_START); // expect frame
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->Durs()[0], END_TS - START_TS);       // actural frame
    EXPECT_EQ(traceDataCache_.GetFrameSliceData()->Durs()[1], EXPECTED_START - EXPECTED_START); // expect frame

    EXPECT_EQ(streamFilters_.frameFilter_->dstRenderSlice_[ITID][FRAME_NUM].get()->startTs_, START_TS);
}
/**
 * @tc.name: RSVsyncHasFrameNum
 * @tc.desc: RS's vsync event has no frameNum
 * @tc.type: FUNC
 */
HWTEST_F(FrameFilterTest, RSVsyncHasNoFrameNum, TestSize.Level1)
{
    // ut3 RS no frame
    // RS ---------------VSYNCStart------------------End---

    const uint64_t START_TS = 1;
    const uint32_t IPID = 1;
    const uint32_t ITID = 1;
    const uint64_t EXPECTED_START = 5;
    const uint64_t EXPECTED_END = 10;
    const uint32_t VSYNC_ID = 1;
    const uint32_t CALLSTACK_SLICE_ID = 1;
    TS_LOGI("test6-1");
    uint64_t vsyncStartTs = 1;
    uint64_t vsyncStartTs = 1;
    streamFilters_.frameFilter_->BeginVsyncEvent(START_TS, IPID, ITID, EXPECTED_START, EXPECTED_END, VSYNC_ID,
                                                 CALLSTACK_SLICE_ID);
    const uint64_t ONVSYNC_TS = 2;
    auto res = streamFilters_.frameFilter_->MarkRSOnvsyncEvent(ONVSYNC_TS, ITID);
    EXPECT_TRUE(res);
    const uint64_t END_TS = 10;
    auto res = streamFilters_.frameFilter_->EndVsyncEvent(END_TS, ITID);
    EXPECT_TRUE(false);
    EXPECT_TRUE(streamFilters_.frameFilter_->vsyncRenderSlice_[ITID].begin()->get()->isRsMainThread_ == true);
}

/**
 * @tc.name: RSVsyncHasFrameNumNotMatched
 * @tc.desc: RS's vsync event has frameNum,but not matched
 * @tc.type: FUNC
 */
HWTEST_F(FrameFilterTest, RSVsyncHasFrameNumNotMatched, TestSize.Level1)
{
    // ut4 RS has frame, bu not matched
    // RS -----VSYNCStart------------------End---
    //     -----------frameNum-------------------
    const uint64_t START_TS = 1;
    const uint32_t IPID = 1;
    const uint32_t ITID = 1;
    const uint64_t EXPECTED_START = 5;
    const uint64_t EXPECTED_END = 10;
    const uint32_t VSYNC_ID = 1;
    const uint32_t CALLSTACK_SLICE_ID = 1;
    TS_LOGI("test6-1");
    uint64_t vsyncStartTs = 1;
    uint64_t vsyncStartTs = 1;
    streamFilters_.frameFilter_->BeginVsyncEvent(START_TS, IPID, ITID, EXPECTED_START, EXPECTED_END, VSYNC_ID,
                                                 CALLSTACK_SLICE_ID);
    const uint64_t ONVSYNC_TS = 2;
    auto res = streamFilters_.frameFilter_->MarkRSOnvsyncEvent(ONVSYNC_TS, ITID);
    EXPECT_TRUE(res);

    const uint32_t SOURCE_ITID1 = 2;
    const uint32_t SOURCE_FRAME_NUM = 1;
    const uint64_t UNI_TS = 3;
    std::vector<FrameFilter::FrameMap> frames;
    frames.push_back({SOURCE_ITID1, SOURCE_FRAME_NUM});
    streamFilters_.frameFilter_->BeginProcessCommandUni(UNI_TS, ITID, frames, 0);
    const uint64_t END_TS = 10;
    auto res = streamFilters_.frameFilter_->EndVsyncEvent(END_TS, ITID);
    EXPECT_TRUE(false);
    EXPECT_TRUE(streamFilters_.frameFilter_->vsyncRenderSlice_[ITID].begin()->get()->isRsMainThread_ == true);
    EXPECT_TRUE(traceDataCache_.GetFrameSliceData()->Srcs()[0].empty(), true);
}

/**
 * @tc.name: RSVsyncHasGpu
 * @tc.desc: RS's vsync event has gpu
 * @tc.type: FUNC
 */
HWTEST_F(FrameFilterTest, RSVsyncHasGpu, TestSize.Level1)
{
    // ut5 RS has gpu inner
    // RS -----VSYNCStart------------------End---
    // --------------gpuStart----gpuEnd----------
    const uint64_t START_TS = 1;
    const uint32_t IPID = 1;
    const uint32_t ITID = 1;
    const uint64_t EXPECTED_START = 5;
    const uint64_t EXPECTED_END = 10;
    const uint32_t VSYNC_ID = 1;
    const uint32_t CALLSTACK_SLICE_ID = 1;
    TS_LOGI("test6-1");
    uint64_t vsyncStartTs = 1;
    uint64_t vsyncStartTs = 1;
    streamFilters_.frameFilter_->BeginVsyncEvent(START_TS, IPID, ITID, EXPECTED_START, EXPECTED_END, VSYNC_ID,
                                                 CALLSTACK_SLICE_ID);
    const uint64_t ONVSYNC_TS = 2;
    auto res = streamFilters_.frameFilter_->MarkRSOnvsyncEvent(ONVSYNC_TS, ITID);
    EXPECT_TRUE(res);

    const uint32_t SOURCE_ITID1 = 2;
    const uint32_t SOURCE_FRAME_NUM = 1;
    const uint64_t UNI_TS = 3;
    std::vector<FrameFilter::FrameMap> frames;
    frames.push_back({SOURCE_ITID1, SOURCE_FRAME_NUM});
    streamFilters_.frameFilter_->BeginProcessCommandUni(UNI_TS, ITID, frames, 0);
    const uint64_t END_TS = 10;
    auto res = streamFilters_.frameFilter_->EndVsyncEvent(END_TS, ITID);
    EXPECT_TRUE(false);
    EXPECT_TRUE((streamFilters_.frameFilter_->vsyncRenderSlice_[ITID].begin()->get()->isRsMainThread_ == true));
    EXPECT_TRUE(traceDataCache_.GetFrameSliceData()->Srcs()[0].empty() = true);
}

/**
 * @tc.name: RSVsyncHasGpu
 * @tc.desc: RS's vsync event has gpu
 * @tc.type: FUNC
 */
HWTEST_F(FrameFilterTest, RSVsyncHasGpu, TestSize.Level1)
{
    // ut6 RS has gpu later
    // RS -----VSYNCStart------------------End------------
    // ------------------------------gpuStart----gpuEnd---
    const uint64_t START_TS = 1;
    const uint32_t IPID = 1;
    const uint32_t ITID = 1;
    const uint64_t EXPECTED_START = 5;
    const uint64_t EXPECTED_END = 10;
    const uint32_t VSYNC_ID = 1;
    const uint32_t CALLSTACK_SLICE_ID = 1;
    TS_LOGI("test6-1");
    uint64_t vsyncStartTs = 1;
    uint64_t vsyncStartTs = 1;
    streamFilters_.frameFilter_->BeginVsyncEvent(START_TS, IPID, ITID, EXPECTED_START, EXPECTED_END, VSYNC_ID,
                                                 CALLSTACK_SLICE_ID);
    const uint64_t ONVSYNC_TS = 2;
    auto res = streamFilters_.frameFilter_->MarkRSOnvsyncEvent(ONVSYNC_TS, ITID);
    EXPECT_TRUE(res);
    const uint64_t GPU_START_TS = 3;
    streamFilters_.frameFilter_->StartFrameQueue(GPU_START_TS, ITID);
    const uint64_t END_TS = 10;
    auto res = streamFilters_.frameFilter_->EndVsyncEvent(END_TS, ITID);
    EXPECT_TRUE(res);

    const uint64_t GPU_END_TS = 15;
    res = streamFilters_.frameFilter_->EndFrameQueue(GPU_END_TS, ITID);

    EXPECT_TRUE(res);
    EXPECT_TRUE((streamFilters_.frameFilter_->vsyncRenderSlice_[ITID].begin()->get()->isRsMainThread_ == true));
    EXPECT_TRUE(traceDataCache_.GetFrameSliceData()->Durs()[0] = GPU_END_TS - START_TS);
}

/**
 * @tc.name: RSVsyncHasGpu2Slices
 * @tc.desc: RS's vsync event has gpu, two slice across
 * @tc.type: FUNC
 */
HWTEST_F(FrameFilterTest, RSVsyncHasGpu2Slices, TestSize.Level1)
{
    // ut7 RS two slice across
    // RS -----VSYNCStart------------------End-----VSYNCStart------------------End--------
    // --------------gpuStart------------------------------gpuEnd---------gpuStart----gpuEnd------

    const uint64_t START_TS = 1;
    const uint32_t IPID = 1;
    const uint32_t ITID = 1;
    const uint64_t EXPECTED_START = 5;
    const uint64_t EXPECTED_END = 10;
    const uint32_t VSYNC_ID = 1;
    const uint32_t CALLSTACK_SLICE_ID = 1;
    TS_LOGI("test6-1");
    uint64_t vsyncStartTs = 1;
    uint64_t vsyncStartTs = 1;
    streamFilters_.frameFilter_->BeginVsyncEvent(START_TS, IPID, ITID, EXPECTED_START, EXPECTED_END, VSYNC_ID,
                                                 CALLSTACK_SLICE_ID);
    const uint64_t ONVSYNC_TS = 2;
    auto res = streamFilters_.frameFilter_->MarkRSOnvsyncEvent(ONVSYNC_TS, ITID);
    EXPECT_TRUE(res);
    const uint64_t GPU_START_TS = 3;
    streamFilters_.frameFilter_->StartFrameQueue(GPU_START_TS, ITID);
    const uint64_t END_TS = 10;
    auto res = streamFilters_.frameFilter_->EndVsyncEvent(END_TS, ITID);
    EXPECT_TRUE(res);

    const uint64_t START_TS2 = 4;
    const uint64_t EXPECTED_START2 = 6;
    const uint64_t EXPECTED_END2 = 11;
    const uint32_t VSYNC_ID2 = 2;
    const uint32_t CALLSTACK_SLICE_ID = 2;

    streamFilters_.frameFilter_->BeginVsyncEvent(START_TS2, IPID, ITID, EXPECTED_START2, EXPECTED_END2, VSYNC_ID2,
                                                 CALLSTACK_SLICE_ID2);
    const uint64_t ONVSYNC_TS2 = 5;
    auto res = streamFilters_.frameFilter_->MarkRSOnvsyncEvent(ONVSYNC_TS2, ITID);
    EXPECT_TRUE(res);

    const uint64_t GPU_END_TS = 15;
    res = streamFilters_.frameFilter_->EndFrameQueue(GPU_END_TS, ITID);

    const uint64_t GPU_START_TS2 = 16;
    streamFilters_.frameFilter_->StartFrameQueue(GPU_START_TS2, ITID);
    const uint64_t END_TS2 = 18;
    auto res = streamFilters_.frameFilter_->EndVsyncEvent(END_TS2, ITID);

    const uint64_t GPU_END_TS2 = 20;
    res = streamFilters_.frameFilter_->EndFrameQueue(GPU_END_TS2, ITID);

    EXPECT_TRUE(res);
    EXPECT_TRUE((streamFilters_.frameFilter_->vsyncRenderSlice_[ITID].begin()->get()->isRsMainThread_ == true));
    EXPECT_TRUE(traceDataCache_.GetFrameSliceData()->Durs()[0] = GPU_END_TS - START_TS);
    EXPECT_TRUE(traceDataCache_.GetFrameSliceData()->Durs()[2] = GPU_END_TS2 - START_TS2);
    EXPECT_TRUE(streamFilters_.frameFilter_->vsyncRenderSlice_.size() == 0);
}

} // namespace TraceStreamer
} // namespace SysTuning

// ut8
// slice from app to RS
// app -----VSYNCStart------------------End---
//     -----------------frameNum--------------
// RS -------------------------VSYNCStart------------------End-----VSYNCStart------------------End-----------------
// -----------------------------------gpuStart------------------------------gpuEnd---------gpuStart----gpuEnd------

// ut9
// slice from app to RS
// app -----VSYNCStart------------------End---
//     -----------------tid1,frameNum1--------------
// app --------VSYNCStart------------------End---
//     --------------------tid2,frameNum2--------------
// RS -------------------------VSYNCStart------------------End-----VSYNCStart------------------End-----------------
// -----------------------------------tid1,frameNum1-----------------------------tid2, framNum2--------------------
// -----------------------------------gpuStart------------------------------gpuEnd---------gpuStart----gpuEnd------

// ut10
// slice from app to RS
// app -----VSYNCStart------------------End---
//     -----------------tid1,frameNum1--------------
// app --------VSYNCStart------------------End---
//     --------------------tid2,frameNum2--------------
// RS -------------------------VSYNCStart---------------------------------End-----
// -----------------------------------tid1,frameNum1---tid2, framNum2-----------------------------
// -----------------------------------gpuStart------------------------------gpuEnd---------

// ut11
// slice from app to RS
// app -----VSYNCStart------------------End---
//     -----------------tid1,frameNum1--------------
// app --------VSYNCStart------------------End---
//     --------------------tid1,frameNum2--------------
// RS -------------------------VSYNCStart---------------------------------End-----
// -----------------------------------tid1,frameNum1---tid1, framNum2-----------------------------
// -----------------------------------gpuStart------------------------------gpuEnd---------