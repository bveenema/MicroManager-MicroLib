#include "MM_Config.h"
#include "MM_Manager.h"

#include <stdio.h>

MM_Manager::MM_Manager(const size_t numObjects)
{
    CommandMap.objects = new ObjectInterface*[numObjects];
    CommandMap.max_size = numObjects;
}

bool MM_Manager::Add(ObjectInterface& Object)
{
    // Out of bounds check
    if(CommandMap.size >= CommandMap.max_size) return false;

    // Add the object to the next position in the CommandMap
    CommandMap.objects[CommandMap.size] = &Object;
    CommandMap.size += 1;
    return true;
}

void MM_Manager::Read(char c)
{
    /*
    Serial Stream comes in form of:
        {key}:{value} for a write, and
        {key} (with no ":") for a read
    followed by a newline character.

    ex. WRITE   --> 1:250
        READ    --> 1
        SPECIAL --> CONFIG
    */
	if(messageIndex >= messageBufferSize) messageIndex = 0; // prevent overflow

	if(FLAG_NoRead) return; // ignore stream while handling current message (prevents value and key buffer from changing while handling)

	if(c != '\n' && c != ':') // Regular characters
	{
		messageBuffer[messageIndex++] = c;
	}

	else if(c == ':') // key/value separator (write signifier)
	{
		messageBuffer[messageIndex] = 0; // Null terminate string
		FLAG_isWrite = true; // Next Stream is value to write
		strcpy(keyBuffer, messageBuffer); // Stream was a key
		messageIndex = 0; // reset buffer position
	}

	else // \n character
	{
		messageBuffer[messageIndex] = 0; // Null terminate string
		if(FLAG_isWrite)
		{
			strcpy(valueBuffer, messageBuffer); // Stream was new value
		}
		else
		{
			strcpy(keyBuffer, messageBuffer); // Stream was a key
		}
		messageIndex = 0; // reset buffer position

		// Interpet the completed command
		MessageHandler();
	}
}

void MM_Manager::MessageHandler()
{
	FLAG_NoRead = true; // Prevent reading new strings into key and value buffer while Handler is running

	unsigned commandNum = atol(keyBuffer); // Attempt to convert the key to a number

	if(commandNum == 0) // keyBuffer was string
	{
		if(strcmp(keyBuffer, "MICROMANAGER")) // App sends micro manager when first connected, send "READY" as response
		{
			MM_Serial_Print("READY\n");
		}
		else if(strcmp(keyBuffer, "CONFIG")) // App is requesting the configuration JSON string
		{
			char ConfigJSON[1024] = "{\"baud\":9600,\"name\":\"DeviceName\",\"settings\":[{\"n\":\"SettingName\",\"t\":\"SettingType\",\"c\":1,\"l\":20,\"h\":30,\"d\":20,\"r\":22,\"f\":0,\"i\":\"SettingUnit\"},],\"state\":[{\"n\":\"StateName\",\"t\":\"StateType\",\"c\":7,\"l\":0,\"h\":25,\"u\":250},]}";
			//TODO compile JSON string
			MM_Serial_Print(ConfigJSON);
		}
	}
	else // keyBuffer was a command
	{
		
	}

	FLAG_NoRead = false; // Allow new messages
}

void MM_Manager::test()
{
    char buffer[100];
    sprintf(buffer, "CommandMap:\n"
                    "  size: %d\n"
                    "  max size: %d\n"
                    "  Elements:\n", CommandMap.size, CommandMap.max_size);
    for(unsigned i=0; i<CommandMap.size; i++)
    {
        char valBuffer[12];
        CommandMap.objects[i]->sValue(valBuffer);
        sprintf(buffer + strlen(buffer), "    %s\n", valBuffer);
    }
    
    MM_Serial_Print(buffer);
}
