#include "stack.hpp"
#include "queue.hpp"
#include "shared_ptr.hpp"
#include "atomic_shared_ptr.hpp"

int main() {
  LF_TEST_RUN(stack)
  LF_TEST_RUN(queue)
  LF_TEST_RUN(shared_ptr)
  LF_TEST_RUN(atomic_shared_ptr)
}
