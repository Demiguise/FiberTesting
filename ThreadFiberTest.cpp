// ThreadFiberTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assert.h>
#include <stack>
#include <vector>
#include <atomic>
#include <random>

#include "MemoryManager.h"
#include "Fiber.h"
#include "FiberScheduler.h"
#include "CoreThread.h"

// Notes
//	(Q) Threads, how do we make one?
//	(Q) Fibers, how do we make one without converting FROM a thread? Can we? 
//		# If no, that's crap. We still incur penalties from creating one that we might avoid elsewise.
//	!A We need to convert a thread to fiber in order to actually schedule/deal with the fibers themselves.
//		# The initial thread is the beginning of the daisy chain that we use for fibers.
//	(Q) Thread/Fiber local storage (TLS/FLS), how do we manage the allocations? Do I need it?
//		# See 'The Last of Us' GDC video on the subject for more information?
//	(Q) How do heaps work? 
//	(Q) How should the atomic counters be spawned and decremented?
//		# How does Fiber (F) decrement the atomic counter Fiber (A) is waiting on?
//		# Grab it via an ID? StringCRC? Inject it into the new fiber? Injecting would mean that each of the child fibers/dependanct fibers need to be in roughly the same place?
//		# If I go with injecting, that's okish I suppose. Just means that the structure of the fibers has to be quite crrrrrrrrrrazily clear.
//		# Would an Audio fiber want to be spawned by some other fiber that's waiting on information about the footstep of a character? Is that even a problem I have right now? (No it isn't)
//		# Advantages of the injection would be substantionally easier code to follow, everything would be parallised but somewhat sequential?

struct SFiberData 
{

};

std::random_device rd;
std::mt19937 rng(rd());
std::uniform_int_distribution<int> uni(1000, 5000);
std::uniform_int_distribution<int> uni2(0, 100000);
std::uniform_int_distribution<int> thing(0, 1);

void __stdcall YieldingTestFunc2(LPVOID lpParam)
{
	srand (time(NULL));
	CFiber::Log("Started work on decrementing the yielded counter!");
	int* pInt = (int*)CFiber::GetDataFromFiber();
	int randVal = uni2(rng);
	CFiber::Log("Set int to %i", *(pInt));
}

void __stdcall YieldingTestFunc(LPVOID lpParam)
{
	CFiber::Log("Started work on Yielding!");
	SFiberCounter yieldCounter(2, __FUNCTION__);

	int test1 = 0;
	int test2 = 0;

	CFiber::YieldForCounter(&yieldCounter);

	CFiber::Log("Completed work on Yielding!");
	CFiber::Log("Int values are [%i, %i]", test1, test2);
}

void __stdcall FiberFunc(LPVOID lpParam)
{
	CFiber::Log("Fiber has completed a (%s) priority task!", (const char*)CFiber::GetDataFromFiber());
}

void __stdcall LoggingTest(LPVOID lpParam)
{
	int a = 4;
	CFiber::Log("Testing, Testing. %x", a);
}

void __stdcall WaitTest(LPVOID lpParam)
{
	auto random_int = uni(rng);
	CFiber::Log("Sleeping for %i ms", random_int);
	Sleep(random_int);
	CFiber::Log("Woke up!");
	auto spawn = thing(rng);

}

struct SSortJobData
{
	SSortJobData(int m, int mx, int* arr)
		: iMin(m)
		, iMax(mx)
		, intArray(arr) {}
	int iMin;
	int iMax;
	int* intArray;
};

#define MERGE_MAX_ELEMENTS 10
//#define ENABLE_COPY_SANITY_CHECK
#define ENABLE_SORT_SANITY_CHECK

