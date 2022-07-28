#include <SerialCommands.h> // https://www.arduino.cc/reference/en/libraries/serialcommands/
#include "ErrorLogger.h"
#include <EEPROM.h> // https://www.arduino.cc/en/Reference/EEPROM

// Controller for the HMC321ALP4E multiplexer, see https://www.analog.com/media/en/technical-documentation/data-sheets/hmc321a.pdf

#define __VERSION__ "1"
#define CTLA_PIN_NUMBER A0
#define CTLB_PIN_NUMBER A1
#define CTLC_PIN_NUMBER A2

ErrorLogger error_logger(13);

// Stuff related to the serial commands --------------------------------
bool command_has_just_finished = false; // Each command must switch this to `true` before every return.

byte currently_connected_channel;

void write_current_connected_channel_to_EEPROM(void) {
	EEPROM.write(0, currently_connected_channel);
}

byte read_connected_channel_from_EEPROM(void) {
	return EEPROM.read(0);
}

void connect_channel(uint8_t channel_number) {
	if (channel_number <= 0 or channel_number > 8) {
		error_logger.new_error(Error(ERROR, "Attempt to connect a channel that is not one of 1,2,3,4,5,6,7,8"));
		return;
	}
	channel_number -= 1;
	digitalWrite(CTLA_PIN_NUMBER, channel_number & (1));
	digitalWrite(CTLB_PIN_NUMBER, channel_number & (1<<1));
	digitalWrite(CTLC_PIN_NUMBER, channel_number & (1<<2));
	
	currently_connected_channel = channel_number+1;
	write_current_connected_channel_to_EEPROM();
}

void cmd_unrecognized(SerialCommands* sender, const char* cmd) {
	// Default command handler when command does not match other commands.
	sender->GetSerial()->print("ERROR: Unknown command '");
	sender->GetSerial()->print(cmd);
	sender->GetSerial()->println("'");
	command_has_just_finished = true;
}

void log_wrong_argument_error(void) {
	error_logger.new_error(Error(ERROR, "Bad arguments for command."));
}

void cmd_idn(SerialCommands* sender) {
	sender->GetSerial()->println("8 channels RF multiplexer, aka The Castle");
	command_has_just_finished = true;
}

void cmd_version(SerialCommands* sender) {
	sender->GetSerial()->println("Version " __VERSION__ " - " __DATE__ " " __TIME__);
	command_has_just_finished = true;
}

void cmd_connect_channel(SerialCommands* sender) {
	char* argument = sender->Next();
	if (argument == NULL) {
		log_wrong_argument_error();
		command_has_just_finished = true;
		return;
	}
	byte channel = *argument - '0'; // https://forum.arduino.cc/t/converting-char-to-int/101204/3
	connect_channel(channel);
	command_has_just_finished = true;
}

void cmd_connected_channel_query(SerialCommands* sender) {
	sender->GetSerial()->println(currently_connected_channel);
}

void cmd_help(SerialCommands* sender) {
	sender->GetSerial()->println("Available commands:");
	sender->GetSerial()->println("- `HELP?`: Print this help.");
	sender->GetSerial()->println("- `IDN?`: Returns information about the device.");
	sender->GetSerial()->println("- `VERSION?`: Returns information about the firmware version.");
	sender->GetSerial()->println("- `CONNECT_CHANNEL x`: Connects a channel, `x` is the channel number from 1,2,...,8. Example: `CONNECT_CHANNEL 3.`");
	sender->GetSerial()->println("- `CONNECTED_CHANNEL?`: Return the number of the channel currently connected.");
}

char serial_command_buffer[32];
SerialCommands serial_commands(&Serial, serial_command_buffer, sizeof(serial_command_buffer), "\n", " ");
SerialCommand commands_list[] = {
	SerialCommand("IDN?", cmd_idn),
	SerialCommand("VERSION?", cmd_version),
	SerialCommand("CONNECT_CHANNEL", cmd_connect_channel),
	SerialCommand("HELP?", cmd_help),
	SerialCommand("CONNECTED_CHANNEL?", cmd_connected_channel_query),
};

// ---------------------------------------------------------------------

void serial_flush(void) {
	while (Serial.available())
		Serial.read();
}

void setup() {
	pinMode(CTLA_PIN_NUMBER, OUTPUT);
	pinMode(CTLB_PIN_NUMBER, OUTPUT);
	pinMode(CTLC_PIN_NUMBER, OUTPUT);
	
	Serial.begin(9600);
	serial_commands.SetDefaultHandler(cmd_unrecognized);
	for (uint16_t i = 0; i < sizeof(commands_list)/sizeof(commands_list[0]); i++)
		serial_commands.AddCommand(&commands_list[i]);
	
	connect_channel(read_connected_channel_from_EEPROM());
}

void loop() {
	while (not Serial.available()) {
		if (command_has_just_finished == true) {
			serial_flush(); // Discard any garbage that was sent while the command was being executed.
			command_has_just_finished = false; // Reset this variable.
		}
	} // Wait...
	serial_commands.ReadSerial(); // Do!
	error_logger.report_all_errors(); // Report.
}
