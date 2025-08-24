/*
  ll_ops_full.cpp

  ============================================================
  Singly Linked List utilities (interview-ready)
  ------------------------------------------------------------
  Included features:

  Cycle detection & analysis (Floyd / Tortoise & Hare)
    - floydMeetingNode(head) -> ListNode* (meeting point or nullptr)
    - hasCycle(head)         -> bool
    - detectCycleStart(head) -> ListNode* (entry node or nullptr)
    - cycleLength(head)      -> int  (0 if no cycle)

  Core list operations
    - LinkedListOps::reverse(head)             -> head of reversed list
    - LinkedListOps::removeNthFromEnd(head,n)  -> head with n-th from end removed
    - LinkedListOps::middleNode(head)          -> pointer to the middle node
    - LinkedListOps::mergeKLists(lists)        -> merged head of K sorted lists

  Intersection
    - getIntersectionNodeNoCycle(a,b)          -> first intersection for acyclic lists
    - getIntersectionNode(a,b)                 -> general: handles acyclic & cyclic lists

  Helpers
    - LLHelpers::build({1,2,3})       // build acyclic list
    - LLHelpers::print(head)          // print "a -> b -> c" (acyclic)
    - LLHelpers::printLimited(head,k) // safe print first k nodes (for cyclic lists)
    - LLHelpers::destroy(head)        // free acyclic list
    - LLHelpers::destroySafe(head)    // break cycle if present, then free

  Build:
    g++ -std=c++17 -O2 -Wall -Wextra ll_ops_full.cpp -o llops && ./llops
*/

#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <initializer_list>

// ---------------------------------
// Node definition & small helpers
// ---------------------------------
struct ListNode {
    int val{};
    ListNode* next{nullptr};
    ListNode() = default;
    explicit ListNode(int v) : val(v), next(nullptr) {}
    ListNode(int v, ListNode* n) : val(v), next(n) {}
};

namespace LLHelpers {
    // Build a list from initializer_list (returns head)
    inline ListNode* build(std::initializer_list<int> xs) {
        ListNode dummy(0);
        ListNode* tail = &dummy;
        for (int x : xs) {
            tail->next = new ListNode(x);
            tail = tail->next;
        }
        return dummy.next;
    }

    // Print list as "a -> b -> c" (acyclic only)
    inline void print(ListNode* head, const char* label = "list") {
        std::cout << label << " : ";
        for (ListNode* p = head; p; p = p->next) {
            std::cout << p->val;
            if (p->next) std::cout << " -> ";
        }
        std::cout << "\n";
    }

    // Print at most `limit` nodes (safe for cyclic lists)
    inline void printLimited(ListNode* head, int limit, const char* label = "list") {
        std::cout << label << " : ";
        int count = 0;
        for (ListNode* p = head; p && count < limit; p = p->next, ++count) {
            std::cout << p->val;
            if (p->next && count + 1 < limit) std::cout << " -> ";
        }
        if (count == limit) std::cout << " -> ...";
        std::cout << "\n";
    }

    // Destroy (free) a *simple* acyclic list
    inline void destroy(ListNode* head) {
        while (head) { ListNode* nxt = head->next; delete head; head = nxt; }
    }
}

// ===============================================
// Cycle utilities: Floyd's Tortoise & Hare
// ===============================================

// Detect if there is a cycle; returns meeting node if cycle exists, else nullptr.
ListNode* floydMeetingNode(ListNode* head) {
    if (!head || !head->next) return nullptr;
    ListNode* slow = head;
    ListNode* fast = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return slow; // meeting point
    }
    return nullptr;
}

// Public API: does a cycle exist?
bool hasCycle(ListNode* head) {
    return floydMeetingNode(head) != nullptr;
}

// If cycle exists, return the node where cycle begins; otherwise nullptr.
ListNode* detectCycleStart(ListNode* head) {
    ListNode* meet = floydMeetingNode(head);
    if (!meet) return nullptr;
    ListNode* p1 = head;
    ListNode* p2 = meet;
    while (p1 != p2) { p1 = p1->next; p2 = p2->next; }
    return p1; // cycle entry
}

// If cycle exists, return its length in nodes; else 0.
int cycleLength(ListNode* head) {
    ListNode* meet = floydMeetingNode(head);
    if (!meet) return 0;
    int len = 1;
    for (ListNode* p = meet->next; p != meet; p = p->next) ++len;
    return len;
}

namespace LLHelpers {
    // Destroy a list that might have a cycle: break the cycle first, then free.
    inline void destroySafe(ListNode* head) {
        if (!head) return;
        ListNode* start = detectCycleStart(head);
        if (start) {
            // Find the last node in the cycle (length-1 steps from start),
            // set last->next = nullptr to break the cycle.
            int len = cycleLength(head);
            ListNode* last = start;
            for (int i = 1; i < len; ++i) last = last->next;
            last->next = nullptr;
        }
        destroy(head);
    }
}