void __stdcall MergeSortFiberised(LPVOID lpParam)
{
	SJobRequest mergeJob(MergeSortFiberised);

	SSortJobData* m_pData = (SSortJobData*)CFiber::GetDataFromFiber();
	int range = (m_pData->iMax - m_pData->iMin);
	int midPoint = m_pData->iMin + (range / 2);
	if (range > MERGE_MAX_ELEMENTS)
	{
		//Split job into two
		SSortJobData job1 (m_pData->iMin, midPoint, m_pData->intArray);
		SSortJobData job2 (midPoint + 1, m_pData->iMax, m_pData->intArray);
		SFiberCounter mergeCounter(2, __FUNCTION__);
		CFiberScheduler::Schedule(mergeJob, eFP_Normal, &mergeCounter, &job1);
		CFiberScheduler::Schedule(mergeJob, eFP_Normal, &mergeCounter, &job2);

		//Yield until these jobs are done
		CFiber::YieldForCounter(&mergeCounter);
	}

	int i = 0;
	int j = midPoint + 1;
	int k = 1;
	int* arr = m_pData->intArray;
	int* temp = new int[range + 1];
	memcpy(temp, arr + m_pData->iMin, sizeof(int) * (range + 1));

#ifdef ENABLE_COPY_SANITY_CHECK
	CFiber::Log("Sanity Checking Blocks [%u] => [%u]", m_pData->iMin, m_pData->iMax);
	for (int sanity = 0 ; sanity <= range ; ++sanity)
	{
		if (temp[sanity] != arr[m_pData->iMin + sanity])
		{
			DebugBreak();
		}
	}
#endif

	CFiber::Log("Sorting Blocks [%u] => [%u]", m_pData->iMin, m_pData->iMax);
	while ((i <= midPoint) && (j <= m_pData->iMax))
	{
		arr[k++] = (arr[j] < temp[i]) ? arr[j++] : temp[i++];
	}

	while (i <= midPoint)
	{
		arr[k++] = temp[i++];
	}
	delete[] temp;
}

void MergeSortNoFiber(int iMin, int iMax, int* arr)
{
	int range = iMax - iMin;
	int midPoint = iMin + (range / 2);
	if (range > MERGE_MAX_ELEMENTS)
	{
		//Split job into two
		MergeSortNoFiber(iMin, midPoint, arr);
		MergeSortNoFiber(midPoint + 1, iMax, arr);
	}

	int* temp = new int[range + 1];
	memcpy(temp, arr + iMin, sizeof(int) * (range + 1));

#ifdef ENABLE_COPY_SANITY_CHECK
	DebugLog("Sanity Checking Copied Blocks [%u] => [%u]", iMin, iMax);
	for (int sanityI = 0 ; sanityI <= range ; ++ sanityI)
	{
		if (temp[sanityI] != arr[iMin + sanityI])
		{
			DebugBreak();
		}
		else
		{
			DebugLog("Arr[%u] = Temp[%u] (%u)", iMin + sanityI, sanityI, temp[sanityI]);
		}
	}
	DebugLog("Sanity Complete");
#endif

	DebugLog("Sorting Blocks [%u] => [%u]", iMin, iMax);

	for (int debug = iMin ; debug < iMax ; ++debug)
	{
		DebugLog("Arr[%u] (%u)", debug, arr[debug]);
	}

	int h, i, j, k;
	h = iMin;
	i = iMin;
	j = midPoint;

	while ((h <= midPoint) && (j <= iMax))
	{
		if (arr[h] <= arr[j])
		{
			temp[i] = arr[h];
			h++;
		}
		else
		{
			temp[i] = arr[j];
			j++;
		}
		i++;
	}

	if (h > midPoint)
	{
		for (k = j ; k <= iMax ; k++ )
		{
			temp[i] = arr[k];
			i++;
		}
	}
	else
	{
		for (k = h ; k <= midPoint ; k++)
		{
			temp[i] = arr[k];
			i++;
		}
	}

	for (k = iMin ; k <= iMax ; k++) 
	{
		arr[k] = temp[k];
	}

	for (int debug = iMin ; debug < iMax ; ++debug)
	{
		DebugLog("Arr[%u] (%u)", debug, arr[debug]);
	}

#ifdef ENABLE_SORT_SANITY_CHECK
	DebugLog("Sanity Checking Sorted Blocks [%u] => [%u]", iMin, iMax);
	int sanity = iMin;
	int sanityHead = sanity + 1;
	for (; sanityHead < iMax ; ++sanity)
	{
		if (arr[sanity] < arr[sanityHead])
		{
			DebugBreak();
		}
		else
		{
			DebugLog("Arr[%u: %u] < Arr[%u: %u]", sanity, arr[sanity], sanityHead, arr[sanityHead]);
		}
		++sanityHead;
	}
#endif

	delete[] temp;
}

