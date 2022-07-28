#include "ErrorLogger.h"

void _warning_sound_(int pin) {
	tone(pin, 1000, 50);
}

void _error_sound_(int pin) {
	tone(pin, 1000, 500);
}

void _fatal_sound_(int pin) {
	tone(pin, 1000, 1000);
}

void _play_sound_alarm_(error_severity_t severity, int pin) {
	if (pin < 0)
		return;
	switch (severity) {
		case NO_ERROR: return;
		case WARNING: _warning_sound_(pin); break;
		case ERROR: _error_sound_(pin); break;
		case FATAL: _fatal_sound_(pin); break;
		default: return;
	}
}

Error::Error() {
	this-> severity = NO_ERROR;
	this-> msg_length = 0;
}

Error::Error(const Error & err) {
	this->severity = err.severity;
	unsigned int i=0;
	while (err.msg[i] != '\0') {
		this->msg[i] = err.msg[i];
		if (++i == MAX_ERROR_MSG_LENGTH) break;
	}
	this->msg_length = i;
}

Error::Error(error_severity_t _severity, char * _msg) {
	this->severity = _severity;
	unsigned int i=0;
	while (_msg[i] != '\0') {
		this->msg[i] = _msg[i];
		if (++i == MAX_ERROR_MSG_LENGTH) break;
	}
	this->msg_length = i;
}

void Error::report_serial() {
	switch (this->severity) {
		case NO_ERROR: return;
		case WARNING: Serial.print(F("WARNING: ")); break;
		case ERROR: Serial.print(F("ERROR: ")); break;
		case FATAL: Serial.print(F("FATAL: ")); break;
		default: return;
	}
	for (int i=0; i<this->msg_length; i++)
		Serial.print(this->msg[i]);
	Serial.print(LINE_TERMINATION);
	if (this->severity == FATAL) {
		Serial.print(F("Execution will be halted due to fatal error, restart the device."));
		Serial.print(LINE_TERMINATION);
		while (true);
	}
}

error_severity_t Error::get_severity(void) {
	return this->severity;
}

ErrorLogger::ErrorLogger(void) {
	this->error_led_pin = -1;
	this->buzzer_pin = -1;
	this->n_errors = 0;
}

ErrorLogger::ErrorLogger(int error_led_pin) {
	this->error_led_pin = error_led_pin;
	this->buzzer_pin = -1;
	pinMode(error_led_pin, OUTPUT);
	this->n_errors = 0;
}

ErrorLogger::ErrorLogger(int error_led_pin, int buzzer_pin) {
	this->error_led_pin = error_led_pin;
	this->buzzer_pin = buzzer_pin;
	pinMode(error_led_pin, OUTPUT);
	pinMode(buzzer_pin, OUTPUT);
	this->n_errors = 0;
}

void ErrorLogger::new_error(const Error & error) {
	_play_sound_alarm_(error.get_severity(), this->buzzer_pin);
	if (error.get_severity() == FATAL)
		this->report_all_errors();
	if (this->n_errors < MAX_ERROR_LOGS) {
		this->errors[this->n_errors] = error;
		this->n_errors += 1;
	} else {
		Serial.print(F(ERROR_LOGGER_MSG_HEADDER));
		Serial.print(F("Error logger is full, cannot log the following error: "));
		error.report_serial();
	}
	if (this->error_led_pin >= 0) digitalWrite(error_led_pin, HIGH);
}

void ErrorLogger::report_first_error() {
	if (this->n_errors > 0) {
		this->errors[0].report_serial();
		for (int i=1; i < this->n_errors; i++)
			this->errors[i-1] = this->errors[i];
		this->n_errors -= 1;
	} else {
		Serial.print(F(ERROR_LOGGER_MSG_HEADDER));
		Serial.print(F("Empty, no errors"));
		Serial.print(LINE_TERMINATION);
	}
	if (this->n_errors == 0)
		if (this->error_led_pin >= 0) digitalWrite(error_led_pin, LOW);
}

void ErrorLogger::report_all_errors() {
	while (this->n_errors > 0)
		this->report_first_error();
}

