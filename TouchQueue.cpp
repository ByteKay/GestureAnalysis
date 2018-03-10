#include "input/TouchQueue.h"

//--------------------------------------------------- TouchQueue --------------------------------------------------
TouchQueue::TouchQueue(unsigned int index) : mActived(false), mTouchIndex(index)
{
    mTouchTrack.ClearAndForceAllocation(32);
}

TouchQueue::~TouchQueue()
{
    mTouchTrack.Clear();
}

void TouchQueue::Clear()
{
    mActived = false;
    while (!mTouchTrack.IsEmpty())
        mTouchTrack.Pop();
}

void TouchQueue::AddTouch(int x, int y, HexTime time)
{
    Clear();
    mActived = true;
    mTouchTrack.Push(TouchPoint(FastMath::Vector2((float)x, (float)y), time));
}

void TouchQueue::TouchMove(int x, int y, HexTime time)
{
    if (mActived)
        mTouchTrack.Push(TouchPoint(FastMath::Vector2((float)x, (float)y), time));
}

void TouchQueue::ReleaseTouch(int x, int y, HexTime time)
{
    if (!mActived)
        return;
    mTouchTrack.Push(TouchPoint(FastMath::Vector2((float)x, (float)y), time));
    mActived = false;
}

void TouchQueue::ForceReleaseTouch(int x, int y, HexTime time)
{
    ReleaseTouch(x, y, time);
}

void TouchQueue::ForceReleaseTouch()
{
    if (!mActived)
        return;
    if (mTouchTrack.Size() == 1)
    {
        TouchPoint &p = mTouchTrack[0];
        ReleaseTouch((int)p.point.x(), (int)p.point.y(), p.time + 200);
    }
    mActived = false;
}

HexTime TouchQueue::GetDuration()
{
    if (mTouchTrack.Size() < 2)
        return 0;
    return mTouchTrack.PeekTail().time - mTouchTrack.Peek().time;
}

HexTime TouchQueue::GetCurrentDuration(HexTime current)
{
    if (mTouchTrack.IsEmpty())
        return 0;
    return current - mTouchTrack.Peek().time;
}

bool TouchQueue::GetMovingSpeeds(float &maxSpeed, float &avgSpeed)
{
    if (mTouchTrack.Size() < 2)
        return false;
    maxSpeed = 0.0f;
    bool res = false;
    for (unsigned int i=1; i<mTouchTrack.Size(); i++)
    {
        TouchPoint &p0 = mTouchTrack[i - 1];
        TouchPoint &p1 = mTouchTrack[i];
        int dt = p1.time - p0.time;
        if (dt == 0)
            continue;
        float speed = (p1.point - p0.point).Length() * 1000.0f / (float)dt;
        if (speed > maxSpeed)
            maxSpeed = speed;
        res = true;
    }
    avgSpeed = 0.0f;
    if (res)
    {
        TouchPoint p0 = mTouchTrack.Peek();
        TouchPoint p1 = mTouchTrack.PeekTail();
        int dt = p1.time - p0.time;
        if (dt != 0)
            avgSpeed = (p1.point - p0.point).Length() * 1000.0f / (float)dt;
    }
    return true;
}

unsigned int TouchQueue::GetTouchPointCount() const
{
    return mTouchTrack.Size();
}

