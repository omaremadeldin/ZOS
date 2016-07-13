//==========================================
//
//		      	ZapperOS - CLI
//
//==========================================
//By Omar Emad Eldin
//==========================================

void cliprint(char c)
{
	if (currentY == currentROWS - 1)
	{
		print("Press any key to continue...\n");
		getKey();
		clearScreen();
	}

	printCh(c);
}

void cliprint(const char* s)
{
	if (s == NULL)
		return;
	
	int i = 0;

	while (s[i] != 0)
	{
		printCh(s[i]);
		i++;

		if (currentY == currentROWS - 1)
		{
			print("Press any key to continue...\n");
			getKey();
			clearScreen();
		}
	}
	
	UpdateCursor();
}

void cliprintf(const char* s, ...)
{
	if (s == NULL)
		return;
	
	va_list args;
	va_start(args,s);
	
	char str[256];
	formatWithArgs(s, str, args);
	cliprint(str);

	va_end(args);

	UpdateCursor();
}

void startCLI()
{
	clearScreen();

	cliprint("ZOS CLI (Command Line Interface)\n\
		Press any key to start...");

	getKey();
	clearScreen();

	bool quitFlag = false;

	while (!quitFlag)
	{
		string cmd;

		cliprint("ZOS#>");

		while (true)
		{
			KEYCODE key = getKey();

			char cKey = kybrdKeyToASCII(key);

			if (key == KEY_RETURN)
			{
				cliprint("\n");
				break;
			}
			else if ((key == KEY_BACKSPACE) && (cmd.length() > 0))
			{
				cmd.erase(cmd.length() - 1);

				if ((currentX == 0) && (currentY != 0))
				{
					currentX = currentCOLS - 1;
					currentY--;
				}
				else if (currentX != 0)
				{
					currentX--;
				}

				UpdateCursor();

				cliprint(' ');

				if ((currentX == 0) && (currentY != 0))
				{
					currentX = currentCOLS - 1;
					currentY--;
				}
				else if (currentX != 0)
				{
					currentX--;
				}

				UpdateCursor();
			}
			else if (isprint(cKey))
			{
				cmd += cKey;
				cliprint(cKey);
				UpdateCursor();
			}
		}

		if (cmd.length() == 0)
		{
			cliprint("Type 'help' for a list of commands.\n");
			continue;
		}
		else
		{
			char* cCmd = new char[cmd.length() + 1];
			cmd.copy(cCmd, cmd.length(), 0);

			char* command = strtok(cCmd, " ");

			if (strcmp(command, "help", true))
			{
				cliprint("\nZOS - Made By Omar Emad Eldin\n\
					-------------------------------\n\
					List of commands:\n\
					  - help : displays this message\n\
					  - list : displays various lists about system components\n\
					  - halt : ends execution\n\n");
			}
			else if (strcmp(command, "list", true))
			{
				command = strtok(NULL, " ");

				if (command == NULL)
				{
					cliprint("\nAvalible lists:\n\
						  - Ide\n\
						  - Partitions\n\
						  - Volumes\n\n");
				}
				else if (strcmp(command, "ide", true))
				{
					cliprintf("\nIDE Device List(%i) :\n", ideDeviceCount);
	
					for (uint16_t i = 0; i < ideDeviceCount; i++)
	 					cliprintf("Index:%i, Model:%s\n", i, ideDevices[i].Model);

	 				cliprint("\n");
				}
				else if (strcmp(command, "partitions", true))
				{
					cliprintf("\nPartition List(%i) :\n", pmgrPartitionCount);

					for (uint16_t i = 0; i < pmgrPartitionCount; i++)
						cliprintf("IDE:%i, SYSID:0x%x, Size:0x%x\n", pmgrPartitions[i].IDEIndex, pmgrPartitions[i].systemID, pmgrPartitions[i].totalSectors);
				
					cliprint("\n");
				}
				else if (strcmp(command, "volumes", true))
				{
					cliprintf("\nVolume List(%i) :\n", vmgrVolumeCount);

					for (uint16_t i = 0; i < vmgrVolumeCount; i++)
						cliprintf("Name:%s Mount Point:%c, FS Name:%s\n", vmgrVolumes[i].Name, vmgrVolumes[i].mountPoint, vmgrVolumes[i].fileSystem->Name.c_str());
				
					cliprint("\n");
				}
				else
				{
					cliprint("Invalid list.\n");
				}
			}
			else if (strcmp(command, "halt", true))
			{
				quitFlag = true;
			}
			else
			{
				cliprintf("'%s' is an invalid command, type 'help' for a list of commands.\n", command);
			}

			delete [] cCmd;
		}
	}
}