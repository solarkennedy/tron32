


#define WAITING_FOR_STX 0
#define READING_DATA 1
#define DATA_VALID 2

static const int STX = 2;
static const int ETX = 3;

int state = WAITING_FOR_STX;
int _iNibbleCtr;
byte _data[6];
byte data[6];
int length = 0;


bool rdm630_available() {
  if (Serial2.available() > 0)
  {
    state = rdm630_dataParser(state, Serial2.read());
    return (state == DATA_VALID);
  }
  return false;
}

byte rdm630_AsciiCharToNum(byte data) {
  return (data > '9' ? data - '0' - 7 : data - '0');
}

int rdm630_dataParser(int s, byte c) {
  switch (s) {
    case WAITING_FOR_STX:
    case DATA_VALID:
      if (c == STX) {
        _iNibbleCtr = -1;
        return READING_DATA;
      }
      break;
    case READING_DATA:
      if (++_iNibbleCtr < 12) {
        _data[_iNibbleCtr >> 1] = ((_iNibbleCtr & 0x1) == 0 ? rdm630_AsciiCharToNum(c) << 4 : _data[_iNibbleCtr>>1] + rdm630_AsciiCharToNum(c));
        return READING_DATA;
      }
      if (c != ETX) { //Expected end character, but got something else
        return WAITING_FOR_STX;
      }
      for (int i = 0; i < 5; i++) {
        _data[5] ^= _data[i];
      }
      if (_data[5] != 0) { //Checksum doesn't match
        return WAITING_FOR_STX;
      }
      return DATA_VALID;
    default:
      return WAITING_FOR_STX;
  }
  return WAITING_FOR_STX;
}

void wait_for_rfid() {

  if (rdm630_available()) {
    Serial.println("Data valid");
    for (int i = 0; i < length; i++) {
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    //concatenate the bytes in the data array to one long which can be
    //rendered as a decimal number
    unsigned long result =
      ((unsigned long int)data[1] << 24) +
      ((unsigned long int)data[2] << 16) +
      ((unsigned long int)data[3] << 8) +
      data[4];
    Serial.print("decimal CardID: ");
    Serial.println(result);
  }
}
