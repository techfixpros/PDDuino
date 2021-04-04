void loop () {
 DEBUG_PRINTL(F("loop(): start"));

  byte rType = CMD_REFERENCE ; // Current request type (command type)
  byte rLength = 0x00; // Current request length (command length)
  byte diff = 0x00;  // Difference between the head and tail buffer indexes

  state = ST_WAITING;

  while(state<ST_WORKING){
#if defined(ENABLE_SLEEP)
    sleepNow();
#endif // ENABLE_SLEEP
    while (CLIENT.available() > 0x00){ // While there's data to read from the TPDD port...
#if defined(ENABLE_SLEEP)
 #if defined(SLEEP_DELAY)
      idleSince = millis();
 #endif // SLEEP_DELAY
#endif // ENABLE_SLEEP
      dataBuffer[head++]=(byte)CLIENT.read();  // ...pull the character from the TPDD port and put it into the command buffer, increment the head index...
      if(tail==head)tail++; // ...if the tail index equals the head index (a wrap-around has occoured! data will be lost!)
                            // ...increment the tail index to prevent the command size from overflowing.

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

    diff=(byte)(head-tail); // ...set the difference between the head and tail index (number of bytes in the buffer)

    if(state == ST_WAITING){
      if(diff >= 0x04){  // ...if there are 4 or more characters in the buffer...
        if(dataBuffer[tail]=='Z' && dataBuffer[(byte)(tail+0x01)]=='Z'){ // ...if the buffer's first two characters are 'Z' (a TPDD command)
          rLength = dataBuffer[(byte)(tail+0x03)]; // ...get the command length...
          rType = dataBuffer[(byte)(tail+0x02)]; // ...get the command type...
          state = ST_PARSING;
        }else if(dataBuffer[tail]=='M' && dataBuffer[(byte)(tail+0x01)]=='1'){ // If a DME command is received
          DME = true; // set the DME mode flag to true
          tail=tail+0x02;  // and skip past the command to the DME request command
        }else{  // ...if the first two characters are not 'Z'...
          tail=tail+(tail==head?0x00:0x01); // ...move the tail index forward to the next character, stop if we reach the head index to prevent an overflow.
        }
      }
    }

    if(state == ST_PARSING){
      if(diff>rLength+0x04){ //...if the amount of data in the buffer satisfies the command length...
          state = ST_WORKING;
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
    case CMD_REFERENCE: command_reference(); break;
    case CMD_OPEN:      command_open(); break;
    case CMD_CLOSE:     command_close(); break;
    case CMD_READ:      command_read(); break;
    case CMD_WRITE:     command_write(); break;
    case CMD_DELETE:    command_delete(); break;
    case CMD_FORMAT:    command_format(); break;
    case CMD_STATUS:    command_status(); break;
    case CMD_DMEREQ:    command_DMEreq(); break;
    case CMD_CONDITION: command_condition(); break;
    case CMD_RENAME:    command_rename(); break;
    default: return_normal(ERR_PARM); break;  // Send a normal return with a parameter error if the command is not implemented
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
