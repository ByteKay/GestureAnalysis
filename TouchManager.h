#ifndef TOUCH_MANAGER_H_
#define TOUCH_MANAGER_H_

#include "input/TouchQueue.h"
#include "HexTimeCounter.h"
#include <vector.h>

using namespace HexmillEngine;

class BaseGestureEvent;
class BaseGestureRecognizer;

class TouchManager
{
public:
    class GestureListener
    {
    public:
        GestureListener() {}
        
        virtual ~GestureListener() { }
        
        virtual void GestureEvent(BaseGestureEvent *event) = 0;
    };
public:
    TouchManager(unsigned int maxCount = 10);
    virtual ~TouchManager();
    
    virtual void AddTouch(int x, int y, unsigned int touchIndex);
    virtual void TouchMove(int x, int y, unsigned int touchIndex);
    virtual void ReleaseTouch(int x, int y, unsigned int touchIndex);

    virtual void Update();
    
    inline bool IsEnabled() const { return mGestureRecognizer && (mGestureListeners.size() > 0); }

    void RegisterGestureListener(TouchManager::GestureListener *listener);
    void UnRegisterGestureListener(TouchManager::GestureListener *listener);
    
    void RegisterGestureRecognizer(const char *recognizerName);
protected:
    virtual void Clear();
    void TryActiveTouchManager();
    
    HexTimeCounter mTimer;
    
    TouchQueue **mTouchQueues;
    unsigned int mMaxTouchQueueCount;
    
    DataStructures::Queue<TouchQueue *> mActivedTouchQueue;
    
    std::vector<TouchManager::GestureListener *> mGestureListeners;

    BaseGestureRecognizer *mGestureRecognizer;
};


#endif
