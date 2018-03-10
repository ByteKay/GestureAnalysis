#include "input/BaseGestureRecognizer.h"

static unsigned int _MAX_INTERVAL_OF_DOUBLE_CLICK_      = 300;
static unsigned int _MIN_STEADY_TIME_FOR_DRAG           = 200;
static unsigned int _MIN_TIME_FOR_LONG_TAP              = 500;
static unsigned int _MAX_TIME_FOR_SWIPE                 = 700;
static float _MIN_X_RATIO_FOR_ARC                       = 0.5f;
static float _MIN_Y_CHANGE_PERSENT_FOR_ARC              = 0.25f;
static float _MAX_DISTANCE_RATIO_FOR_STEADY             = 0.005f;
static float _MAX_ANGLE_COS_VALUE_FOR_ROTATE            = 0.98f;
static float _MAX_SWIPE_DURATION_FOR_WHOLE_SCREEN       = 0.5f;

enum TouchQueueChangingMode
{
    TQC_NONE        = 0,
    TQC_PRESS       = 1,
    TQC_MOVE        = 2,
    TQC_RELEASE     = 3,
};

//--------------------------------------------- BaseGestureRecognizer ---------------------------------------------
BaseGestureRecognizer::BaseGestureRecognizer() : mId("BaseGestureRecognizer")
{
    mCurrentGestureEvent = new BaseGestureEvent();
    InitializeDefaultParameters();
}

BaseGestureRecognizer::~BaseGestureRecognizer()
{
    SAFE_DELETE(mCurrentGestureEvent);
}

BaseGestureRecognizer *BaseGestureRecognizer::Create(const char *id)
{
    if (strcmp("BaseGestureRecognizer", id) == 0)
        return new BaseGestureRecognizer();
    return 0;
}

void BaseGestureRecognizer::Initialize()
{
    InitializeDefaultParameters();
}

void BaseGestureRecognizer::InitializeDefaultParameters()
{
    mMaxIntervalOfDoubleClick = _MAX_INTERVAL_OF_DOUBLE_CLICK_;
    mMinSteadyTimeForDrag = _MIN_STEADY_TIME_FOR_DRAG;
    mMinTimeForLongTap = _MIN_TIME_FOR_LONG_TAP;
    mMinYChangePersentForArc = _MIN_Y_CHANGE_PERSENT_FOR_ARC;
    mMaxSwipeDuration = _MAX_TIME_FOR_SWIPE;

    mMaxAngleCosValForRotate = _MAX_ANGLE_COS_VALUE_FOR_ROTATE;
    
    unsigned int width = Game::GetInstance()->GetViewport().width;
    mMinXDistanceForArc = (int)(_MIN_X_RATIO_FOR_ARC * width + 0.5f);
    
    mMaxSteadyMoveDistanceX = (int)(_MAX_DISTANCE_RATIO_FOR_STEADY * width + 0.5f);
    unsigned int height = Game::GetInstance()->GetViewport().height;
    mMaxSteadyMoveDistanceY = (int)(_MAX_DISTANCE_RATIO_FOR_STEADY * height + 0.5f);
    
    mMinSpeedForSwipe = sqrtf((float)((width * width) + (height * height))) / _MAX_SWIPE_DURATION_FOR_WHOLE_SCREEN;
}

void BaseGestureRecognizer::ResetCurrentGesture()
{
    mCurrentGestureEvent->~BaseGestureEvent();
    new (mCurrentGestureEvent) BaseGestureEvent();
}

BaseGestureRecognizer::TouchQueueInfomation &BaseGestureRecognizer::FindQueueInfomation(TouchQueue *queue)
{
    static BaseGestureRecognizer::TouchQueueInfomation _empty_infomation;
    for (unsigned int i=0; i<mChangedTouchQueues.Size(); i++)
    {
        if (mChangedTouchQueues[i].touchQueue == queue)
            return mChangedTouchQueues[i];
    }
    return _empty_infomation;
}

void BaseGestureRecognizer::TryRemoveTouchQueueInfomation(TouchQueue *queue)
{
    for (unsigned int i=0; i<mChangedTouchQueues.Size(); i++)
    {
        if (mChangedTouchQueues[i].touchQueue == queue)
        {
            mChangedTouchQueues.RemoveAtIndex(i);
            queue->Clear();
            break;
        }
    }
}

