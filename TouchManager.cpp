#include "input/TouchManager.h"
#include "input/BaseGestureRecognizer.h"

//--------------------------------------------------- TouchManager --------------------------------------------------
TouchManager::TouchManager(unsigned int maxCount) : mMaxTouchQueueCount(maxCount), mGestureRecognizer(0)
{
    assert(mMaxTouchQueueCount >= 1);
    mTouchQueues = (TouchQueue **)malloc(sizeof(TouchQueue *) * mMaxTouchQueueCount);
    for (unsigned int i=0; i<mMaxTouchQueueCount; i++)
        mTouchQueues[i] = new TouchQueue(i);
}

TouchManager::~TouchManager()
{
    mTimer.StopTimer();
    Clear();
}
    
void TouchManager::Clear()
{
    for (unsigned int i=0; i<mMaxTouchQueueCount; i++)
        delete mTouchQueues[i];
    free(mTouchQueues);
    mTouchQueues = 0;
    mActivedTouchQueue.Clear();
    mGestureRecognizer = 0;
    mGestureListeners.clear();
}
    
void TouchManager::AddTouch(int x, int y, unsigned int touchIndex)
{
    if (touchIndex >= mMaxTouchQueueCount)
        return;
    mTouchQueues[touchIndex]->AddTouch(x, y, mTimer.GetTimeSlapped());
    if (mGestureRecognizer)
        mGestureRecognizer->TryAddTouchQueueChanging(mTouchQueues[touchIndex], 1, mTimer.GetTimeSlapped());
}

void TouchManager::TouchMove(int x, int y, unsigned int touchIndex)
{
    if (touchIndex >= mMaxTouchQueueCount)
        return;
    mTouchQueues[touchIndex]->TouchMove(x, y, mTimer.GetTimeSlapped());
    if (mGestureRecognizer)
        mGestureRecognizer->TryAddTouchQueueChanging(mTouchQueues[touchIndex], 2, mTimer.GetTimeSlapped());
}

void TouchManager::ReleaseTouch(int x, int y, unsigned int touchIndex)
{
    if (touchIndex >= mMaxTouchQueueCount)
        return;
    mTouchQueues[touchIndex]->ReleaseTouch(x, y, mTimer.GetTimeSlapped());
    if (mGestureRecognizer)
        mGestureRecognizer->TryAddTouchQueueChanging(mTouchQueues[touchIndex], 3, mTimer.GetTimeSlapped());
}
    
void TouchManager::Update()
{
    if (mTimer.IsTimerStopped())
        return;
    mGestureRecognizer->Update(mTimer.GetTimeSlapped());
    BaseGestureEvent *event = mGestureRecognizer->GetCurrentGestureEvent();
    if (!event)
        return;
    for (unsigned int i=0; i<mGestureListeners.size(); i++)
    {
        mGestureListeners[i]->GestureEvent(event);
    }
    mGestureRecognizer->ResetCurrentGesture();
}

void TouchManager::RegisterGestureListener(TouchManager::GestureListener *listener)
{
    for (unsigned int i=0; i<mGestureListeners.size(); i++)
    {
        if (listener == mGestureListeners[i])
            return;
    }
    mGestureListeners.push_back(listener);
    TryActiveTouchManager();
}

void TouchManager::UnRegisterGestureListener(TouchManager::GestureListener *listener)
{
    for (std::vector<TouchManager::GestureListener *>::iterator it = mGestureListeners.begin(); it != mGestureListeners.end(); it++)
    {
        if (*it == listener)
        {
            mGestureListeners.erase(it);
            break;
        }
    }
    TryActiveTouchManager();
}

void TouchManager::RegisterGestureRecognizer(const char *recognizerName)
{
    //delete the old one
    if (mGestureRecognizer)
    {
        if (strcmp(recognizerName, mGestureRecognizer->GetId()) == 0)
            return;
        SAFE_DELETE(mGestureRecognizer);
    }
    //try create a new one
    mGestureRecognizer = BaseGestureRecognizer::Create(recognizerName);
    mGestureRecognizer->Initialize();
    
    TryActiveTouchManager();
}

void TouchManager::TryActiveTouchManager()
{
    bool lastEnabled = !mTimer.IsTimerStopped();
    bool currentEnabled = IsEnabled();
    if (lastEnabled == currentEnabled)
        return;
    if (currentEnabled)
        mTimer.StartTimer();
    else
        mTimer.StopTimer();
}