// ---------------------------------
// Linked-list operations "class"
// ---------------------------------
struct LinkedListOps {

    // 1) Reverse a list: iterative, in-place
    //    Time O(n), Space O(1)
    static ListNode* reverse(ListNode* head) {
        ListNode* prev = nullptr;
        ListNode* cur  = head;
        while (cur) {
            ListNode* nxt = cur->next;
            cur->next = prev;
            prev = cur;
            cur = nxt;
        }
        return prev;
    }

    // 2) Remove Nth from end (1-indexed)
    //    Uses fast/slow with a dummy head to handle removing the real head cleanly.
    //    If n > length, returns the original head unchanged.
    //    Time O(n), Space O(1)
    static ListNode* removeNthFromEnd(ListNode* head, int n) {
        if (!head || n <= 0) return head;
        ListNode dummy(0, head);
        ListNode* fast = &dummy;
        ListNode* slow = &dummy;

        // Move fast n steps ahead
        for (int i = 0; i < n; ++i) {
            fast = fast->next;
            if (!fast) return head; // n > length -> unchanged
        }

        // Move both until fast at last node
        while (fast->next) {
            fast = fast->next;
            slow = slow->next;
        }

        // slow->next is the node to remove
        ListNode* toDel = slow->next;
        slow->next = slow->next ? slow->next->next : nullptr;
        delete toDel;
        return dummy.next;
    }

    // 3) Find the middle node (second middle for even length)
    //    Time O(n), Space O(1)
    static ListNode* middleNode(ListNode* head) {
        if (!head) return nullptr;
        ListNode* slow = head;
        ListNode* fast = head;
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
        }
        return slow; // middle (second middle if even)
    }

    // 4) Merge K sorted lists using a min-heap
    //    Reuses existing nodes (no new node allocations).
    //    Time O(N log K), Space O(K), where N = total nodes.
    static ListNode* mergeKLists(const std::vector<ListNode*>& lists) {
        struct Cmp {
            bool operator()(const ListNode* a, const ListNode* b) const {
                return a->val > b->val; // min-heap by value
            }
        };
        std::priority_queue<ListNode*, std::vector<ListNode*>, Cmp> pq;

        for (ListNode* h : lists) if (h) pq.push(h);

        ListNode dummy(0);
        ListNode* tail = &dummy;

        while (!pq.empty()) {
            ListNode* node = pq.top(); pq.pop();
            tail->next = node;
            tail = tail->next;
            if (node->next) pq.push(node->next);
        }
        return dummy.next;
    }
};

// ===============================
// Intersection of two linked lists
// ===============================

// A) No-cycle case: classic two-pointer switch trick.
//    Time O(n+m), Space O(1).
ListNode* getIntersectionNodeNoCycle(ListNode* a, ListNode* b) {
    if (!a || !b) return nullptr;
    ListNode *p = a, *q = b;
    while (p != q) {
        p = p ? p->next : b;
        q = q ? q->next : a;
    }
    return p; // nullptr (no intersection) or the intersecting node
}

// Helpers for the cyclic case
static int lenTo(ListNode* head, ListNode* stop) {
    // count steps from head to 'stop' (exclusive); assumes stop is reachable
    int len = 0;
    while (head != stop) { head = head->next; ++len; }
    return len;
}
static ListNode* advanceN(ListNode* p, int d) {
    while (d-- > 0) p = p->next;
    return p;
}

// B) General case: lists may have cycles.
//    Uses detectCycleStart(). Cases:
// 1) both acyclic      -> use A.
// 2) one cyclic only   -> cannot intersect (acyclic would become cyclic) -> nullptr.
// 3) both cyclic:
//    3a) same cycle entry S -> possibly intersect before S; else intersection is S.
//    3b) different entries  -> traverse cycle of one to see if it reaches the other's entry.
//                              If yes, they intersect (in the cycle); else disjoint.
ListNode* getIntersectionNode(ListNode* a, ListNode* b) {
    if (!a || !b) return nullptr;

    ListNode* ca = detectCycleStart(a);
    ListNode* cb = detectCycleStart(b);

    // Case 1: both acyclic
    if (!ca && !cb) return getIntersectionNodeNoCycle(a, b);

    // Case 2: one cyclic, one not -> no intersection
    if ((ca && !cb) || (!ca && cb)) return nullptr;

    // Case 3: both cyclic
    if (ca == cb) {
        // Align prefixes up to the common cycle entry
        int la = lenTo(a, ca);
        int lb = lenTo(b, cb);
        ListNode* pa = advanceN(a, la > lb ? la - lb : 0);
        ListNode* pb = advanceN(b, lb > la ? lb - la : 0);

        // Walk until nodes equal or until reaching the cycle entry
        while (pa != pb && pa != ca && pb != cb) {
            pa = pa->next;
            pb = pb->next;
        }
        // If we met before the cycle, that's the first intersection; otherwise it's the entry
        return (pa == pb) ? pa : ca;
    } else {
        // Different cycle entries: check if cycles are the same loop.
        for (ListNode* p = ca->next; p != ca; p = p->next) {
            if (p == cb) return ca; // or cb; any node in the shared cycle is a valid intersection
        }
        return nullptr; // disjoint cycles
    }
}

