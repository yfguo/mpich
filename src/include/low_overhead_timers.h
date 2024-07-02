/* This code is taken from https://github.com/jdmccalpin/low-overhead-timers
 * By: John D. McCalpin
 * Original License
 * BSD 3-Clause License

   Copyright (c) 2018, John D McCalpin and University of Texas at Austin
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

   * Neither the name of the copyright holder nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Some timers
//
// rdtsc() returns the number of "nominal" processor cycles since the system booted in a 64-bit unsigned integer.
//       For all recent Intel processors, this counter increments at a fixed rate, independent of the actual
//       core clock speed or the energy-saving mode.
// rdtscp() is the same as rdtsc except that it is partially ordered -- it will not execute until all prior
//       instructions in program order have executed.  (See also full_rdtscp)
// full_rdtscp() returns the number of "nominal" processor cycles in a 64-bit unsigned integer and also
//       modifies its two integer arguments to show the processor socket and processor core that were in use
//       when the call was made.  (Note: the various cores in a chip usually have very similar values for
//       the TSC, but they are allowed to vary by processor.  This function guarantees that you know exactly
//       which processor the TSC reading came from.)
// get_core_number() uses the RDTSCP instruction, but returns only the core number in an integer variable.
// get_socket_number() uses the RDTSCP instruction, but returns only the socket number in an integer variable.
// rdpmc_instructions() uses a "fixed-function" performance counter to return the count of retired instructions on
//       the current core in the low-order 48 bits of an unsigned 64-bit integer.
// rdpmc_actual_cycles() uses a "fixed-function" performance counter to return the count of actual CPU core cycles
//       executed by the current core.  Core cycles are not accumulated while the processor is in the "HALT" state,
//       which is used when the operating system has no task(s) to run on a processor core.
// rdpmc_reference_cycles() uses a "fixed-function" performance counter to return the count of "reference" (or "nominal")
//       CPU core cycles executed by the current core.  This counts at the same rate as the TSC, but does not count
//       when the core is in the "HALT" state.  If a timed section of code shows a larger change in TSC than in
//       rdpmc_reference_cycles, the processor probably spent some time in a HALT state.
// rdpmc() reads the programmable core performance counter number specified in the input argument.
//               No error or bounds checking is performed.
//
// get_TSC_frequency() parses the Brand Identification string from the CPUID instruction to get the "nominal"
//       frequency of the processor, which is also the invariant TSC frequency, and returned as a float value in Hz.
//       This can then be used to convert TSC cycles to seconds.
//

#ifndef LOW_OVERHEAD_TIMERS_H_INCLUDED
#define LOW_OVERHEAD_TIMERS_H_INCLUDED

unsigned long rdtsc();
unsigned long rdtscp();
unsigned long full_rdtscp(int *chip, int *core);
int get_core_number();
int get_socket_number();
unsigned long rdpmc_instructions();
unsigned long rdpmc_actual_cycles();
unsigned long rdpmc_reference_cycles();
unsigned long rdpmc(int c);
int get_num_core_counters();
int get_core_counter_width();
int get_fixed_counter_width();
unsigned long corrected_pmc_delta(unsigned long end, unsigned long start, int pmc_width);
float get_TSC_frequency();

#endif /* end of include guard: LOW_OVERHEAD_TIMERS_H_INCLUDED */
