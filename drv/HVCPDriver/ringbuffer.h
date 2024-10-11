#pragma once

#include "internal.h"

class CRingBuffer
{
private:

    SIZE_T m_Size;

    PBYTE m_Base;

    PBYTE m_End;

    PBYTE m_Head;

    PBYTE m_Tail;

private:

    void GetAvailableSpace(__out SIZE_T *AvailableSpace)
    {
        WUDF_TEST_DRIVER_ASSERT(AvailableSpace);

        PBYTE headSnapshot = NULL;
        PBYTE tailSnapshot = NULL;
        PBYTE tailPlusOne = NULL;

        //
        // Take a snapshot of the head and tail pointers. We will compute the
        // available space based on this snapshot. This is safe to do in a
        // single-producer, single-consumer model, because -
        //     * A producer will call GetAvailableSpace() to determine whether
        //       there is enough space to write the data it is trying to write.
        //       The only other thread that could modify the amount of space
        //       available is the consumer thread, which can only increase the
        //       amount of space available. Hence it is safe for the producer
        //       to write based on this snapshot.
        //     * A consumer thread will call GetAvailableSpace() to determine
        //       whether there is enough data in the buffer for it to read.
        //       (Available data = Buffer size - Available space). The only
        //       other thread that could modify the amount of space available
        //       is the producer thread, which can only decrease the amount of
        //       space available (thereby increasing the amount of data
        //       available. Hence it is safe for the consumer to read based on
        //       this snapshot.
        //
        headSnapshot = m_Head;
        tailSnapshot = m_Tail;

        //
        // In order to distinguish between a full buffer and an empty buffer,
        // we always leave the last byte of the buffer unused. So, an empty
        // buffer is denoted by -
        //      tail == head
        // ... and a full buffer is denoted by -
        //      (tail+1) == head
        //
        tailPlusOne = ((tailSnapshot+1) == m_End) ? m_Base : (tailSnapshot+1);

        if (tailPlusOne == headSnapshot)
        {
            //
            // Buffer full
            //
            *AvailableSpace = 0;
        }
        else if (tailSnapshot == headSnapshot)
        {
            //
            // Buffer empty
            // The -1 in the computation below is to account for the fact that
            // we always leave the last byte of the ring buffer unused in order
            // to distinguish between an empty buffer and a full buffer.
            //
            *AvailableSpace = m_Size - 1;
        }
        else
        {
            if (tailSnapshot > headSnapshot)
            {
                //
                // Data has not wrapped around the end of the buffer
                // The -1 in the computation below is to account for the fact
                // that we always leave the last byte of the ring buffer unused
                // in order to distinguish between an empty buffer and a full
                // buffer.
                //
                *AvailableSpace = m_Size - (tailSnapshot - headSnapshot) - 1;
            }
            else
            {
                //
                // Data has wrapped around the end of the buffer
                // The -1 in the computation below is to account for the fact
                // that we always leave the last byte of the ring buffer unused
                // in order to distinguish between an empty buffer and a full
                // buffer.
                //
                *AvailableSpace = (headSnapshot - tailSnapshot) - 1;
            }
        }

        return;
    }

public:

    CRingBuffer(VOID);

    ~CRingBuffer(VOID);

    HRESULT Initialize(__in SIZE_T BufferSize);

    HRESULT Write(__in_bcount(DataSize) PBYTE Data,__in SIZE_T DataSize);

    HRESULT Read(__out_bcount(DataSize) PBYTE Data,__in SIZE_T DataSize,__out SIZE_T *BytesCopied);

    void GetAvailableData(__out SIZE_T *AvailableData)
    {
        SIZE_T availableSpace;

        WUDF_TEST_DRIVER_ASSERT(AvailableData);

        GetAvailableSpace(&availableSpace);

        //
        // The -1 in the arithmetic below accounts for the fact that we always
        // keep 1 byte of the ring buffer unused in order to distinguish
        // between a full buffer and an empty buffer.
        //
        *AvailableData = m_Size - availableSpace - 1;

        return;
    }
};