bool BaseGestureRecognizer::TryAddTouchQueueChanging(TouchQueue *queue, int changingMode, HexTime time)
{
    BaseGestureRecognizer::TouchQueueInfomation &info = FindQueueInfomation(queue);
    if (info.IsEmpty())
    {
        if (changingMode == TQC_PRESS)
        {
            BaseGestureRecognizer::TouchQueueInfomation tqcInfo(queue, changingMode, time);
            mChangedTouchQueues.Push(tqcInfo);
            return true;
        }
        return false;
    }
    else
    {
        if (changingMode == TQC_RELEASE)
        {
            assert((info.lastChangingMode == TQC_PRESS) || (info.lastChangingMode == TQC_MOVE));
            info.releaseTime = time;
            info.repeatTimes ++;
        }
        bool changed = (info.lastChangingMode == changingMode);
        info.lastChangingMode = changingMode;
        
        return changed;
    }
}


void BaseGestureRecognizer::OnTapState(TouchQueueInfomation &info, HexTime time)
{
    int x, y;
    if (!info.touchQueue->IsActived())
    {
        // touch release
        if (info.repeatTimes <= 1)
        {
            info.touchQueue->GetTrackStartingPosition(x, y);
            if (info.releaseTime + mMaxIntervalOfDoubleClick < time)
            {
                // tap event
                ResetCurrentGesture();
                new (mCurrentGestureEvent) GestureTapEvent(x, y, time, 1);
            }
            else
            {
                mChangedTouchQueues.Push(info);
            }
            return;
        }
        else
        {
            // double click
            info.curState = STATE_DOUBLE_TAP;
            mChangedTouchQueues.Push(info);
            return;
        }
    }
    else
    {
        info.touchQueue->GetAbsMaxMovingDistance(x, y);
        if ((x > mMaxSteadyMoveDistanceX) || (y > mMaxSteadyMoveDistanceY))
        {
            // point moved, change to swipe or move state
            float maxSpeed;
            float avgSpeed;
            bool gotSpeed = info.touchQueue->GetMovingSpeeds(maxSpeed, avgSpeed);
            if (gotSpeed && (maxSpeed < mMinSpeedForSwipe))
            {
                info.curState = STATE_MOVE;
            }
            else
            {
                info.curState = STATE_SWIPE;
            }
            mChangedTouchQueues.Push(info);
            return;
        }
        //printf("OnTapState %u\n", info.touchQueue->GetCurrentDuration(time));
        info.touchQueue->GetTrackStartingPosition(x, y);
        if (info.touchQueue->GetCurrentDuration(time) > mMinSteadyTimeForDrag)
        {
            info.curState = STATE_DRAG;
            /*
            ResetCurrentGesture();
            new (mCurrentGestureEvent) GestureDragEvent(x, y, time, 1);
            */
            mChangedTouchQueues.Push(info);
            return;
        }
        mChangedTouchQueues.Push(info);
    }
}

void BaseGestureRecognizer::OnSwipeState(TouchQueueInfomation &info, HexTime time)
{
    int x, y;
    info.touchQueue->GetTrackStartingPosition(x, y);
    if (!info.touchQueue->IsActived() || (info.touchQueue->GetCurrentDuration(time) >= mMaxSwipeDuration))
    {
        ResetCurrentGesture();

        TouchQueue::ArcShape arcType;
        TouchQueue::Direction direction;
        bool isArc = info.touchQueue->IsArcTrack(mMinXDistanceForArc, mMinYChangePersentForArc, arcType, direction);
        if (isArc)
            new (mCurrentGestureEvent) GestureArcEvent(x, y, time, 1, arcType, direction);
        else
            new (mCurrentGestureEvent) GestureSwipeEvent(x, y, time, 1, direction);
        
        //force deactive the touch queue when it expired
        if (info.touchQueue->IsActived())
            info.touchQueue->ForceReleaseTouch();
    }
    else
    {
        mChangedTouchQueues.Push(info);
    }
}

void BaseGestureRecognizer::OnLongTapState(TouchQueueInfomation &info, HexTime time)
{
    int x, y;
    info.touchQueue->GetTrackStartingPosition(x, y);
    if (!info.touchQueue->IsActived())
    {
        ResetCurrentGesture();
        new (mCurrentGestureEvent) GestureLongTapEvent(x, y, time, 1, info.touchQueue->GetDuration());
    }
    else
    {
        mChangedTouchQueues.Push(info);
    }
}

void BaseGestureRecognizer::OnDoubleTapState(TouchQueueInfomation &info, HexTime time)
{
    int x, y;
    if (!info.touchQueue->IsActived())
    {
        ResetCurrentGesture();
        info.touchQueue->GetTrackStartingPosition(x, y);
        new (mCurrentGestureEvent) GestureDoubleClickEvent(x, y, time, 1);
    }
    else
    {
        info.touchQueue->GetAbsMaxMovingDistance(x, y);
        if ((x > mMaxSteadyMoveDistanceX) || (y > mMaxSteadyMoveDistanceY))
        {
            // point moved, change to swipe state
            info.curState = STATE_SWIPE;
            mChangedTouchQueues.Push(info);
            return;
        }
        if (info.touchQueue->GetCurrentDuration(time) > mMinSteadyTimeForDrag)
        {
            info.curState = STATE_DRAG;
            mChangedTouchQueues.Push(info);
            return;
        }
        mChangedTouchQueues.Push(info);
    }
}

