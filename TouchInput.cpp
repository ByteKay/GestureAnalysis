#include "inputs/TouchInput.h"

#define _SAME_POSITION_GATE_VALUE_          5
#define _LONG_TOUCH_TIME_GATE_VALUE_        1500

#define _MAX_GESTURE_POINTS_                2

//-------------------------------------------------------- TouchInput::TouchInfo -------------------------------------------------------
TouchInput::TouchInfo::TouchInfo(unsigned int index) : mTouchIndex(index), mLastMoveDirection(FastMath::Vector2::Zero())
{
    mTouchTrack.ClearAndForceAllocation(32);
    mTouchTimer.StopTimer();
}

TouchInput::TouchInfo::~TouchInfo()
{
    Clear();
}

void TouchInput::TouchInfo::Clear()
{
    mLastMoveDirection = FastMath::Vector2::Zero();
    mTouchTimer.StopTimer();
    while (!mTouchTrack.IsEmpty())
        mTouchTrack.Pop();
}

bool TouchInput::TouchInfo::IsTouched()
{
    return !mTouchTrack.IsEmpty();
}

const FastMath::Vector2 &TouchInput::TouchInfo::GetFirstTouchPoint() const
{
    assert(!mTouchTrack.IsEmpty());
    return mTouchTrack[0];
}

const FastMath::Vector2 &TouchInput::TouchInfo::GetLastTouchPoint() const
{
    assert(!mTouchTrack.IsEmpty());
    return mTouchTrack[mTouchTrack.Size() - 1];
}

void TouchInput::TouchInfo::TryTouch(int x, int y)
{
    mTouchTimer.StopTimer();
    Clear();
    mTouchTrack.Push(FastMath::Vector2(x, y));
    mTouchTimer.StartTimer();
}

void TouchInput::TouchInfo::TryTouchMove(int x, int y)
{
    if (!IsTouched())
        TryTouch(x, y);
    const FastMath::Vector2 &lastPoint = GetLastTouchPoint();
    if ((fabs(x - lastPoint.x()) > _SAME_POSITION_GATE_VALUE_) || (fabs(y - lastPoint.y()) > _SAME_POSITION_GATE_VALUE_))
        mTouchTimer.StartTimer();
    mTouchTrack.Push(FastMath::Vector2(x, y));
    mLastMoveDirection = FastMath::Vector2(x - lastPoint.x(), y - lastPoint.y());
}

bool TouchInput::TouchInfo::IsLongTouch()
{
    if (!IsTouched())
        return false;
    bool result = mTouchTimer.GetTimeSlapped() > _LONG_TOUCH_TIME_GATE_VALUE_;
    if (result)
    {
        FastMath::Vector2 lastPoint = GetLastTouchPoint();
        Clear();
        TryTouch(lastPoint.x(), lastPoint.y());
    }
    return result;
}

void TouchInput::TouchInfo::TryReleaseTouch(int x, int y)
{
    //roc todo, try resolve gesture here, later
    Clear();
}


//----------------------------------------------------------- TouchInput ----------------------------------------------------------
TouchInput::TouchInput(unsigned int maxTouchCount) : mTouchInfoes(0), mMaxTouchCount(0)
{
    Initialize(maxTouchCount);
}

TouchInput::~TouchInput()
{
    Clear();
}

void TouchInput::Clear()
{
    for (unsigned int i=0; i<mMaxTouchCount; i++)
        delete mTouchInfoes[i];
    if (mTouchInfoes)
        free(mTouchInfoes);
    mTouchInfoes = 0;
    mMaxTouchCount = 0;
}

void TouchInput::SetMaxTouchCount(unsigned int count)
{
    if (mMaxTouchCount == count)
        return;
    Clear();
    mMaxTouchCount = count;
    Initialize(mMaxTouchCount);
}

void TouchInput::Initialize(unsigned int maxTouchCount)
{
    Clear();
    mMaxTouchCount = maxTouchCount;
    mTouchInfoes = (TouchInput::TouchInfo **)malloc(sizeof(TouchInput::TouchInfo *) * mMaxTouchCount);
    for (unsigned int i=0; i<mMaxTouchCount; i++)
        mTouchInfoes[i] = new TouchInput::TouchInfo(i);
}