// -------------------------
// Tiny demos in main()
// -------------------------
int main() {
    using namespace LLHelpers;

    // 1) Reverse
    {
        ListNode* a = build({1,2,3,4,5});
        print(a, "reverse() input ");
        ListNode* r = LinkedListOps::reverse(a);
        print(r, "reverse() output");
        destroy(r);
        std::cout << "\n";
    }

    // 2) Remove Nth from end
    {
        ListNode* a = build({1,2,3,4,5});
        print(a, "removeNth() input ");
        ListNode* b = LinkedListOps::removeNthFromEnd(a, 2); // remove '4'
        print(b, "removeNth() output");
        destroy(b);
        std::cout << "\n";
    }

    // 3) Middle node
    {
        ListNode* odd  = build({10,20,30,40,50});     // middle = 30
        ListNode* even = build({1,2,3,4,5,6});        // middle (second) = 4
        std::cout << "middle(odd)  : " << (LinkedListOps::middleNode(odd)->val)  << "\n";
        std::cout << "middle(even) : " << (LinkedListOps::middleNode(even)->val) << "\n";
        destroy(odd); destroy(even);
        std::cout << "\n";
    }

    // 4) Merge K sorted lists
    {
        ListNode* l1 = build({1,4,5});
        ListNode* l2 = build({1,3,4});
        ListNode* l3 = build({2,6});
        std::vector<ListNode*> lists = {l1, l2, l3};

        ListNode* m = LinkedListOps::mergeKLists(lists);
        print(m, "mergeK() output");  // 1->1->2->3->4->4->5->6

        // IMPORTANT: l1/l2/l3 nodes are now part of merged list `m`.
        // Do not destroy l1/l2/l3 separately (would double-delete).
        destroy(m);
        std::cout << "\n";
    }

    // 5) Cycle detection demo
    {
        // Build cyclic list: 10 -> 20 -> 30 -> 40 -> 50 -> back to 30
        ListNode* b1 = new ListNode(10);
        ListNode* b2 = new ListNode(20);
        ListNode* b3 = new ListNode(30);
        ListNode* b4 = new ListNode(40);
        ListNode* b5 = new ListNode(50);
        b1->next = b2; b2->next = b3; b3->next = b4; b4->next = b5; b5->next = b3; // cycle at 30

        std::cout << std::boolalpha;
        std::cout << "hasCycle: " << hasCycle(b1) << "\n";
        ListNode* start = detectCycleStart(b1);
        std::cout << "cycle start val: " << (start ? std::to_string(start->val) : "null") << "\n";
        std::cout << "cycle length: " << cycleLength(b1) << "\n";
        printLimited(b1, 10, "cyclic list (first 10 nodes)");

        // Cleanup safely (break cycle then free)
        destroySafe(b1);
        std::cout << "\n";
    }

    // 6) Intersection demos
    {
        // --- Acyclic intersection ---
        // x: 1 -> 2 -> 3
        // y: 9 -> 9 ->(join at node with value 2)
        ListNode* x1 = build({1,2,3});
        ListNode* y1 = build({9,9});
        y1->next->next = x1->next; // intersect at '2'
        ListNode* inter1 = getIntersectionNode(x1, y1);
        std::cout << "Intersection (acyclic): "
                  << (inter1 ? std::to_string(inter1->val) : "null") << "\n";
        // Free the combined structure once (don't double-delete):
        destroy(x1); // frees shared tail; y1 is now dangling (demo OK)

        // --- Cyclic intersection ---
        // c1: 1 -> 2 -> 3 -> [4 -> 5 -> 6 -> back to 5]
        // c2: 7 -> 8 -------^ (points into cycle at '5')
        ListNode* c1 = build({1,2,3});
        ListNode* c2 = build({7,8});
        ListNode* loop = build({4,5,6});
        c1->next->next->next = loop;          // 3 -> 4
        c2->next->next = loop->next;          // 8 -> 5 (cycle entry at 5)
        // make the cycle: 6 -> 5
        ListNode* tail = loop; while (tail->next) tail = tail->next;
        tail->next = loop->next;

        ListNode* inter2 = getIntersectionNode(c1, c2);
        std::cout << "Intersection (cyclic): "
                  << (inter2 ? std::to_string(inter2->val) : "null") << "\n";

        // Cleanup safely: one call is enough to free the whole structure
        destroySafe(c1);
        // (Do not destroy c2 separately; it would be dangling now.)
    }

    return 0;
}

