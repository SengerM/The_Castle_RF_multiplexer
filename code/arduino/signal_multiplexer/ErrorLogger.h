#ifndef _STATUS_H_
#define _STATUS_H_

#include <Arduino.h>

#define MAX_ERROR_LOGS 3
#define MAX_ERROR_MSG_LENGTH 111
#define LINE_TERMINATION '\n'
#define ERROR_LOGGER_MSG_HEADDER "ErrorLogger: "

typedef enum {
	NO_ERROR, // Used internally, you shouldn't need to use NO_ERROR
	WARNING, // Use WARNING when there is something wrong but your device can still keep working
	ERROR, // Use ERROR when the problem needs human attention
	FATAL // Use FATAL if the problem is so severe that your device has to take an aborting action (not yet implemented)
} error_severity_t;

class Error {
private:
	error_severity_t severity;
	char msg[MAX_ERROR_MSG_LENGTH];
	unsigned int msg_length;
public:
	Error();
	Error(const Error & err);
	Error(error_severity_t _severity, char * _msg); // Use this to create errors
	void report_serial(); // Reports the error through the serial port
	error_severity_t get_severity(void);
};

class ErrorLogger {
private:
	Error errors[MAX_ERROR_LOGS];
	int error_led_pin;
	int buzzer_pin;
	unsigned int n_errors;
public:
	ErrorLogger();
	ErrorLogger(int error_led_pin); // In this case an LED is turned on if there is any error in the ErrorLogger
	ErrorLogger(int error_led_pin, int error_buzzer_pin); // This mode also allows a buzzer
	void new_error(const Error & error); // Use this to add an error to the ErrorLogger
	void report_first_error(); // Errors are stored in a FIFO array
	void report_all_errors();
};

#endif
