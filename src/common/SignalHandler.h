#pragma once

#include <signal.h>
#include <stdlib.h>

namespace hydla {
namespace signal_handler {

extern volatile sig_atomic_t interrupted;

void interrupt_handler(int sig);
void term_handler(int sig);

} // namespace signal_handler
} // namespace hydla