bool TouchQueue::IsArcTrack(int minXDistance, float minYChangePersent, TouchQueue::ArcShape &arcType, Direction &direction)
{
    arcType = TouchQueue::ARC_NONE;
    direction = TouchQueue::DIR_NONE;
    
    unsigned int trackCount = mTouchTrack.Size();

    TouchPoint &p0 = mTouchTrack[0];
    TouchPoint &p1 = mTouchTrack[trackCount - 1];

    if (trackCount >= 4)
    {
        int dx = abs((int)p1.point.x() - (int)p0.point.x());
        if (dx > minXDistance)
        {
            //identity matrix
            FastMath::Matrix4x4f m;
            //build rotation matrix
            static FastMath::Point3f _x_vector = {1.0f, 0.0f, 0.0f};
            FastMath::Point3f vec = {p1.point.x() - p0.point.x(), p1.point.y() - p0.point.y(), 0.0f};
            FastMath::NormalizeVector3f(vec);
            FastMath::CreateRotationMatrix4f(_x_vector, vec, m);
            
            std::vector<float> yArray;
            unsigned int topYIndex = 0;
            float maxYDist = -1e20f;
            for (unsigned int i=1; i<trackCount - 1; i++)
            {
                TouchPoint &p = mTouchTrack[i];
                FastMath::Point3f pt = {p.point.x() - p0.point.x(), p.point.y()-p0.point.y(), 0.0f};
                FastMath::Point3f myPt;
                FastMath::PositionTransform3f(pt, m, myPt);
                yArray.push_back(myPt[1]);
                float yDist = myPt[1] - vec[1];
                if (fabs(yDist) > maxYDist)
                {
                    maxYDist = fabs(yDist);
                    topYIndex = i;
                }
            }
            
            float dist = (p1.point - p0.point).Length();
            float yChangePersent = maxYDist / dist;
            if (yChangePersent >= minYChangePersent)
            {
                if (yArray[topYIndex] > 0)
                    arcType = TouchQueue::ARC_UP;
                else
                    arcType = TouchQueue::ARC_DOWN;

                if (p1.point.x() > p0.point.x())
                    direction = TouchQueue::DIR_RIGHT;
                else
                    direction = TouchQueue::DIR_LEFT;
                
                return true;
            }
        }
    }
    //not a movement in curve, determin the direction
    static const FastMath::Point3f _const_directions[8] =
    {
        {0.0f, -1.0f, 0.0f},                //top
        {0.707107f, -0.707107f, 0.0f},      //right-top
        {1.0f, 0.0f, 0.0f},                 //right
        {0.707107f, 0.707107f, 0.0f},       //right-bottom
        {0.0f, 1.0f, 0.0f},                 //bottom
        {-0.707107f, 0.707107f, 0.0f},      //left-bottom
        {-1.0f, 0.0f, 0.0f},                //left
        {-0.707107f, -0.707107f, 0.0f}      //left-top
    };
    
    FastMath::Point3f vec = {p1.point.x() - p0.point.x(), p1.point.y() - p0.point.y(), 0.0f};
    FastMath::NormalizeVector3f(vec);
    
    unsigned int closestIndex = 100;
    float maxDot = -1e20f;
    for (unsigned int i=0; i<8; i++)
    {
        float dot = FastMath::VectorDotProduct3f(vec, _const_directions[i]);
        if (dot > maxDot)
        {
            maxDot = dot;
            closestIndex = i;
        }
    }
    switch (closestIndex)
    {
        case 0:
            direction = TouchQueue::DIR_TOP;
            break;
        case 1:
            direction = TouchQueue::DIR_TOP_RIGHT;
            break;
        case 2:
            direction = TouchQueue::DIR_RIGHT;
            break;
        case 3:
            direction = TouchQueue::DIR_BOTTOM_RIGHT;
            break;
        case 4:
            direction = TouchQueue::DIR_BOTTOM;
            break;
        case 5:
            direction = TouchQueue::DIR_BOTTOM_LEFT;
            break;
        case 6:
            direction = TouchQueue::DIR_LEFT;
            break;
        case 7:
            direction = TouchQueue::DIR_TOP_LEFT;
            break;
        default:
            direction = TouchQueue::DIR_NONE;
            assert(false);
            break;
    }
    return false;
}

void TouchQueue::GetAbsMaxMovingDistance(int &x, int &y)
{
    if (mTouchTrack.Size() < 2)
    {
        x = y = 0;
    }
    else
    {
        x = y = -1e20;
        for (unsigned int i=1; i<mTouchTrack.Size(); i++)
        {
            TouchPoint &p1 = mTouchTrack[i];
            TouchPoint &p0 = mTouchTrack[i - 1];
            int tx = fabs(p1.point.x() - p0.point.x());
            int ty = fabs(p1.point.y() - p0.point.y());
            if (x < tx)
                x = tx;
            if (y < ty)
                y = ty;
        }
    }
}

void TouchQueue::GetTrackStartingPosition(int &x, int &y)
{
    if (mTouchTrack.IsEmpty())
    {
        x = y = 0;
    }
    else
    {
        TouchPoint &p = mTouchTrack[0];
        x = p.point.x();
        y = p.point.y();
    }
}

void TouchQueue::GetTrackEndingPosition(int &x, int &y)
{
    if (mTouchTrack.IsEmpty())
    {
        x = y = 0;
    }
    else
    {
        TouchPoint &p = mTouchTrack[mTouchTrack.Size() - 1];
        x = p.point.x();
        y = p.point.y();
    }
}