void BaseGestureRecognizer::OnMoveState(TouchQueueInfomation &info, HexTime time)
{
    int x, y;
    info.touchQueue->GetTrackEndingPosition(x, y);
    if (!info.touchQueue->IsActived())
    {
        ResetCurrentGesture();
        new (mCurrentGestureEvent) GestureEndMoveEvent(x, y, time, 1);
    }
    else
    {
        ResetCurrentGesture();
        new (mCurrentGestureEvent) GestureMoveEvent(x, y, time, 1);
        mChangedTouchQueues.Push(info);
    }
}

void BaseGestureRecognizer::OnDragState(TouchQueueInfomation &info, HexTime time)
{
    int x, y;
    if (!info.touchQueue->IsActived())
    {
        //release touch in drag state, triggered tap or long-tap event
        ResetCurrentGesture();
        info.touchQueue->GetTrackStartingPosition(x, y);
        if (info.touchQueue->GetCurrentDuration(time) >= mMinTimeForLongTap)
            new (mCurrentGestureEvent) GestureLongTapEvent(x, y, time, 1, info.touchQueue->GetDuration());
        else
            new (mCurrentGestureEvent) GestureTapEvent(x, y, time, 1);
    }
    else
    {
        info.touchQueue->GetAbsMaxMovingDistance(x, y);
        if ((x > mMaxSteadyMoveDistanceX) || (y > mMaxSteadyMoveDistanceY))
        {
            // point moved, trigger drag event and change to drag-move state
            info.touchQueue->GetTrackStartingPosition(x, y);
            ResetCurrentGesture();
            new (mCurrentGestureEvent) GestureDragEvent(x, y, time, 1);
            info.curState = STATE_DRAG_MOVE;
            mChangedTouchQueues.Push(info);
            return;
        }
        mChangedTouchQueues.Push(info);
    }
}

void BaseGestureRecognizer::OnDragMoveState(TouchQueueInfomation &info, HexTime time)
{
    int x, y;
    info.touchQueue->GetTrackEndingPosition(x, y);
    if (!info.touchQueue->IsActived())
    {
        ResetCurrentGesture();
        new (mCurrentGestureEvent) GestureDropEvent(x, y, time, 1);
    }
    else
    {
        ResetCurrentGesture();
        new (mCurrentGestureEvent) GestureDragMoveEvent(x, y, time, 1);
        mChangedTouchQueues.Push(info);
    }
}

static bool _inMultiTouchMove = false;
static int _lastMultiTouchMoveX = 0;
static int _lastMultiTouchMoveY = 0;

