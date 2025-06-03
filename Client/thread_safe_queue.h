//#pragma once
//#include <list>
//#include <thread>
//#include <vector>
//
//template <typename C>
//class ThreadSafeQueue : protected std::list<C> {
//public:
//    ThreadSafeQueue(int nMaxCount)
//    {
//        m_bOverflow = false;
//
//        m_hSemaphore = ::CreateSemaphore(
//            NULL,		// no security attributes
//            0,			// initial count
//            nMaxCount,	// max count
//            NULL);		// anonymous
//    }
//
//    ~ThreadSafeQueue()
//    {
//        ::CloseHandle(m_hSemaphore);
//        m_hSemaphore = NULL;
//    }
//
//    void push(C& c)
//    {
//        CComCritSecLock<CComAutoCriticalSection> lock(m_Crit, true);
//        this->push_back(c);
//        lock.Unlock();
//
//        if (!::ReleaseSemaphore(m_hSemaphore, 1, NULL))
//        {
//            // If the semaphore is full, then take back the entry.
//            lock.Lock();
//            this->pop_back();
//            if (GetLastError() == ERROR_TOO_MANY_POSTS)
//            {
//                m_bOverflow = true;
//            }
//        }
//    }
//
//    bool pop(C& c)
//    {
//        CComCritSecLock<CComAutoCriticalSection> lock(m_Crit, true);
//
//        // If the user calls pop() more than once after the
//        // semaphore is signaled, then the semaphore count will
//        // get out of sync.  We fix that when the queue empties.
//        if (this->empty())
//        {
//            while (::WaitForSingleObject(m_hSemaphore, 0) != WAIT_TIMEOUT)
//                1;
//            return false;
//        }
//
//        c = this->front();
//        this->pop_front();
//
//        return true;
//    }
//
//    // If overflow, use this to clear the queue.
//    void clear()
//    {
//        CComCritSecLock<CComAutoCriticalSection> lock(m_Crit, true);
//
//        for (DWORD i = 0; i < this->size(); i++)
//        {
//            // semaphore count decrease size, means reset semaphore count
//            WaitForSingleObject(m_hSemaphore, 0);
//        }
//
//        __super::clear();
//
//        m_bOverflow = false;
//    }
//
//    bool overflow()
//    {
//        return m_bOverflow;
//    }
//
//    HANDLE GetWaitHandle() { return m_hSemaphore; }
//
//protected:
//    bool m_bOverflow;
//    HANDLE m_hSemaphore;
//    CComAutoCriticalSection m_Crit;
//};
