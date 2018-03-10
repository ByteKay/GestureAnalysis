#ifndef TOUCH_QUEUE_H_
#define TOUCH_QUEUE_H_

#include "HexmillEngine.h"
#include "DS_Queue.h"

using namespace HexmillEngine;

struct TouchPoint
{
    TouchPoint() : point(FastMath::Vector2::Zero()), time(0) {}
    TouchPoint(const FastMath::Vector2 &p, HexTime t) : point(p), time(t) {}
    
    inline bool IsValid() { return time != 0; }
    
    FastMath::Vector2 point;
    HexTime time;
};

class TouchQueue
{
public:
    typedef DataStructures::Queue<TouchPoint> TouchTrack;
    
    enum ArcShape
    {
        ARC_NONE    = 0,
        ARC_UP      = 1,
        ARC_DOWN    = 2,
    };
    
    enum Direction
    {
        DIR_NONE            = 0x00000000,
        DIR_TOP             = 0x00000001,
        DIR_TOP_RIGHT       = 0x00000002,
        DIR_RIGHT           = 0x00000004,
        DIR_BOTTOM_RIGHT    = 0x00000008,
        DIR_BOTTOM          = 0x00000010,
        DIR_BOTTOM_LEFT     = 0x00000020,
        DIR_LEFT            = 0x00000040,
        DIR_TOP_LEFT        = 0x00000080,
    };
public:
    TouchQueue(unsigned int index);
    virtual ~TouchQueue();
    
    virtual void Clear();
    
    virtual inline bool IsActived() const { return mActived; }
    virtual inline unsigned int GetTouchIndex() const { return mTouchIndex; }
    
    //try add touch point in track queue, if the queue is not actived, the input will be ignored
    virtual void AddTouch(int x, int y, HexTime time);
    virtual void TouchMove(int x, int y, HexTime time);
    virtual void ReleaseTouch(int x, int y, HexTime time);

    //try deactive the queue, after these calls, the input should be ignored, even the touch event still triggered
    virtual void ForceReleaseTouch(int x, int y, HexTime time);
    virtual void ForceReleaseTouch();
    
    HexTime GetDuration();
    HexTime GetCurrentDuration(HexTime current);
    bool GetMovingSpeeds(float &maxSpeed, float &avgSpeed);
    unsigned int GetTouchPointCount() const;
    bool IsArcTrack(int minXDistance, float minYChangePersent, TouchQueue::ArcShape &arcType, Direction &direction);
    void GetAbsMaxMovingDistance(int &x, int &y);
    void GetTrackStartingPosition(int &x, int &y);
    void GetTrackEndingPosition(int &x, int &y);
protected:
    TouchTrack mTouchTrack;
    bool mActived;
    unsigned int mTouchIndex;
};


#endif
