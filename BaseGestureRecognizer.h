#ifndef BASE_GESTURE_RECOGNIZER_H_
#define BASE_GESTURE_RECOGNIZER_H_

#include "HexmillEngine.h"
#include "HexTimeCounter.h"
#include "DS_Queue.h"
#include "input/GestureEvents.h"

class TouchQueue;

class BaseGestureRecognizer
{
public:
    BaseGestureRecognizer();
    virtual ~BaseGestureRecognizer();

    inline const char *GetId() const { return mId.c_str(); }
    
    static BaseGestureRecognizer *Create(const char *id);
    
    virtual void Initialize();
    
    virtual void ResetCurrentGesture();
    inline BaseGestureEvent *GetCurrentGestureEvent() const { return mCurrentGestureEvent->IsValid() ? mCurrentGestureEvent : 0; }
    
    virtual void Update(HexTime currentTime);
    
protected:
    std::string mId;
    BaseGestureEvent *mCurrentGestureEvent;
    
    HexTime mMaxIntervalOfDoubleClick;
    HexTime mMinSteadyTimeForDrag;
    HexTime mMinTimeForLongTap;
    HexTime mMaxSwipeDuration;
    float mMinSpeedForSwipe;
    float mMaxAngleCosValForRotate;
    int mMinXDistanceForArc;
    float mMinYChangePersentForArc;
    int mMaxSteadyMoveDistanceX;
    int mMaxSteadyMoveDistanceY;

private:
    void InitializeDefaultParameters();

protected:
    
    enum TouchState
    {
        STATE_TAP         = 0,
        STATE_BEGIN_MOVE  = 1,
        STATE_MOVE        = 2,
        STATE_END_MOVE    = 3,
        STATE_SWIPE       = 4,
        STATE_LONG_TAP    = 5,
        STATE_DOUBLE_TAP  = 6,
        STATE_DRAG        = 7,
        STATE_DRAG_MOVE   = 8,
        STATE_MULTI       = 9,
        STATE_NONE        = 10,
    };
    
    struct TouchQueueInfomation
    {
        TouchQueueInfomation() : touchQueue(0), releaseTime(0), lastChangingMode(0), repeatTimes(0) {}
        TouchQueueInfomation(TouchQueue *queue, int changingMode, HexTime time) : touchQueue(queue), releaseTime(time), lastChangingMode(changingMode), repeatTimes(0),
                curState(STATE_NONE)
        {
            if (touchQueue->IsActived())
                curState = STATE_TAP;
        }
        
        inline bool IsEmpty() const { return !touchQueue && !releaseTime; }
        
        TouchQueue *touchQueue;
        HexTime releaseTime;
        int lastChangingMode;
        int repeatTimes;
        TouchState curState;
    };
    
    TouchQueueInfomation &FindQueueInfomation(TouchQueue *queue);
    void TryRemoveTouchQueueInfomation(TouchQueue *queue);
    
    DataStructures::Queue<TouchQueueInfomation> mChangedTouchQueues;
    unsigned int GetActiveTouchQueueCount();
    
    
public:
    bool TryAddTouchQueueChanging(TouchQueue *queue, int changingMode, HexTime time);

protected:
    virtual void OnTapState(TouchQueueInfomation &info, HexTime time);
    virtual void OnMoveState(TouchQueueInfomation &info, HexTime time);
    virtual void OnSwipeState(TouchQueueInfomation &info, HexTime time);
    virtual void OnLongTapState(TouchQueueInfomation &info, HexTime time);
    virtual void OnDoubleTapState(TouchQueueInfomation &info, HexTime time);
    virtual void OnDragState(TouchQueueInfomation &info, HexTime time);
    virtual void OnDragMoveState(TouchQueueInfomation &info, HexTime time);
    virtual void OnMultiTouch(HexTime time);
    
};

#endif
