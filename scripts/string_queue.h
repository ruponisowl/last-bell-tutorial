#ifndef STRING_QUEUE_H
#define STRING_QUEUE_H

// ---------------------------------------------------------------------
//  StringQueue -- a FIFO queue of text lines.
//
//  Last Bell: Campus Rush -- Data Structures (CSE 2103)
//
//  Written by hand with linked nodes. No STL: no std::queue, no
//  std::string, no std::vector.
//
//  WHY A LINKED QUEUE HERE:
//  Head and tail pointers give O(1) enqueue and O(1) dequeue. A plain
//  array-backed queue would need O(n) shifting on every dequeue, and the
//  number of dialogue lines is not fixed ahead of time, so paying for
//  dynamic nodes is the right trade. (The projectile pool needs the
//  opposite trade -- fixed size, no allocation at all -- which is why
//  that one is a circular queue instead.)
//
//  COMPLEXITY
//      enqueue   O(1)
//      dequeue   O(1)
//      peek      O(1)
//      isEmpty   O(1)
//      size      O(1)
//      clear     O(n)
//  Space: O(n), one node per queued line.
// ---------------------------------------------------------------------

const int MAX_LINE = 128;


class QueueNode
{
public:
	char       text[MAX_LINE];
	QueueNode* next;

	QueueNode(const char* source)
	{
		next = nullptr;
		copyText(source, text);
	}

	// Hand-written so this header depends on nothing at all.
	static void copyText(const char* from, char* to)
	{
		int i = 0;
		while (from[i] != '\0' && i < MAX_LINE - 1)
		{
			to[i] = from[i];
			i++;
		}
		to[i] = '\0';
	}
};


class StringQueue
{
private:
	QueueNode* head;    // front -- the next line to come out
	QueueNode* tail;    // back  -- where new lines go in
	int        count;

public:
	StringQueue()
	{
		head  = nullptr;
		tail  = nullptr;
		count = 0;
	}

	~StringQueue() { clear(); }

	// Rule of Three: this class owns raw pointers, so a copy would
	// double-free on destruction. Forbid copying rather than be
	// silently wrong.
	StringQueue(const StringQueue&) = delete;
	StringQueue& operator=(const StringQueue&) = delete;


	// O(1). Append at the tail.
	bool enqueue(const char* line)
	{
		if (line == nullptr) return false;

		QueueNode* node = new QueueNode(line);

		if (tail == nullptr)        // queue was empty
		{
			head = node;
			tail = node;
		}
		else
		{
			tail->next = node;
			tail = node;
		}

		count++;
		return true;
	}


	// O(1). Remove from the head. Returns false on underflow.
	// 'out' must point to a buffer of at least MAX_LINE chars,
	// or be nullptr if the caller does not want the text.
	bool dequeue(char* out)
	{
		if (head == nullptr) return false;      // underflow

		QueueNode* gone = head;

		if (out != nullptr)
			QueueNode::copyText(gone->text, out);

		head = head->next;
		if (head == nullptr) tail = nullptr;    // now empty

		delete gone;
		count--;
		return true;
	}


	// O(1). Read the front without removing it.
	const char* peek() const
	{
		if (head == nullptr) return nullptr;
		return head->text;
	}

	bool isEmpty() const { return head == nullptr; }
	int  size()    const { return count; }

	void clear()
	{
		while (head != nullptr)
		{
			QueueNode* gone = head;
			head = head->next;
			delete gone;
		}
		tail  = nullptr;
		count = 0;
	}
};

#endif
