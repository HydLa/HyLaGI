#include "SignalHandler.h"

namespace hydla
{
namespace signal_handler
{

volatile sig_atomic_t interrupted = 0;

void interrupt_handler(int sig)
{
  interrupted = 1;
}

void term_handler(int sig)
{
  exit(-1);
}


}
}
