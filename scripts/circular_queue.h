#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

// ---------------------------------------------------------------------
//  CircularQueue -- a fixed-size pool of the guard's projectiles.
//
//  Last Bell: Campus Rush -- Data Structures (CSE 2103)
//
//  Written by hand. No STL: no std::queue, no std::vector, no
//  std::deque. The storage is one plain array that never grows.
//
//  WHY A CIRCULAR QUEUE HERE:
//  The boss fires continuously for the whole fight. A linked queue
//  would call new/delete on every single shot -- hundreds of
//  allocations during one boss fight, each one a chance to fragment
//  the heap or stall a frame. A circular queue allocates its array
//  once and then reuses the same slots forever: front and rear chase
//  each other around the ring, wrapping with % CAPACITY.
//
//  That is the opposite trade from the dialogue queue, which is a
//  LINKED queue precisely because the number of lines is not known
//  ahead of time and is read once rather than every frame.
//
//  OVERFLOW AND UNDERFLOW
//  Because the array is fixed, both states are real and both are
//  handled rather than ignored:
//      overflow  -- enqueue() on a full ring returns false, and the
//                   guard simply skips that shot. The pool becomes a
//                   natural rate limiter.
//      underflow -- dequeue() on an empty ring returns false instead
//                   of reading a slot that holds nothing.
//
//  LAZY REMOVAL
//  A queue can only remove from the front, but a shot in the middle
//  can be destroyed early by hitting the player. Those are marked
//  alive = false and reclaimed when they reach the front. This keeps
//  the structure a true FIFO queue rather than a list with holes.
//
//  COMPLEXITY
//      enqueue   O(1)
//      dequeue   O(1)
//      peekAt    O(1)
//      isFull    O(1)
//      isEmpty   O(1)
//      size      O(1)
//      clear     O(1)
//  Space: O(CAPACITY), fixed at compile time. No allocation, ever.
// ---------------------------------------------------------------------

const int POOL_CAPACITY = 8;


struct Shot
{
	double x, y;        // world position
	double vx, vy;      // velocity, pixels per second
	double age;         // seconds since it was fired
	bool   alive;       // false once it has hit something or timed out

	Shot()
	{
		x = 0.0; y = 0.0;
		vx = 0.0; vy = 0.0;
		age = 0.0;
		alive = false;
	}
};


class CircularQueue
{
private:
	Shot slots[POOL_CAPACITY];
	int  front;     // index of the oldest live shot
	int  count;     // how many are live right now

public:
	CircularQueue()
	{
		front = 0;
		count = 0;
	}

	bool isEmpty() const  { return count == 0; }
	bool isFull()  const  { return count == POOL_CAPACITY; }
	int  size()    const  { return count; }
	int  capacity() const { return POOL_CAPACITY; }


	// O(1). Put a new shot at the rear. Returns false on OVERFLOW,
	// which the guard treats as "skip this shot".
	bool enqueue(const Shot& s)
	{
		if (isFull()) return false;

		int rear = (front + count) % POOL_CAPACITY;
		slots[rear] = s;
		count++;
		return true;
	}


	// O(1). Take the oldest shot off the front. Returns false on
	// UNDERFLOW rather than reading a slot that holds nothing.
	bool dequeue(Shot* out)
	{
		if (isEmpty()) return false;

		if (out != nullptr) *out = slots[front];

		front = (front + 1) % POOL_CAPACITY;
		count--;
		return true;
	}


	// O(1). The i-th live shot, 0 being the oldest. Returns nullptr
	// if i is outside the live window.
	Shot* peekAt(int i)
	{
		if (i < 0 || i >= count) return nullptr;
		return &slots[(front + i) % POOL_CAPACITY];
	}


	// O(1). Which array slot the i-th live shot occupies. The guard
	// uses this to pick which pooled sprite draws it.
	int slotOf(int i) const
	{
		if (i < 0 || i >= count) return -1;
		return (front + i) % POOL_CAPACITY;
	}


	// O(1). Drop everything. Nothing is freed because nothing was
	// ever allocated.
	void clear()
	{
		front = 0;
		count = 0;
		for (int i = 0; i < POOL_CAPACITY; i++)
			slots[i].alive = false;
	}
};

#endif