void BaseGestureRecognizer::OnMultiTouch(HexTime time)
{
    DataStructures::Queue<TouchQueue *> temp;
    int count = mChangedTouchQueues.Size();
    for (int i = 0; i < count; ++i)
    {
        if (mChangedTouchQueues[i].touchQueue->IsActived())
        {
            temp.Push(mChangedTouchQueues[i].touchQueue);
            mChangedTouchQueues[i].curState = STATE_MULTI;
        }
    }
    count = temp.Size();
    if (count == 2) // rotate pinch
    {
        int x0, y0, x1, y1;
        temp[0]->GetTrackStartingPosition(x0, y0);
        FastMath::Vector3 p0 = FastMath::Vector3(x0, y0, 0.0f);
        temp[0]->GetTrackEndingPosition(x1, y1);
        FastMath::Vector3 p1 = FastMath::Vector3(x1, y1, 0.0f);
        FastMath::Vector3 v0 = FastMath::Vector3(x1 - x0, y1 - y0, 0.0f);
        temp[1]->GetTrackStartingPosition(x0, y0);
        FastMath::Vector3 p2 = FastMath::Vector3(x0, y0, 0.0f);
        temp[1]->GetTrackEndingPosition(x1, y1);
        FastMath::Vector3 p3 = FastMath::Vector3(x1, y1, 0.0f);
        FastMath::Vector3 v1 = FastMath::Vector3(x1 - x0, y1 - y0, 0.0f);
        float len1 = v0.Length();
        float len2 = v1.Length();
        if ((len1 < _MAX_DISTANCE_RATIO_FOR_STEADY) && (len2 < _MAX_DISTANCE_RATIO_FOR_STEADY))
        {
            //hold stady, exit anyway
            return;
        }
        if ((len1 < _MAX_DISTANCE_RATIO_FOR_STEADY) || (len2 < _MAX_DISTANCE_RATIO_FOR_STEADY))
        {
            //roc todo, buggy here
            ResetCurrentGesture();
            new (mCurrentGestureEvent) GestureRotateEvent(p1.x(), p1.y(), time, 2, 1.0f);
        }
        else
        {
            float dot = v0.DotProduct(v1);
            if (dot > 0.0f)
            {
                //two tracks in the same direction, move
                p0 = (p1 + p3) * 0.5f;
                ResetCurrentGesture();
                new (mCurrentGestureEvent) GestureMoveEvent(p0.x(), p0.y(), time, 2);
                _inMultiTouchMove = true;
                _lastMultiTouchMoveX = p0.x();
                _lastMultiTouchMoveY = p0.y();
            }
            else
            {
                //pinch
                ResetCurrentGesture();
                new (mCurrentGestureEvent) GesturePinchEvent(p3.x(), p3.y(), time, 2, p1.DistanceSquared(p3) / p0.DistanceSquared(p2));
            }
        }
        /*
        int x, y;
        temp[0]->GetTrackStartingPosition(x, y);
        FastMath::Point3f p0 = {x, y, 0.0f};
        temp[0]->GetTrackEndingPosition(x, y);
        FastMath::Point3f p1 = {x, y, 0.0f};
        temp[1]->GetTrackStartingPosition(x, y);
        FastMath::Point3f p2 = {x, y, 0.0f};
        temp[1]->GetTrackEndingPosition(x, y);
        FastMath::Point3f p3 = {x, y, 0.0f};
        FastMath::Point3f v1 = {p2[0] - p0[0], p2[1] - p0[1], 0.0f};
        FastMath::Point3f v2 = {p3[0] - p1[0], p3[1] - p1[1], 0.0f};
        float VectorAngleCos = 0.0f;
        FastMath::VectorAngleCosine3f(v1, v2, &VectorAngleCos);
        if(VectorAngleCos <= mMaxAngleCosValForRotate)
        {
            // rotate
            float angle;
            FastMath::FastArcCos(VectorAngleCos, &angle);
            ResetCurrentGesture();
            new (mCurrentGestureEvent) GestureRotateEvent(p3[0], p3[1], time, 2, angle);
        }
        else
        {
            float r1;
            FastMath::VectorDistanceSqr3f(p0, p2, &r1);
            float r2;
            FastMath::VectorDistanceSqr3f(p1, p3, &r2);
            // pinch
            ResetCurrentGesture();
            new (mCurrentGestureEvent) GesturePinchEvent(p3[0], p3[1], time, 2, r2/r1);
        }
        */
    }
}

void BaseGestureRecognizer::Update(HexTime currentTime)
{
    unsigned int count = mChangedTouchQueues.Size();
    if (GetActiveTouchQueueCount() > 1)
    {
        OnMultiTouch(currentTime);
    }
    else
    {
        if (_inMultiTouchMove)
        {
            ResetCurrentGesture();
            new (mCurrentGestureEvent) GestureEndMoveEvent(_lastMultiTouchMoveX, _lastMultiTouchMoveY, currentTime, 2);
            _lastMultiTouchMoveX = _lastMultiTouchMoveY = 0;
            _inMultiTouchMove = false;
        }
        unsigned int idx = 0;
        while (!mChangedTouchQueues.IsEmpty() && (idx < count))
        {
            BaseGestureRecognizer::TouchQueueInfomation info = mChangedTouchQueues.Pop();
            idx ++;
            int x, y;
            info.touchQueue->GetTrackStartingPosition(x, y);
            switch (info.curState)
            {
                case STATE_TAP:
                    OnTapState(info, currentTime);
                    break;
                case STATE_SWIPE:
                    OnSwipeState(info, currentTime);
                    break;
                case STATE_MOVE:
                    OnMoveState(info, currentTime);
                    break;
                case STATE_LONG_TAP:
                    OnLongTapState(info, currentTime);
                    break;
                case STATE_DOUBLE_TAP:
                    OnDoubleTapState(info, currentTime);
                    break;
                case STATE_DRAG:
                    OnDragState(info, currentTime);
                    break;
                case STATE_DRAG_MOVE:
                    OnDragMoveState(info, currentTime);
                    break;
                case STATE_MULTI:
                    info.touchQueue->ForceReleaseTouch();
                    break;
                case STATE_NONE:
                    info.touchQueue->ForceReleaseTouch();
                    break;
                default:
                    info.touchQueue->ForceReleaseTouch();
                    break;
            }
        }
    }
}

unsigned int BaseGestureRecognizer::GetActiveTouchQueueCount()
{
    unsigned int count = mChangedTouchQueues.Size();
    unsigned int val = 0;
    for (int i = 0; i < count; ++i)
    {
        if (mChangedTouchQueues[i].touchQueue->IsActived())
        {
            val++;
        }
    }
    return val;
}



