#ifndef BASE_GESTURE_EVENTS_H_
#define BASE_GESTURE_EVENTS_H_

#include "input/TouchQueue.h"

#define GESTURE_UNKNOWN         0
#define GESTURE_BEGIN_MOVE      1
#define GESTURE_MOVE            2
#define GESTURE_END_MOVE        3
#define GESTURE_TAP             4
#define GESTURE_LONG_TAP        5
#define GESTURE_DOUBLE_CLICK    6
#define GESTURE_SWIPE           7
#define GESTURE_ARC             8
#define GESTURE_DRAG            9
#define GESTURE_DRAG_MOVE       10
#define GESTURE_DROP            11
#define GESTURE_PINCH           12
#define GESTURE_ROTATE          13

//---------------------------- class for basic gesture event ----------------------------
// note: the BaseGestureEvent includes all data, DO NOT introduce ANY DATA in the sub class(es)
// so, we can using new (eventInstance) GestureXXXEvent without any memory-fragment
class BaseGestureEvent
{
public:
    BaseGestureEvent() : mEventX(0), mEventY(0), mEventTime(0), mTouchCount(1), mEventType(GESTURE_UNKNOWN), mFloatParameter(0.0f), mIntParameter(0)
    {}
    
    BaseGestureEvent(int x, int y, HexTime time, unsigned int touchCount) : mEventX(x), mEventY(y), mEventTime(time), mTouchCount(touchCount), mEventType(GESTURE_UNKNOWN), mFloatParameter(0.0f), mIntParameter(0), mIntParameter1(0)
    {}
    
    virtual ~BaseGestureEvent() {}

    inline const __u8 GetEventType() const { return mEventType; }
    
    inline const int GetEventX() const { return mEventX; }
    inline const int GetEventY() const { return mEventY; }
    inline void GetEventCoordinate(int &x, int &y) const { x = mEventX; y = mEventY; }
    
    inline const HexTime GetEventTime() const { return mEventTime; }
    inline const unsigned int GetTouchCount() const { return mTouchCount; }
    
    virtual inline bool IsValid() const { return false; }
protected:
    __u8 mEventType;
    int mEventX;
    int mEventY;
    unsigned int mTouchCount;
    HexTime mEventTime;
    float mFloatParameter;
    __u32 mIntParameter;
    __u32 mIntParameter1;
};

//---------------------------- class for tap gesture event ----------------------------
class GestureTapEvent : public BaseGestureEvent
{
public:
    GestureTapEvent(int x, int y, HexTime time, unsigned int touchCount) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_TAP;
    }

    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for long-tap gesture event ----------------------------
class GestureLongTapEvent : public BaseGestureEvent
{
public:
    GestureLongTapEvent(int x, int y, HexTime time, unsigned int touchCount, HexTime duration) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_LONG_TAP;
        mIntParameter = static_cast<__u32>(duration);
    }
    
    inline const HexTime GetDuration() const { return static_cast<HexTime>(mIntParameter); }

    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for double-click gesture event ----------------------------
class GestureDoubleClickEvent : public BaseGestureEvent
{
public:
    GestureDoubleClickEvent(int x, int y, HexTime time, unsigned int touchCount) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_DOUBLE_CLICK;
    }

    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for swipe gesture event ----------------------------
class GestureSwipeEvent : public BaseGestureEvent
{
public:
    GestureSwipeEvent(int x, int y, HexTime time, unsigned int touchCount, TouchQueue::Direction direction) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_SWIPE;
        mIntParameter = static_cast<__u32>(direction);
    }
    
    inline const TouchQueue::Direction GetDirection() const { return static_cast<TouchQueue::Direction>(mIntParameter); }

    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for arc gesture event ----------------------------
class GestureArcEvent : public BaseGestureEvent
{
public:
    GestureArcEvent(int x, int y, HexTime time, unsigned int touchCount, TouchQueue::ArcShape arcShape, TouchQueue::Direction direction) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_ARC;
        mIntParameter = static_cast<__u32>(arcShape);
        mIntParameter1 = static_cast<__u32>(direction);
    }

    inline const TouchQueue::ArcShape GetArcShape() const { return static_cast<TouchQueue::ArcShape>(mIntParameter); }
    inline const TouchQueue::Direction GetDirection() const { return static_cast<TouchQueue::Direction>(mIntParameter1); }
    
    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for move gesture event ----------------------------
class GestureMoveEvent : public BaseGestureEvent
{
public:
    GestureMoveEvent(int x, int y, HexTime time, unsigned int touchCount) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_MOVE;
    }
    
    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for end move gesture event ----------------------------
class GestureEndMoveEvent : public BaseGestureEvent
{
public:
    GestureEndMoveEvent(int x, int y, HexTime time, unsigned int touchCount) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_END_MOVE;
    }
    
    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for drag gesture event ----------------------------
class GestureDragEvent : public BaseGestureEvent
{
public:
    GestureDragEvent(int x, int y, HexTime time, unsigned int touchCount) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_DRAG;
    }

    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for drag-move gesture event ----------------------------
class GestureDragMoveEvent : public BaseGestureEvent
{
public:
    GestureDragMoveEvent(int x, int y, HexTime time, unsigned int touchCount) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_DRAG_MOVE;
    }

    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for drop gesture event ----------------------------
class GestureDropEvent : public BaseGestureEvent
{
public:
    GestureDropEvent(int x, int y, HexTime time, unsigned int touchCount) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_DROP;
    }

    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for pinch gesture event ----------------------------
class GesturePinchEvent : public BaseGestureEvent
{
public:
    GesturePinchEvent(int x, int y, HexTime time, unsigned int touchCount, float scale) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_PINCH;
        mFloatParameter = scale;
    }
    
    inline const float GetScale() const { return mFloatParameter; }

    virtual inline bool IsValid() const { return true; }
};

//---------------------------- class for rotate gesture event ----------------------------
class GestureRotateEvent : public BaseGestureEvent
{
public:
    GestureRotateEvent(int x, int y, HexTime time, unsigned int touchCount, float angle) : BaseGestureEvent(x, y, time, touchCount)
    {
        mEventType = GESTURE_ROTATE;
        mFloatParameter = angle;
    }
    
    inline const float GetAngle() const { return mFloatParameter; }

    virtual inline bool IsValid() const { return true; }
};

#endif
