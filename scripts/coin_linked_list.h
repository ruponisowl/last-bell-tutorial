#ifndef COIN_LINKED_LIST_H
#define COIN_LINKED_LIST_H

// ============================================================================
//  Last Bell: Campus Rush  --  Custom Singly Linked List
//  Data Structures Lab Project, CSE 2103
//
//  This is the SAME linked list you already tested in the console. It has
//  been merged into one header file so a Jenova C++ script can #include it
//  with no build setup. The logic is unchanged.
//
//  NO Godot code. NO STL containers. Every node is allocated by hand.
// ============================================================================

#include <cstdio>   // printf -- shows up in Godot's Output panel

// The result of trying to collect a coin.
// Three outcomes, because gameplay needs to tell them apart:
//   Success          -> add score, hide the sprite
//   AlreadyCollected -> do nothing (this is what stops re-collecting)
//   NotFound         -> a bug: a sprite with no data record behind it
enum class CollectResult {
    Success,
    AlreadyCollected,
    NotFound
};

// ----------------------------------------------------------------------------
//  CoinNode -- one coin's DATA record (not its picture on screen)
// ----------------------------------------------------------------------------
class CoinNode {
public:
    int   id;         // unique identifier, e.g. 101, 102, ...
    int   section;    // 1 = parking lot, 2 = guard, 3 = elevator shaft
    bool  collected;  // has the player picked this up yet?
    int   value;      // score this coin is worth (in tenths of a CG)
    float x, y;       // where it sits in the level

    CoinNode* next;   // pointer to the NEXT node, or nullptr if last

    CoinNode(int p_id, int p_section, int p_value, float p_x, float p_y) {
        id        = p_id;
        section   = p_section;
        value     = p_value;
        x         = p_x;
        y         = p_y;
        collected = false;
        next      = nullptr;   // ALWAYS set this
    }
};

// ----------------------------------------------------------------------------
//  CoinLinkedList -- owns and manages the chain of CoinNodes
// ----------------------------------------------------------------------------
class CoinLinkedList {
private:
    CoinNode* head;   // first node, nullptr when empty
    CoinNode* tail;   // last node -- kept so insert is O(1), not O(n)
    int       count;

public:
    CoinLinkedList() {
        head  = nullptr;
        tail  = nullptr;
        count = 0;
    }

    ~CoinLinkedList() {
        clear();   // every `new` is matched by a `delete` in clear()
    }

    // Copying would make two lists own the same nodes, and both would
    // delete them -- a crash. Forbidden. (The "Rule of Three".)
    CoinLinkedList(const CoinLinkedList&) = delete;
    CoinLinkedList& operator=(const CoinLinkedList&) = delete;

    // ------------------------------------------------------------------
    //  SEARCH -- find a node by id. O(n): no index exists, so we walk.
    // ------------------------------------------------------------------
    CoinNode* search(int id) const {
        CoinNode* current = head;
        while (current != nullptr) {
            if (current->id == id) return current;
            current = current->next;
        }
        return nullptr;             // walked the whole list, no match
    }

    // ------------------------------------------------------------------
    //  INSERT -- append a record. Returns false if the id already exists.
    //  O(n) because of the duplicate check; the append itself is O(1).
    // ------------------------------------------------------------------
    bool insert(int id, int section, int value, float x, float y) {
        if (search(id) != nullptr) return false;     // reject duplicates

        CoinNode* node = new CoinNode(id, section, value, x, y);

        if (head == nullptr) {
            head = node;            // CASE 1: empty list
            tail = node;
        } else {
            tail->next = node;      // CASE 2: link old tail to new node
            tail       = node;
        }

        count++;
        return true;
    }

    // ------------------------------------------------------------------
    //  COLLECT -- the UPDATE operation, and the heart of the game.
    //  O(n) search, then O(1) to flip the flag.
    // ------------------------------------------------------------------
    CollectResult collect(int id) {
        CoinNode* node = search(id);

        if (node == nullptr)  return CollectResult::NotFound;
        if (node->collected)  return CollectResult::AlreadyCollected;

        node->collected = true;
        return CollectResult::Success;
    }

    // ------------------------------------------------------------------
    //  DELETE -- unlink and free one node. O(n).
    //  Used for section cleanup and resets, not on every pickup.
    // ------------------------------------------------------------------
    bool remove(int id) {
        if (head == nullptr) return false;          // EDGE: empty list

        if (head->id == id) {                       // EDGE: delete head
            CoinNode* doomed = head;
            head = head->next;
            if (head == nullptr) tail = nullptr;    // EDGE: was the only node
            delete doomed;
            count--;
            return true;
        }

        // Walk with a "previous" pointer, because a singly linked list
        // cannot step backwards to repair the link.
        CoinNode* previous = head;
        while (previous->next != nullptr) {
            if (previous->next->id == id) {
                CoinNode* doomed = previous->next;
                previous->next   = doomed->next;    // bridge over it
                if (doomed == tail) tail = previous; // EDGE: deleted tail
                delete doomed;
                count--;
                return true;
            }
            previous = previous->next;
        }

        return false;                               // EDGE: id not found
    }

    // Delete every record in one section. Used when a section is locked
    // behind the player and can never be revisited.
    int removeSection(int section) {
        int removed = 0;
        bool again = true;
        while (again) {
            again = false;
            for (CoinNode* c = head; c != nullptr; c = c->next) {
                if (c->section == section) {
                    remove(c->id);
                    removed++;
                    again = true;      // restart: the chain just changed
                    break;
                }
            }
        }
        return removed;
    }

    // ------------------------------------------------------------------
    //  CLEAR -- free every node. O(n) time, O(1) extra space.
    // ------------------------------------------------------------------
    void clear() {
        CoinNode* current = head;
        while (current != nullptr) {
            CoinNode* nextNode = current->next;  // SAVE next BEFORE delete!
            delete current;                      // reading current->next
            current = nextNode;                  // after this is undefined
        }
        head  = nullptr;
        tail  = nullptr;
        count = 0;
    }

    // ------------------------------------------------------------------
    //  Queries used by the HUD
    // ------------------------------------------------------------------
    int  size()    const { return count; }
    bool isEmpty() const { return head == nullptr; }

    int collectedCount() const {
        int n = 0;
        for (CoinNode* c = head; c != nullptr; c = c->next)
            if (c->collected) n++;
        return n;
    }

    // Total score in tenths. 40 tenths = CG 4.00
    int totalScore() const {
        int score = 0;
        for (CoinNode* c = head; c != nullptr; c = c->next)
            if (c->collected) score += c->value;
        return score;
    }

    int remainingInSection(int section) const {
        int n = 0;
        for (CoinNode* c = head; c != nullptr; c = c->next)
            if (c->section == section && !c->collected) n++;
        return n;
    }

    // TRAVERSE -- print the whole list. This is your live-demo function:
    // run it before and after a pickup to show the structure changing.
    void traverse() const {
        if (head == nullptr) {
            printf("  (coin list is empty)\n");
            return;
        }
        printf("  HEAD\n");
        for (CoinNode* c = head; c != nullptr; c = c->next) {
            printf("   -> [id=%d  section=%d  value=%d  pos=(%.0f,%.0f)  %s]\n",
                   c->id, c->section, c->value, c->x, c->y,
                   c->collected ? "COLLECTED" : "not collected");
        }
        printf("   -> nullptr\n");
    }
};

#endif // COIN_LINKED_LIST_H