/* left is the index of the leftmost element of the subarray; right is one
 * past the index of the rightmost element */
void merge_helper(int *input, int left, int right, int *scratch)
{
    /* base case: one element */
    if(right == left + 1)
    {
        return;
    }
    else
    {
        int i = 0;
        int length = right - left;
        int midpoint_distance = length/2;
        /* l and r are to the positions in the left and right subarrays */
        int l = left, r = left + midpoint_distance;

        /* sort each subarray */
        merge_helper(input, left, left + midpoint_distance, scratch);
        merge_helper(input, left + midpoint_distance, right, scratch);

        /* merge the arrays together using scratch for temporary storage */ 
        for(i = 0; i < length; i++)
        {
            /* Check to see if any elements remain in the left array; if so,
             * we check if there are any elements left in the right array; if
             * so, we compare them.  Otherwise, we know that the merge must
             * use take the element from the left array */
            if(l < left + midpoint_distance && 
                    (r == right || max(input[l], input[r]) == input[l]))
            {
                scratch[i] = input[l];
                l++;
            }
            else
            {
                scratch[i] = input[r];
                r++;
            }
        }
        /* Copy the sorted subarray back to the input */
        for(i = left; i < right; i++)
        {
            input[i] = scratch[i - left];
        }
    }
}

int mergesort(int *input, int size)
{
    int *scratch = (int *)malloc(size * sizeof(int));
    if(scratch != NULL)
    {
        merge_helper(input, 0, size, scratch);
        free(scratch);
        return 1;
    }
    else
    {
        return 0;
    }
}

class HeapDestructorTest
{
public:
	HeapDestructorTest()
	{
		DebugLog("Constructor called");
		testA = 1;
		testB = 2;
		testC = 4;
		testD = 8;
	}

	~HeapDestructorTest()
	{
		DebugLog("Destructor called");
	}

private:
	int testA;
	int testB;
	int testC;
	int testD;
};

int _tmain(int argc, _TCHAR* argv[])
{
	g_pMemoryManager = new CMemoryManager();
	g_pFiberScheduler = new CFiberScheduler();

	int* arr = CCoreThread::AllocateArray<int>(5, eMT_Tag1);
	arr[0] = 1;
	arr[1] = 1;
	arr[2] = 1;
	arr[3] = 1;
	arr[4] = 1;

	HeapDestructorTest* test = CCoreThread::Allocate<HeapDestructorTest>(eMT_Tag1);
	CCoreThread::Deallocate(test);

	CCoreThread::DeallocateTag(eMT_Tag1);

	int arraySize = 100000;
	int* sortingArray = new int[arraySize];
	std::uniform_int_distribution<int> sortRNG(0);
	for (int i = 0 ; i < arraySize ; ++i)
	{
		sortingArray[i] = sortRNG(rng);
	}

	SJobRequest sortJob(MergeSortFiberised);
	SJobRequest yieldJob(YieldingTestFunc);
	SJobRequest fiberJob(FiberFunc);
	SJobRequest waitJob(WaitTest);

	SSortJobData jobData(0, arraySize - 1, sortingArray);

	DebugLog("Starting Sort.");

	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_High);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);

	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_High);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);

	CCoreThread t1;
	CCoreThread t2;
	CCoreThread t3;
	CCoreThread t4;
	CCoreThread t5;
	CCoreThread t6;

	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_High);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_High);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_High);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(fiberJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_High);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	CFiberScheduler::Schedule(waitJob, eFP_Normal);


	//mergesort(sortingArray, arraySize);

	//MergeSortNoFiber(0, arraySize, sortingArray);

	//while (g_pFiberScheduler->HasJobs())
	//{
	//	DebugLog("Main thread sleeping for 5 seconds.");
	//	Sleep(5000);
	//}

	//DebugLog("Fiber scheduler has no jobs!");

	while (true)
	{
	}

	int i = 0;
	int j = 1;
	for ( ; j < arraySize ; ++i)
	{
		if (sortingArray[i] < sortingArray[j])
		{
			DebugLog("Error. Sorting issue at Elements [%u/%u] = (%u, %u)", i, j, sortingArray[i], sortingArray[j]);
			DebugBreak();
		}
		++j;
	}

	DebugLog("Sorting was successful!");

	return 0;
}