void TouchInput::ProcessTouchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    if (contactIndex >= mMaxTouchCount)
        return;
    TouchInput::TouchInfo *touchInfo = mTouchInfoes[contactIndex];
    switch (evt)
    {
        case Touch::TOUCH_PRESS:
            touchInfo->TryTouch(x, y);
            break;
        case Touch::TOUCH_MOVE:
            touchInfo->TryTouchMove(x, y);
            break;
        case Touch::TOUCH_RELEASE:
            touchInfo->TryReleaseTouch(x, y);
            break;
    }
}

unsigned int TouchInput::GetCurrentTouchCount()
{
    unsigned int count = 0;
    for (unsigned int i=0; i<mMaxTouchCount; i++)
    {
        TouchInput::TouchInfo *touchInfo = mTouchInfoes[i];
        if (touchInfo->IsTouched())
            count ++;
    }
    return count;
}

unsigned int TouchInput::GetherTouchMovings(TouchInput::TouchInfo **touchInfos, FastMath::Vector2 *movings)
{
    unsigned int count = 0;
    for (unsigned int i=0; i<mMaxTouchCount; i++)
    {
        TouchInput::TouchInfo *touchInfo = mTouchInfoes[i];
        if (touchInfo->IsTouched())
        {
            touchInfos[count] = touchInfo;
            movings[count] = touchInfo->GetLastMoveDirection();
            count ++;
            if (count >= _MAX_GESTURE_POINTS_)
                break;
        }
    }
    return count;
}

TouchInput::Touch2Gestures TouchInput::TryGetTouch2Gesture(FastMath::Vector2 &value)
{
    TouchInput::TouchInfo *touchInfos[_MAX_GESTURE_POINTS_];
    FastMath::Vector2 lastDirections[_MAX_GESTURE_POINTS_];
    unsigned int count = GetherTouchMovings(touchInfos, lastDirections);

    if (count != 2)
        return TouchInput::GESTURE_NONE;

    FastMath::Vector2 lastPositions[_MAX_GESTURE_POINTS_];
    for (unsigned int i=0; i<count; i++)
        lastPositions[i] = touchInfos[i]->GetLastTouchPoint() - lastDirections[i];
    
    TouchInput::Touch2Gestures result = TouchInput::GESTURE_NONE;

    bool t1Hold = lastDirections[0].IsZero();
    bool t2Hold = lastDirections[1].IsZero();
    //two touch-points both moved
    if (!t1Hold && !t2Hold)
    {
        FastMath::Vector2 t1 = lastDirections[0];
        FastMath::Vector2 t2 = lastDirections[1];
        t1.Normalize();
        t2.Normalize();
        float dot = t1.DotProduct(t2);
        if (dot < 0.0f)
        {
            result = TouchInput::GESTURE_PINCH;
            float l = (lastPositions[1] - lastPositions[0]).Length();
            float d = (touchInfos[1]->GetLastTouchPoint() - touchInfos[0]->GetLastTouchPoint()).Length();
            if (d < l)
                d = -1.0f;
            else
                d = 1.0f;
            value.x() = 1.0f + d * (fabs(lastDirections[1].x()) + fabs(lastDirections[0].x())) / l;
            value.y() = 1.0f + d * (fabs(lastDirections[1].y()) + fabs(lastDirections[0].y())) / l;
        }
        else
        {
            result = TouchInput::GESTURE_MOVE;
            value.x() = (lastDirections[0].x() + lastDirections[1].x()) * 0.5f;
            value.y() = (lastDirections[0].y() + lastDirections[1].y()) * 0.5f;
        }
        return result;
    }
    //both touch-points are holdind
    if (t1Hold || t2Hold)
    {
        if (touchInfos[0]->IsLongTouch() && touchInfos[1]->IsLongTouch())
        {
            result = TouchInput::GESTURE_LONG_TAP;
            value.x() = (lastPositions[0].x() + lastPositions[1].x()) * 0.5f;
            value.y() = (lastPositions[0].y() + lastPositions[1].y()) * 0.5f;
        }
    }
    else
    {
        //one touch-point mnoved while another hold
        result = TouchInput::GESTURE_PINCH;
        float l = (lastPositions[1] - lastPositions[0]).Length();
        float d = (touchInfos[1]->GetLastTouchPoint() - touchInfos[0]->GetLastTouchPoint()).Length();
        if (d < l)
            d = -1.0f;
        else
            d = 1.0f;
        value.x() = 1.0f + d * (fabs(lastDirections[1].x()) + fabs(lastDirections[0].x())) / l;
        value.y() = 1.0f + d * (fabs(lastDirections[1].y()) + fabs(lastDirections[0].y())) / l;
        return result;
    }
    return result;
}

