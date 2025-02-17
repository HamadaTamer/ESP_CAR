#include <ESP8266WiFi.h>
// #include <Servo.h>
// Servo myservo;

// H-bridge connections
int ena1 = 4;   // D2 - controls motor1 speed via PWM (0-255)
int in1  = 5;   // D1 - motor1 direction
int in2  = 16;  // D0 - motor1 direction

int ena2 = 14;  // D5 - controls motor2 speed via PWM (0-255)
int in3  = 0;   // D3 - motor2 direction
int in4  = 2;   // D4 - motor2 direction

// int distance =0;
// int duration =0;
// int echopin = 15;
// int trigpin = 13;

// int maxAngle =0;
// int maxDistance =0;

int servopin = 12;
// Wi-Fi credentials
const char* ssid     = "277353";
const char* password = "2004ahmed";

// Create an ESP8266 WiFi server on port 80
WiFiServer server(80);

void setup() {
  // Set H-bridge pins as outputs
  pinMode(ena1, OUTPUT);
  pinMode(in1,  OUTPUT);
  pinMode(in2,  OUTPUT);
  
  pinMode(ena2, OUTPUT);
  pinMode(in3,  OUTPUT);
  pinMode(in4,  OUTPUT);


  Serial.begin(115200); 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Server started.");
}

void loop() {
  // Listen for incoming clients
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c); // Echo to Serial Monitor

        if (c == '\n') {
          // End of the HTTP request
          if (currentLine.length() == 0) {
            // Send a simple response back to the client
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            client.println("Command received");
            break;
          } else {
            // Parse the request line if it starts with GET
            if (currentLine.startsWith("GET")) {
              int firstSpace  = currentLine.indexOf(' ');
              int secondSpace = currentLine.indexOf(' ', firstSpace + 1);
              
              // Extract the path from "GET /path HTTP/1.1"
              String url = currentLine.substring(firstSpace + 1, secondSpace);
              Serial.println("URL: " + url);

              // Example URL: /FORWARD/155/15
              // We'll split on '/'
              // url[0]  = '/'
              // so /FORWARD -> command, /155 -> speed, /15 -> time
              String cmd = "";
              String param1 = "";
              String param2 = "";

              int firstSlash = url.indexOf('/');
              if (firstSlash != -1) {
                int secondSlash = url.indexOf('/', firstSlash + 1);
                if (secondSlash != -1) {
                  // Command is between the firstSlash+1 and secondSlash
                  cmd = url.substring(firstSlash + 1, secondSlash);

                  int thirdSlash = url.indexOf('/', secondSlash + 1);
                  if (thirdSlash != -1) {
                    // param1 between secondSlash+1 and thirdSlash
                    param1 = url.substring(secondSlash + 1, thirdSlash);
                    // param2 from thirdSlash+1 to end
                    param2 = url.substring(thirdSlash + 1);
                  } else {
                    // Only one parameter
                    param1 = url.substring(secondSlash + 1);
                  }
                } else {
                  // Command is the only piece
                  cmd = url.substring(firstSlash + 1);
                }
              }

              // Execute the command
              executeCommand(cmd, param1, param2);
            }

            currentLine = "";
          }
        } else if (c != '\r') {
          // Add characters to the current line
          currentLine += c;
        }
      }
    }

    // Close the connection
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void executeCommand(String command, String speedstr, String timestr) {
  // Default speed = 255 if nothing provided
  int speed = 255;
  if (speedstr != "") {
    speed = speedstr.toInt();  // parse speed
  }

  // Default time = -1 (means "run indefinitely" if not provided)
  int timeInSeconds = -1;
  if (timestr != "") {
    timeInSeconds = timestr.toInt();  // parse time
  }

  // For forward/backward, we’ll do indefinite if time is -1,
  // else we’ll do a delay for the specified seconds and then stop.

  if (command == "forward") {
    Serial.println("CMD: FORWARD  speed=" + String(speed) + " time=" + String(timeInSeconds));
    if (timeInSeconds == -1) {
      forward(speed);
    } else {
      forward(speed);
      delay(timeInSeconds * 1000);  // convert seconds to milliseconds
      stop();
    }
  }
  else if (command == "backward") {
    Serial.println("CMD: BACKWARD speed=" + String(speed) + " time=" + String(timeInSeconds));
    if (timeInSeconds == -1) {
      backward(speed);
    } else {
      backward(speed);
      delay(timeInSeconds * 1000);
      stop();
    }
  }
  else if (command == "stop") {
    Serial.println("CMD: STOP");
    stop();
  }
  else if (command == "rotateclockwise") {
    Serial.println("CMD: ROTATECLOCKWISE");
     if (timeInSeconds == -1) {
      rotateclockwise();
    } else {
      rotateclockwise();
      delay(timeInSeconds * 1000);  // convert seconds to milliseconds
      stop();
    }
  }
  else if (command == "rotateanticlockwise") {
    Serial.println("CMD: rotateanticlockwise");
  if (timeInSeconds == -1) {
      rotateanticlockwise();
    } else {
      rotateanticlockwise();
      delay(timeInSeconds * 1000);  // convert seconds to milliseconds
      stop();
    }  }
  else if (command == "turn45degclockwise") {
    Serial.println("CMD: TURN45DEGCLOCKWISE");
    turn45degclockwise();
  }
  else if (command == "turn45deganticlockwise") {
    Serial.println("CMD: TURN45DEGANTICLOCKWISE");
    turn45deganticlockwise();
  }
  // else if(command == "AutonmousMode"){
  //   selfDriving();
  // }
  else {
    Serial.println("unknown command");
  }
}


