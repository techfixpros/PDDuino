void loop () {
 DEBUG_PRINTL(F("loop(): start"));

  byte rType = 0x00; // Current request type (command type)
  byte rLength = 0x00; // Current request length (command length)
  byte diff = 0x00;  // Difference between the head and tail buffer indexes

  state = 0x00; // 0 = waiting for command, 1 = waiting for full command, 2 = have full command

  while(state<0x02){ // While waiting for a command...
#if defined(ENABLE_SLEEP)
    sleepNow();
#endif // ENABLE_SLEEP
    while (CLIENT.available() > 0x00){ // While there's data to read from the TPDD port...
#if defined(ENABLE_SLEEP)
 #if defined(SLEEP_DELAY)
      idleSince = millis();
 #endif // SLEEP_DELAY
#endif // ENABLE_SLEEP
      dataBuffer[head++]=(byte)CLIENT.read();  //...pull the character from the TPDD port and put it into the command buffer, increment the head index...
      if(tail==head)tail++; //...if the tail index equals the head index (a wrap-around has occoured! data will be lost!)
                            //...increment the tail index to prevent the command size from overflowing.

#if DEBUG > 1
      DEBUG_PRINTI((byte)(head-1),HEX);
      DEBUG_PRINT("-");
      DEBUG_PRINTI(tail,HEX);
      DEBUG_PRINTI((byte)(head-tail),HEX);
      DEBUG_PRINT(":");
      DEBUG_PRINTI(dataBuffer[head-1],HEX);
      DEBUG_PRINT(";");
      DEBUG_PRINTL((dataBuffer[head-1]>=0x20)&&(dataBuffer[head-1]<=0x7E)?(char)dataBuffer[head-1]:' ');
#endif
    }

    diff=(byte)(head-tail); //...set the difference between the head and tail index (number of bytes in the buffer)

    if(state == 0x00){ //...if we're waiting for a command...
      if(diff >= 0x04){  //...if there are 4 or more characters in the buffer...
        if(dataBuffer[tail]=='Z' && dataBuffer[(byte)(tail+0x01)]=='Z'){ //...if the buffer's first two characters are 'Z' (a TPDD command)
          rLength = dataBuffer[(byte)(tail+0x03)]; //...get the command length...
          rType = dataBuffer[(byte)(tail+0x02)]; //...get the command type...
          state = 0x01;  //...set the state to "waiting for full command".
        }else if(dataBuffer[tail]=='M' && dataBuffer[(byte)(tail+0x01)]=='1'){ //If a DME command is received
          DME = true; //set the DME mode flag to true
          tail=tail+0x02;  //and skip past the command to the DME request command
        }else{  //...if the first two characters are not 'Z'...
          tail=tail+(tail==head?0x00:0x01); //...move the tail index forward to the next character, stop if we reach the head index to prevent an overflow.
        }
      }
    }

    if(state == 0x01){ //...if we're waiting for the full command to come in...
      if(diff>rLength+0x04){ //...if the amount of data in the buffer satisfies the command length...
          state = 0x02;  //..set the state to "have full command".
        }
    }
  }

#if DEBUG > 1
  DEBUG_PRINTI(tail,HEX); // show the tail index in the buffer where the command was found...
  DEBUG_PRINT("=");
  DEBUG_PRINT("T:"); //...the command type...
  DEBUG_PRINTI(rType, HEX);
  DEBUG_PRINT("|L:");  //...and the command length.
  DEBUG_PRINTI(rLength, HEX);
  DEBUG_PRINTL(DME?'D':'.');
#endif

  switch(rType){  // Select the command handler routine to jump to based on the command type
    case 0x00: command_reference(); break;
    case 0x01: command_open(); break;
    case 0x02: command_close(); break;
    case 0x03: command_read(); break;
    case 0x04: command_write(); break;
    case 0x05: command_delete(); break;
    case 0x06: command_format(); break;
    case 0x07: command_status(); break;
    case 0x08: command_DMEReq(); break; // DME Command
    case 0x0C: command_condition(); break;
    case 0x0D: command_rename(); break;
    default: return_normal(0x36); break;  // Send a normal return with a parameter error if the command is not implemented
  }

#if DEBUG > 1
  DEBUG_PRINTI(head,HEX);
  DEBUG_PRINT(":");
  DEBUG_PRINTI(tail,HEX);
  DEBUG_PRINT("->");
#endif

  tail = tail+rLength+0x05;  // Increment the tail index past the previous command

#if DEBUG > 1
  DEBUG_PRINTIL(tail,HEX);
#endif
}
