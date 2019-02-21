#ifndef TEST_FUNCTIONS_H
#define TEST_FUNCTIONS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
#include <list>
#include <chrono>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>
#include <execinfo.h>
#include "muParserTest.h"

using namespace std;
using namespace std::chrono;
typedef high_resolution_clock Clock;

typedef int (mu::Test::ParserTester::*TPrueba)();

void check_test_case(bool result);

class TestFunctions {

	public:
		/*Constructor:
			- measure_time: set to true for measuring the time of the tests (false by default).
			- timeout_seconds: set a timeout in seconds (10 seconds by default).	
		*/
		TestFunctions(bool measure_time = false, unsigned int timeout_seconds = 10, bool continue_test = true);

		/* set_timetout:  sets a timeout in seconds.
		*/
		void set_timeout(unsigned int seconds);

		/* test_case: starts the execution of a new test case:
			- pru: name of the method to invoke for the test case execution.
		*/
		void test_case(TPrueba pru);

		bool has_failed();     

                bool continue_execution();
	private:	

		/* save_result:  saves the result of a test case:
			result = 1: pass
			result = 0: fail
		*/
		void save_result(unsigned int result);

		/* begin_test:  starts the execution of a test case in order to measure its time. 
				It should be invoked only when the user wants to measure the time 
				from a certain point in the code until the next calling to "save_result".
		*/
		void begin_test();

		//Check correct results
		bool is_valid_result(unsigned int o);	


		//To measure the time
		bool active;	   				// activity state of timer.
       		high_resolution_clock::time_point t0, t1;    	// initial and final time.

		//Timeout
		static bool time_out;
		static void reset_timeout();
		unsigned int secs; 

		//Booleans
		bool continue_test; 				//continue or not when a test case fails
		static bool failed_test_case;			//if continue_test is "true", failed_test_case indicates when the test case fails

		//Signals
		static void capture(int n);
		static void captureChild(int n);
		static void captureAlarmChild(int n);
		static void captureTERM(int n);
		static void captureTERMChild(int n);
		static void initializeCapture();
		static void initializeCaptureChild();
		static void establishSignalStack();
		static void finish_child();
};

#endif