// autonomous mode:
// void selfDriving(){
//   myservo.write(90);
//   if (distance > 30){
//     forward(255);
//   } else {
//     stop();
//     myservo.write(0);
//     for (int pos=0; pos<=180; pos+=10){
//       myservo.write(pos);
//       delay(15);
//       ultrasonic();
//       if (maxDistance > distance){
//         maxDistance = distance;
//         maxAngle = pos;
//       } 
//     }
//      if (maxDistance < 30){
//       turn45degclockwise();
//       turn45degclockwise();
//     }
//     if (maxAngle > 90){
//       for (int i=0; i< maxAngle %45; i++){
//         turn45degclockwise();
//       }
//     }else {
//       for (int i=0; i< maxAngle %45; i++){
//         turn45deganticlockwise();
//       }
//     }
//   } 
  
// }

// --- Motor Control Functions ---

void forward(int num) {
  // Move both motors forward at speed=num
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(ena1, num);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(ena2, num);
}

void backward(int num) {
  // Move both motors backward at speed=num
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(ena1, num);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(ena2, num);
}

void stop() {
  // Stop both motors
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void rotateclockwise() {
  // Spin in place clockwise
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(ena1, 255);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(ena2, 255);
}

void rotateanticlockwise() {
  // Example: left motor full speed forward, right motor slower or backward
  // Adjust as desired for your exact turning behavior
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(ena1, 255);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(ena2, 128);
}

void turn45degclockwise() {
  // Turn 45 degrees in place, guessed as ~250ms at full speed
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(ena1, 255);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(ena2, 255);

  delay(700);
  stop();
}

void turn45deganticlockwise() {
  // Turn 45 degrees in place the other way
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(ena1, 255);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(ena2, 255);

  delay(700);
  stop();
}

// void ultrasonic(){ 
//   float duration=0;
//   	//sending out an ultrasonic wave
// 	digitalWrite(trigpin, LOW);
//   	delayMicroseconds(2);
//   	digitalWrite(trigpin, HIGH);
//   	delayMicroseconds(10);
//   	digitalWrite(trigpin, LOW);
  	
//   	//recieving it
//   	duration= pulseIn(echopin, HIGH);
//   	distance=(0.0343*duration)/2; 
//       //s=d/t
//       //d=s*t
// }