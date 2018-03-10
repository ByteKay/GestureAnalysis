#ifndef TOUCH_INPUT_H_
#define TOUCH_INPUT_H_

#include "HexmillEngine.h"
#include "HexTimeCounter.h"
#include "DS_Queue.h"

using namespace HexmillEngine;

class TouchInput
{
public:
    class TouchInfo
    {
    public:
        typedef DataStructures::Queue<FastMath::Vector2> TouchTrack;
    public:
        TouchInfo(unsigned int index);
        virtual ~TouchInfo();
        
        virtual void Clear();

        inline const TouchTrack *GetTouchTrack() const { return &mTouchTrack; }
        inline const unsigned int GetTouchIndex() const { return mTouchIndex; }
        
        bool IsTouched();
        void TryTouch(int x, int y);
        void TryTouchMove(int x, int y);
        void TryReleaseTouch(int x, int y);
        bool IsLongTouch();
 
        const FastMath::Vector2 &GetFirstTouchPoint() const;
        const FastMath::Vector2 &GetLastTouchPoint() const;
        inline const FastMath::Vector2 &GetLastMoveDirection() const { return mLastMoveDirection; }
    private:
        
        const unsigned int mTouchIndex;
        HexTimeCounter mTouchTimer;
        
        TouchTrack mTouchTrack;
        FastMath::Vector2 mLastMoveDirection;
    };
    
public:
    TouchInput(unsigned int maxTouchCount);
    virtual ~TouchInput();

    void ProcessTouchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);
    
    unsigned int GetCurrentTouchCount();

    inline unsigned int GetMaxTouchCount() { return mMaxTouchCount; }
    virtual void SetMaxTouchCount(unsigned int count);
    inline TouchInfo *operator [] (unsigned int index) { assert(index < mMaxTouchCount); return mTouchInfoes[index]; }
    
public:
    enum Touch2Gestures
    {
        GESTURE_NONE            = 0,
        GESTURE_LONG_TAP        = 1,
        GESTURE_PINCH           = 2,
        GESTURE_MOVE            = 3,
    };
    
    Touch2Gestures TryGetTouch2Gesture(FastMath::Vector2 &value);
protected:
    unsigned int GetherTouchMovings(TouchInfo **touchInfos, FastMath::Vector2 *movings);
    
    void Initialize(unsigned int maxTouchCount);
    virtual void Clear();
        
    TouchInfo **mTouchInfoes;
    unsigned int mMaxTouchCount;
};

#endif
