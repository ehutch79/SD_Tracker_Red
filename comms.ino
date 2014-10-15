// basic message format:
//
// $devicename:messagetype:payload*XX where XX is a checksum excluding * but including the $

String getChecksum(String message) {
  int sum = 0;
  String sumstring;
  for (int i=0; i < message.length(); i++) {
    sum = message[i] + sum;
  }
  
  sumstring = String(sum, HEX);

  return sumstring.substring(sumstring.length()-2);
}

String getLocString(String deviceName, float lat, float lng) {
  String output = "$";
  output = output + deviceName + String(":");
  output = output + "loc:";
  output = output + String(lat,6);
  output = output + "," + String(lng,6);
  output = output + "*" + getChecksum(output);
  
  return output;  
}

String getNofixString(String deviceName) {
  String output = "$";
  output = output + deviceName + String(":");
  output = output + "loc:";
  output = output + "nofix";
  output = output + "*" + getChecksum(output);
 
  return output;  
}

void processSerial(String data) {
  String chkSum;
  String senderName;
  String messType;
  String payload;
  unsigned int pos = 1;
  unsigned int startPos = 1;
  
  data.trim();
  
  if(!data.startsWith("$")) { //toss invalid strings
    return; 
  }
  
  chkSum = data.substring(data.length()-2);
  data = data.substring(0, data.length()-3);
  if(chkSum != getChecksum(data)) {
    return; //strings invalid, toss it.
  }  

  while(pos<data.length()) {
    if(data.substring(pos, pos+1) == ":") { 
      senderName = data.substring(startPos, pos);
      startPos = pos+1;
      pos++;
      break;
    }
    pos++;
  }

  while(pos<data.length()) {
    if(data.substring(pos, pos+1) == ":") { 
      messType = data.substring(startPos, pos);
      startPos = pos+1;
      pos++;
      break;
    }
    pos++;
  }
  payload = data.substring(startPos);
  
  if(messType="loc") {
     parseLocMessage(senderName, payload);
  }
    
}

void parseLocMessage(String senderName, String payload) {
  unsigned int comma = payload.indexOf(",");
  
  float lat = 0.0;
  float lon = 0.0;

  if(comma) {
    char latBuff[comma+2];
    char lonBuff[(payload.length() - comma) +2];

  
    for(int i=0;i<comma;i++) {
      latBuff[i] = payload[i];  
    }
  
    for(int i=comma+1;i<payload.length();i++) {
      lonBuff[i-(comma+1)] = payload[i];
    }
  
    lat = atof(latBuff);
    lon = atof(lonBuff);
  }
  
  if(senderName == "black") {
    if(comma > 0) {
      blackLat = lat;
      blackLon = lon;
      blackFix = true;
    } else {
     blackFix = false; 
    }
  } else {
    if(comma > 0) {
      enemyLat = lat;
      enemyLon = lon;
      enemyFix = true;
    } else {
     enemyFix = false; 
    } 
  }
  
  
}
