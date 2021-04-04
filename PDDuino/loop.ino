void loop () {
  state_t state = ST_IDLE;
  uint8_t i = 0;
  uint8_t data;
  uint8_t cmd = 0; // make the compiler happy

  DEBUG_PRINTL(F("loop(): start"));

  while(true) {
  #if defined(ENABLE_SLEEP)
    sleepNow();
  #endif // ENABLE_SLEEP
    // should check for a timeout...
    while(CLIENT.available()) {
      #if defined(ENABLE_SLEEP)
       #if defined(SLEEP_DELAY)
        idleSince = millis();
       #endif // SLEEP_DELAY
      #endif // ENABLE_SLEEP
      data = (uint8_t)CLIENT.read();
      #if DEBUG > 1
        DEBUG_PRINTI((uint8_t)i, HEX);
        DEBUG_PRINT(" - ");
        DEBUG_PRINTI((uint8_t)data, HEX);
        if(data > 0x20 && data < 0x7f) {
          DEBUG_PRINT(";");
          DEBUG_PRINT((char)data);
        }
        DEBUG_PRINTL("");
      #endif
      switch (state) {
      case ST_IDLE:
        if(data == 'Z')
          state = ST_FOUND_Z;
        else if(data == 'M')
          state = ST_FOUND_DME_SET;
        break;
      case ST_FOUND_Z:
        if(data == 'Z')
          state = ST_FOUND_Z2;
        break;
      case ST_FOUND_Z2:
        cmd = data;
        state = ST_FOUND_CMD;
        break;
      case ST_FOUND_CMD:
        _length = data;
        i = 0;
        state = ST_FOUND_LEN;
        break;
      case ST_FOUND_LEN:
        if(i < _length)
          _cmd_buffer[i++] = data;
        else { // cmd is complete.  Execute
          #if DEBUG > 1
            DEBUG_PRINT("T:"); //...the command type...
            DEBUG_PRINTI(cmd, HEX);
            DEBUG_PRINT("|L:");  //...and the command length.
            DEBUG_PRINTI(_length, HEX);
            DEBUG_PRINTL(DME ? 'D' : '.');
          #endif
          switch(cmd){  // Select the command handler routine to jump to based on the command type
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
            case CMD_RENAME: command_rename(); break;
            default: return_normal(ERR_PARM); break;  // Send a normal return with a parameter error if the command is not implemented
          }
          state = ST_IDLE;
        }
        break;
      case ST_FOUND_DME_SET:
        // I *think* a DME set command is 'M1\r' to turn on and 'M0\r' to turn off, but we'll assume just the 'M1' for now
        if(data == '1') {
          DME = true;
        }
        state = ST_IDLE;
        break;
      default:
        // not sure how you'd get here, but...
        state = ST_IDLE;
        break;
      }
    }
  }
}
